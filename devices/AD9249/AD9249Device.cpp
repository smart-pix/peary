/** Caribou bare device example implementation
 */

#include "AD9249Device.hpp"
#include "utils/log.hpp"

// OS SPI support
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

using namespace caribou;

AD9249Device::AD9249Device(caribou::Configuration config)
    : CaribouDevice(config, InterfaceConfiguration(config.Get("devicepath", std::string(DEFAULT_DEVICEPATH)))) {

  _dispatcher.add("configureClock", &AD9249Device::configureClock, this);
  _dispatcher.add("resetAdc", &AD9249Device::resetAdc, this);
  _dispatcher.add("trigger", &AD9249Device::trigger, this);
  _dispatcher.add("dump", &AD9249Device::dump, this);
  _dispatcher.add("setDelay", &AD9249Device::setDelay, this);
  _dispatcher.add("setDelay2", &AD9249Device::setDelay2, this);
  _dispatcher.add("calibDelay", &AD9249Device::calibDelay, this);
  _dispatcher.add("setPattern", &AD9249Device::setPattern, this);
  _dispatcher.add("setBurst", &AD9249Device::setBurst, this);
  _dispatcher.add("setThreshold", &AD9249Device::setThreshold, this);
  _dispatcher.add("setDataDelay", &AD9249Device::setDataDelay, this);
  _dispatcher.add("enableTrigger", &AD9249Device::enableTrigger, this);
  _dispatcher.add("resetCounters", &AD9249Device::resetCounters, this);
  _dispatcher.add("fileDaqStart", &AD9249Device::fileDaqStart, this);
  _dispatcher.add("fileDaqStop", &AD9249Device::fileDaqStop, this);
  _dispatcher.add("testPattern", &AD9249Device::testPattern, this);
  _dispatcher.add("printFrame", &AD9249Device::printFrame, this);
  _dispatcher.add("printFrameFile", &AD9249Device::printFrameFile, this);

  // Add the register definitions to the dictionary for convenient lookup of names:
  _registers.add(AD9249_REGISTERS);

  // Add memory pages to the dictionary:
  _memory.add(AD9249_MEMORY);

  _memfd = open("/dev/mem", O_RDONLY | O_SYNC);

  // If we still don't have one, something went wrong:
  if(_memfd == -1) {
    throw DeviceException("Can't open /dev/mem.\n");
  }

  void* map0 = mmap(nullptr, ADC0_READOUT_MAP_SIZE, PROT_READ, MAP_SHARED, _memfd, ADC0_READOUT_BASE_ADDRESS);
  if(map0 == reinterpret_cast<void*>(-1)) {
    throw DeviceException("Can't map the memory to user space.\n");
  }

  void* map1 = mmap(nullptr, ADC1_READOUT_MAP_SIZE, PROT_READ, MAP_SHARED, _memfd, ADC1_READOUT_BASE_ADDRESS);
  if(map1 == reinterpret_cast<void*>(-1)) {
    throw DeviceException("Can't map the memory to user space.\n");
  }

  adc0_readout = reinterpret_cast<uint16_t*>(map0);
  adc1_readout = reinterpret_cast<uint16_t*>(map1);

  LOG(DEBUG) << "Reading config...";
  poll_interval = _config.Get<uint32_t>("poll_interval", 100);
  LOG(DEBUG) << "Setting polling interval to " << poll_interval;

  trigger_adc0 = _config.Get<uintptr_t>("trigger_adc0", 0);
  trigger_adc1 = _config.Get<uintptr_t>("trigger_adc1", 0);
}

AD9249Device::~AD9249Device() {
  LOG(INFO) << "Shutdown, delete device";

  munmap(adc0_readout, ADC0_READOUT_MAP_SIZE);
  munmap(adc1_readout, ADC1_READOUT_MAP_SIZE);
  close(_memfd);
}

std::string AD9249Device::getFirmwareVersion() {
  return "42.23alpha";
}

std::string AD9249Device::getType() {
  return "AD9249";
}

pearydata AD9249Device::getData() {
  pearydata x;
  x[{0u, 0u}] = std::make_unique<pixel>();
  x[{8u, 16u}] = std::make_unique<pixel>();
  return x;
}

void AD9249Device::configureClock(bool internal) {

  LOG(DEBUG) << "Configuring Si5345 clock source";
  _hal->configureSI5345(si5345_revb_registers, SI5345_REVB_REG_CONFIG_NUM_REGS);
  mDelay(100); // let the PLL lock

  // If required, check whether we are locked to external clock:
  if(!internal) {
    LOG(DEBUG) << "Waiting for clock to lock...";
    // Try for a limited time to lock, otherwise abort:
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    while(!_hal->isLockedSI5345()) {
      auto dur = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start);
      if(dur.count() > 3)
        throw DeviceException("Cannot lock to external clock.");
    }
  }

  setMemory("counter_reset", 4);
  mDelay(1);
  setMemory("counter_reset", 0);
  mDelay(1);
}

void AD9249Device::setPattern(uint16_t v) {

  LOG(DEBUG) << "Setting pattern " << to_hex_string(v);

  // Set same pattern on both channels. Output is the 14 MSBs.
  uint32_t val = ((static_cast<uint32_t>(v) << 18) | (static_cast<uint32_t>(v) << 2)) & 0xFFFCFFFC;

  setRegister("user_pattern1_lsb", val & 0xFF);
  setRegister("user_pattern1_msb", (val >> 8) & 0xFF);
  setRegister("user_pattern2_lsb", (val >> 16) & 0xFF);
  setRegister("user_pattern2_msb", (val >> 24) & 0xFF);
}

void AD9249Device::configure() {

  if(_config.Get<bool>("configure_clock", true)) {
    auto clk = _config.Get<bool>("clock_internal", true);
    LOG(INFO) << "Clock to be configured: " << (clk ? "internal" : "external") << " (" << clk << ")";
    configureClock(clk);
  } else {
    LOG(INFO) << "NOT configuring clocks";
  }

  setRegister("power_mode", 0x03); // Reset
  mDelay(100);

  setRegister("power_mode", 0x00);              // Running mode
  setRegister("clock_divide", 0x00);            // No clock division
  setRegister("output_mode", 0x00);             // Output mode offset binary
  setRegister("output_adjust", 0x00);           // No termination
  setRegister("serial_output_data_ctrl", 0x41); // DDR, 14 bits
  setRegister("test_mode", 0x00);               // Disable test pattern

  // Set additional busy time after a readout:
  auto dtc = _config.Get<uintptr_t>("deadtime_cycles", 0);
  setMemory("deadtime_cycles", dtc);
  LOG(INFO) << "Additional deadtime cycles after readout: " << dtc;

  // Reset the ADC
  resetAdc();

  setBurst(_config.Get<uintptr_t>("burst_length", 1));
  setDataDelay(_config.Get<uintptr_t>("data_delay", 1));
  setThreshold(_config.Get<uintptr_t>("threshold_low", 0), _config.Get<uintptr_t>("threshold_high", 0));

  setMemory("adc_finedelay", 0);
  mDelay(1);
  setMemory("adc_framedata_alignment", _config.Get<uintptr_t>("adc0_framedata_alignment", 0));
  mDelay(1);
  setMemory("adc_finedelay", 1);
  mDelay(1);
  setMemory("adc_framedata_alignment", _config.Get<uintptr_t>("adc1_framedata_alignment", 0));
  mDelay(1);

  setDelay(0, _config.Get("adc0_delay", 0));
  setDelay(1, _config.Get("adc1_delay", 0));

  setDelay2(0, _config.Get("ch0_delay", 0));
  setDelay2(1, _config.Get("ch1_delay", 0));
  setDelay2(2, _config.Get("ch2_delay", 0));
  setDelay2(3, _config.Get("ch3_delay", 0));
  setDelay2(4, _config.Get("ch4_delay", 0));
  setDelay2(5, _config.Get("ch5_delay", 0));
  setDelay2(6, _config.Get("ch6_delay", 0));
  setDelay2(7, _config.Get("ch7_delay", 0));
  setDelay2(8, _config.Get("ch8_delay", 0));
  setDelay2(9, _config.Get("ch9_delay", 0));
  setDelay2(10, _config.Get("ch10_delay", 0));
  setDelay2(11, _config.Get("ch11_delay", 0));
  setDelay2(12, _config.Get("ch12_delay", 0));
  setDelay2(13, _config.Get("ch13_delay", 0));
  setDelay2(14, _config.Get("ch14_delay", 0));
  setDelay2(15, _config.Get("ch15_delay", 0));
}

void AD9249Device::resetAdc() {
  setMemory("fifo_reset", 3);
  mDelay(1);
  setMemory("fifo_reset", 0);
  mDelay(1);

  setMemory("address_reset", 3);
  mDelay(1);
  setMemory("address_reset", 0);
  mDelay(1);

  setMemory("data_gen_reset", 3);
  mDelay(1);
  setMemory("data_gen_reset", 0);
  mDelay(1);

  setMemory("burst_enable", 3);
  mDelay(1);

  resetCounters();
}

void AD9249Device::trigger(uintptr_t t) {
  for(uintptr_t i = 0; i < t; i++) {
    setMemory("trigger", 3);
    usleep(100);
    setMemory("trigger", 0);
    usleep(100);
  }
}

void AD9249Device::dump() {
  LOG(INFO) << "ADC0: " << to_hex_string(getMemory("adc0_current_address"));
  LOG(INFO) << "ADC1: " << to_hex_string(getMemory("adc1_current_address"));
  LOG(INFO) << "ADC0 trigger: " << getMemory("adc0_trigger_count");
  LOG(INFO) << "ADC1 trigger: " << getMemory("adc1_trigger_count");

  for(int i = 0; i < 32; i++) {
    uint16_t val0 = adc0_readout[i] & 0x3FFF;
    uint16_t val1 = adc1_readout[i] & 0x3FFF;

    LOG(INFO) << to_hex_string(val0) << ' ' << to_hex_string(val1);
  }
}

void AD9249Device::setDelay(int ch, int delay) {
  setMemory("adc_finedelay", (static_cast<uintptr_t>(ch) & 1) | ((static_cast<uintptr_t>(delay) & 0x1F) << 1));
  mDelay(1);
  setMemory("adc_finedelay", (static_cast<uintptr_t>(ch) & 1) | ((static_cast<uintptr_t>(delay) & 0x1F) << 1) | (1 << 6));
  mDelay(1);

  LOG(INFO) << "Reading fine delay: " << getMemory("adc_fine_delay_rb");
  LOG(INFO) << "Delay stable: " << getMemory("fine_delay_stable");

  setMemory("adc_finedelay", (static_cast<uintptr_t>(ch) & 1) | ((static_cast<uintptr_t>(delay) & 0x1F) << 1));
  mDelay(1);
  LOG(INFO) << "Reading fine delay: " << getMemory("adc_fine_delay_rb");
  LOG(INFO) << "Delay stable: " << getMemory("fine_delay_stable");
  setMemory("adc_finedelay", 0);
}

void AD9249Device::setDelay2(int ch, int delay) {
  LOG(INFO) << "Delay " << delay << " channel " << ch;
  setMemory("adc_finedelay", ((static_cast<uintptr_t>(ch) & 0xF) << 12) | ((static_cast<uintptr_t>(delay) & 0x1F) << 7));
  mDelay(1);
  setMemory("adc_finedelay",
            ((static_cast<uintptr_t>(ch) & 0xF) << 12) | ((static_cast<uintptr_t>(delay) & 0x1F) << 7) | (1 << 16));
  mDelay(1);
  setMemory("adc_finedelay", ((static_cast<uintptr_t>(ch) & 0xF) << 12) | ((static_cast<uintptr_t>(delay) & 0x1F) << 7));
  mDelay(1);

  setMemory("adc_finedelay", 0);
}

void AD9249Device::testPattern() {
  uintptr_t tr = 10;
  setRegister("test_mode", 8); // Enable test pattern

  for(uintptr_t i = 0; i < 0x4000; i++) {
    if(i % 0x400 == 0) {
      LOG(INFO) << "Testing pattern " << to_hex_string(i) << "-" << to_hex_string(i + 0x3FF);
    }

    setPattern(static_cast<uint16_t>(i));
    resetAdc();
    trigger(tr);
    uintptr_t incorrect[16] = {0};

    for(uintptr_t j = 0; j < 128 * burst_length * tr; j++) {
      for(uintptr_t c = 0; c < 16; c++) {
        uint16_t val;

        if(c < 8) {
          val = adc0_readout[j * 8 + c] & 0x3FFF;
        } else {
          val = adc1_readout[j * 8 + c - 8] & 0x3FFF;
        }

        if(val != i) {
          LOG(INFO) << "Channel " << c << ": expected " << to_hex_string(i) << " received " << to_hex_string(val);
          incorrect[c]++;
        }
      }
    }

    for(uintptr_t c = 0; c < 16; c++) {
      if(incorrect[c] > 0) {
        LOG(INFO) << "Pattern " << to_hex_string(i) << " incorrect " << incorrect[c] << "/" << 128 * burst_length * tr;
      }
    }
  }

  setRegister("test_mode", 0); // Disable test pattern
}

void AD9249Device::calibDelay() {
  uintptr_t tr = 10;
  uint16_t p = 0x2DDC;

  uint16_t pattern = p & 0x3FFF;
  uint16_t pattern_p1 = ((pattern << 1) | (pattern >> 13)) & 0x3FFF;
  uint16_t pattern_p2 = ((pattern << 2) | (pattern >> 12)) & 0x3FFF;
  uint16_t pattern_m1 = ((pattern >> 1) | (pattern << 13)) & 0x3FFF;
  uint16_t pattern_m2 = ((pattern >> 2) | (pattern << 12)) & 0x3FFF;

  std::stringstream out;

  setPattern(pattern);
  setRegister("test_mode", 8); // Enable test pattern

  for(int i = 0; i < 32; i++) {
    resetAdc();

    for(int j = 0; j < 16; j++) {
      setDelay2(j, i);
    }

    trigger(tr);

    LOG(INFO) << "Delay: " << i;

    out << i << '\t';

    for(uintptr_t c = 0; c < 16; c++) {
      uintptr_t count_p1 = 0;
      uintptr_t count_p2 = 0;
      uintptr_t count_0 = 0;
      uintptr_t count_m1 = 0;
      uintptr_t count_m2 = 0;
      uintptr_t count_msc = 0;

      for(uintptr_t j = 0; j < 128 * burst_length * tr; j++) {
        uint16_t val;

        if(c < 8) {
          val = adc0_readout[j * 8 + c] & 0x3FFF;
        } else {
          val = adc1_readout[j * 8 + c - 8] & 0x3FFF;
        }

        if(val == pattern_p1) {
          count_p1++;
        } else if(val == pattern_p2) {
          count_p2++;
        } else if(val == pattern) {
          count_0++;
        } else if(val == pattern_m1) {
          count_m1++;
        } else if(val == pattern_m2) {
          count_m2++;
        } else {
          count_msc++;
        }
      }

      if(count_p1 == 128 * burst_length * tr) {
        out << '+';
      } else if(count_p2 == 128 * burst_length * tr) {
        out << '*';
      } else if(count_0 == 128 * burst_length * tr) {
        out << '0';
      } else if(count_m1 == 128 * burst_length * tr) {
        out << '-';
      } else if(count_m2 == 128 * burst_length * tr) {
        out << '/';
      } else {
        out << '?';
      }
    }
    out << '\n';
  }

  std::cout << out.str();
  setRegister("test_mode", 0); // Disable test pattern
}

void AD9249Device::fileDaqStart() {
  // ensure only one daq thread is running
  if(daqRunning) {
    LOG(WARNING) << "Data aquisition is already running";
    return;
  }

  if(_daqThread.joinable()) {
    LOG(WARNING) << "DAQ thread is already running";
    return;
  }

  data_out.open("adc.dat", std::ofstream::out | std::ofstream::binary);

  if(data_out.fail()) {
    throw DeviceException("Can't open output file.\n");
  }

  // Enable triggers et cetera:
  daqStart();

  _daqContinue.test_and_set();
  _daqThread = std::thread(&AD9249Device::runDaq, this);

  daqRunning = true;
}

void AD9249Device::daqStart() {
  enableTrigger(0, 0); // Disable triggers
  resetAdc();
  setRegister("test_mode", 0x0);

  enableTrigger(trigger_adc0, trigger_adc1);

  prev_addr_ch0 = 0;
  prev_addr_ch1 = 0;

  LOG(DEBUG) << "DAQ running";
}

void AD9249Device::fileDaqStop() {
  if(_daqThread.joinable()) {
    // signal to daq thread that we want to stop and wait until it does
    _daqContinue.clear();
    _daqThread.join();
  }
  _daqContinue.clear();

  daqRunning = false;

  data_out.close();

  // Stop triggers
  daqStop();
}

void AD9249Device::daqStop() {
  enableTrigger(0, 0); // Disable triggers

  LOG(INFO) << "Received triggers: " << getMemory("adc0_trigger_count") << ' ' << getMemory("adc1_trigger_count");
}

void AD9249Device::runDaq() {
  bool flag;
  size_t total_ch0 = 0;
  size_t total_ch1 = 0;

  Log::setReportingLevel(LogLevel::INFO);

  for(;;) {
    flag = _daqContinue.test_and_set();
    if(!flag) {
      LOG(INFO) << "Recorded bytes: " << total_ch0 << " " << total_ch1;
      break;
    }

    auto curr_addr_ch0 = getMemory("adc0_current_address");
    auto curr_addr_ch1 = getMemory("adc1_current_address");

    uintptr_t event_size = burst_length * 128 * 8 * 2; // = 2048
    auto data_ch0 = (prev_addr_ch0 <= curr_addr_ch0 ? curr_addr_ch0 - prev_addr_ch0
                                                    : ADC0_READOUT_MAP_SIZE - prev_addr_ch0 + curr_addr_ch0);
    auto data_ch1 = (prev_addr_ch1 <= curr_addr_ch1 ? curr_addr_ch1 - prev_addr_ch1
                                                    : ADC1_READOUT_MAP_SIZE - prev_addr_ch1 + curr_addr_ch1);

    if(data_ch0 >= event_size) {
      auto data0 = readChannel(0, prev_addr_ch0, curr_addr_ch0, adc0_readout);
      data_out.write(reinterpret_cast<const char*>(data0.data()),
                     static_cast<std::streamsize>(data0.size() * sizeof(pearyRawDataWord)));
      total_ch0 += data0.size() * sizeof(pearyRawDataWord);
    }

    if(data_ch1 >= event_size) {
      auto data1 = readChannel(1, prev_addr_ch1, curr_addr_ch1, adc1_readout);
      data_out.write(reinterpret_cast<const char*>(data1.data()),
                     static_cast<std::streamsize>(data1.size() * sizeof(pearyRawDataWord)));
      total_ch1 += data1.size() * sizeof(pearyRawDataWord);
    }

    mDelay(poll_interval);
  }
}

pearyRawData AD9249Device::getRawData() {
  auto curr_addr_ch0 = getMemory("adc0_current_address");
  auto curr_addr_ch1 = getMemory("adc1_current_address");

  uintptr_t event_size = burst_length * 128 * 8 * 2; // = 2048
  auto data_ch0 =
    (prev_addr_ch0 <= curr_addr_ch0 ? curr_addr_ch0 - prev_addr_ch0 : ADC0_READOUT_MAP_SIZE - prev_addr_ch0 + curr_addr_ch0);
  auto data_ch1 =
    (prev_addr_ch1 <= curr_addr_ch1 ? curr_addr_ch1 - prev_addr_ch1 : ADC1_READOUT_MAP_SIZE - prev_addr_ch1 + curr_addr_ch1);

  pearyRawData data;

  if(data_ch0 >= event_size && data_ch1 >= event_size) {
    data = readChannel(0, prev_addr_ch0, curr_addr_ch0, adc0_readout);
    auto data2 = readChannel(1, prev_addr_ch1, curr_addr_ch1, adc1_readout);
    data.insert(data.end(), data2.begin(), data2.end());
  }

  if(data.empty()) {
    throw NoDataAvailable();
  }

  LOG(DEBUG) << "Read one event, size " << data.size() * sizeof(pearyRawDataWord);
  return data;
}

pearyRawData AD9249Device::readChannel(uintptr_t ch, uintptr_t& prev_addr, uintptr_t curr_addr, uint16_t* base) {
  pearyRawData data;

  //                    bursts         data pt/burst   channels   byte per pt
  uintptr_t event_size = burst_length * 128 * 8 * 2; // = 2048

  // DMA not empy:
  LOG(DEBUG) << "CH" << ch << " Previous: " << to_hex_string(prev_addr) << " Current: " << to_hex_string(curr_addr)
             << "    Size: " << to_hex_string(curr_addr - prev_addr);

  pearyRawDataWord header =
    (static_cast<pearyRawDataWord>(ch) & 0xFFFF) | ((static_cast<pearyRawDataWord>(burst_length) & 0xFFFF) << 16);
  data.push_back(header);

  if(prev_addr <= curr_addr) {
    auto len = curr_addr - prev_addr;

    if(len > event_size) {
      len = event_size;
    }

    data.push_back(len);
    auto size = data.size();
    data.resize(size + len / sizeof(pearyRawDataWord));
    memcpy(&data[size], reinterpret_cast<const char*>(base) + prev_addr, len);

    // Advance read pointer
    prev_addr += event_size;
  } else {
    // LOG(INFO) << "Passed end of buffer";

    // Split length in two parts - from previous pos to end of buffer and from beginning to current pos
    auto len_end = ADC0_READOUT_MAP_SIZE - prev_addr;
    auto len_begin = curr_addr;

    auto len_total = len_end + len_begin;

    if(len_total > event_size) {
      len_total = event_size;
    }

    data.push_back(len_total);
    auto size = data.size();
    data.resize(size + len_total / sizeof(data[0]));

    // Read to end of buffer:
    if(len_end > event_size) {
      len_end = event_size;
      len_begin = 0;
    } else {
      len_begin = event_size - len_end;
    }
    memcpy(&data[size], reinterpret_cast<const char*>(base) + prev_addr, len_end);

    // Read from beginning of buffer if necessary
    if(len_begin > 0) {
      auto size_end = size + len_end / sizeof(data[0]);
      memcpy(&data[size_end], reinterpret_cast<const char*>(base), len_begin);

      // Advance read pointer
      prev_addr = len_begin;
    } else {
      // Advance read pointer
      prev_addr += event_size;
    }
  }
  return data;
}

void AD9249Device::printFrame() {
  printFrameFile("apts_frames.txt");
}

void AD9249Device::printFrameFile(std::string filename) {

  // std::vector<uintptr_t>
  pearyRawData rawdata;

  bool foundOne = false;

  try {

    // just read untill the buffer is empty
    while(true) {

      rawdata = getRawData();

      // the decoder in eudaq is for std::vector<uint8_t>
      const size_t header_offset = 8;
      std::vector<uint8_t> datablock0;
      datablock0.resize(sizeof(rawdata[0]) * rawdata.size() / sizeof(uint8_t));
      std::memcpy(&datablock0[0], &rawdata[0], sizeof(rawdata[0]) * rawdata.size());

      // std::ofstream out;
      // out.open("/tmp/out.dat", std::ofstream::out | std::ofstream::binary |
      // std::ofstream::app); out.write(reinterpret_cast<char*>(datablock0.data()),
      // datablock0.size()); out.close();

      // Get configured burst length from header:
      uint32_t burst_length_read = (static_cast<uint32_t>(datablock0.at(3)) << 8) | datablock0.at(2);

      // Check total available data against expected event size:
      const size_t evt_length = burst_length_read * 128 * 2 * 16 + 16;
      if(datablock0.size() < evt_length) {
        // FIXME throw something at someone?
        // std::cout << "Event length " << datablock0.size() << " not enough for
        // full event, requires " << evt_length << std::endl;
        return;
      }

      LOG(DEBUG) << "Burst: " << burst_length_read;

      // Read waveforms
      std::vector<std::vector<uint16_t>> waveforms;
      waveforms.resize(16);

      uint32_t size_ADC0 = (static_cast<uint32_t>(datablock0.at(7)) << 24) +
                           (static_cast<uint32_t>(datablock0.at(6)) << 16) + (static_cast<uint32_t>(datablock0.at(5)) << 8) +
                           static_cast<uint32_t>(datablock0.at(4) << 0);
      uint32_t size_ADC1 = (static_cast<uint32_t>(datablock0.at(header_offset + size_ADC0 + 7)) << 24) +
                           (static_cast<uint32_t>(datablock0.at(header_offset + size_ADC0 + 6)) << 16) +
                           (static_cast<uint32_t>(datablock0.at(header_offset + size_ADC0 + 5)) << 8) +
                           static_cast<uint32_t>(datablock0.at(header_offset + size_ADC0 + 4) << 0);

      // Decode channels:
      uint64_t timestamp0 = 0;
      uint64_t timestamp1 = 0;
      decodeChannel(0, datablock0, size_ADC0, header_offset, waveforms, timestamp0);
      decodeChannel(1, datablock0, size_ADC1, 2 * header_offset + size_ADC0, waveforms, timestamp1);

      LOG(DEBUG) << "ADC0 timestamp " << timestamp0;
      LOG(DEBUG) << "ADC1 timestamp " << timestamp1;

      // Channels are sorted like ADC0: A1 C1 E1 ...
      //                          ADC1: B1 D1 F1 ...
      std::vector<std::pair<int, int>> mapping = {{1, 2},
                                                  {0, 2},
                                                  {1, 1},
                                                  {1, 0},
                                                  {0, 3},
                                                  {0, 1},
                                                  {0, 0},
                                                  {2, 0},
                                                  {2, 1},
                                                  {3, 0},
                                                  {3, 2},
                                                  {3, 3},
                                                  {3, 1},
                                                  {2, 2},
                                                  {2, 3},
                                                  {1, 3}};
      // AD9249 channels to pixel matrix map:
      // A2, H2, F2, H1
      // C1, A1, D2, F1
      // C2, E1, B1, B2
      // E2, G1, G2, D1

      // TODO continue

      // Open and print preamble
      m_outfileFrames.open(filename, std::ios_base::app); // append

      for(size_t ch = 0; ch < waveforms.size(); ch++) {
        m_outfileFrames << trigger_ << " " << ch << " " << mapping.at(ch).first << " " << mapping.at(ch).second << " : ";
        auto const& waveform = waveforms.at(ch);
        for(auto const& sample : waveform) {
          m_outfileFrames << sample << " ";
        }
        m_outfileFrames << std::endl;
      }

      m_outfileFrames.close();

      trigger_++;
      foundOne = true;

    } // while true

  } catch(NoDataAvailable&) {
    if(foundOne) {
      LOG(INFO) << "Buffer empty, stop reading";
    } else {
      LOG(ERROR) << "No frame data available";
    }
    return;
  }

  return;
}

void AD9249Device::decodeChannel(const size_t adc,
                                 const std::vector<uint8_t>& data,
                                 size_t size,
                                 size_t offset,
                                 std::vector<std::vector<uint16_t>>& waveforms,
                                 uint64_t& timestamp) {

  // Timestamp index
  size_t ts_i = 0;

  for(size_t i = offset; i < offset + size; i += 2) {
    // Channel is ADC half times channels plus channel number within data block
    size_t ch = adc * 8 + ((i - offset) / 2) % 8;

    // Get waveform data
    uint16_t val = static_cast<uint16_t>(data.at(i) + ((static_cast<uint16_t>(data.at(i + 1)) & 0x3F) << 8));
    waveforms.at(ch).push_back(val);

    // If we have a full timestamp, skip the rest:
    if(ts_i >= 28) {
      continue;
    }

    // Treat timestamp data
    auto ts = static_cast<uint64_t>(data.at(i + 1) >> 6);

    // Channel 7 (or 15) have status bits only:
    if(ch == adc * 8 + 7) {
      // Check if this is a timestamp start - if not, reset timestamp index to
      // zero:
      if(ts_i < 8 && (ts & 0x1) == 0) {
        ts_i = 0;
      }
    } else {
      timestamp += (ts << 2 * ts_i);
      ts_i++;
    }
  }

  // Convert timestamp to picoseconds from the 65MHz clock (~15ns cycle):
  //  timestamp *= static_cast<uint64_t>(1. / 65. * 1e6);
  timestamp = static_cast<uint64_t>(static_cast<double>(timestamp * 1000000) / 65.);
}

void AD9249Device::setBurst(uintptr_t len) {
  setMemory("adc0_burst_length", len);
  mDelay(1);
  setMemory("adc1_burst_length", len);
  mDelay(1);

  burst_length = len;
}

void AD9249Device::setThreshold(uintptr_t low, uintptr_t high) {
  setMemory("trigger_threshold", (low & 0x3FFF) | ((high & 0x3FFF) << 16));
}

void AD9249Device::setDataDelay(uintptr_t delay) {
  setMemory("data_delay", delay);
  mDelay(1);
  setMemory("data_delay", delay | (1 << 9));
  mDelay(1);
  setMemory("data_delay", delay);
  mDelay(1);
}

void AD9249Device::enableTrigger(uintptr_t ch0, uintptr_t ch1) {
  auto val = ch0 | (ch1 << 16);
  setMemory("trigger_flags", val);
}

void AD9249Device::setTriggerMask(uintptr_t ch0, uintptr_t ch1) {
  trigger_adc0 = ch0;
  trigger_adc1 = ch1;
}

void AD9249Device::resetCounters() {
  setMemory("counter_reset", 3);
  mDelay(1);
  setMemory("counter_reset", 0);
  mDelay(1);
}
