/**
 * Caribou implementation for the dSiPM
 */

#include "dSiPMDevice.hpp"
#include "utils/log.hpp"

using namespace caribou;

sig_atomic_t dSiPMDevice::endPrint_(0);

dSiPMDevice::dSiPMDevice(const caribou::Configuration config)
    : CaribouDevice(config, iface_mem::configuration_type(MEM_PATH, DSIPM_READOUT)) {

  _dispatcher.add("configureClock", &dSiPMDevice::configureClock, this);
  _dispatcher.add("powerStatusLog", &dSiPMDevice::powerStatusLog, this);
  _dispatcher.add("configureMatrix", &dSiPMDevice::configureMatrix, this);
  _dispatcher.add("ledOn", &dSiPMDevice::ledOn, this);
  _dispatcher.add("ledOff", &dSiPMDevice::ledOff, this);
  _dispatcher.add("tOn", &dSiPMDevice::tOn, this);
  _dispatcher.add("tOff", &dSiPMDevice::tOff, this);
  _dispatcher.add("writeConfig", &dSiPMDevice::writeConfig, this);
  _dispatcher.add("printFrames", &dSiPMDevice::printFrames, this);
  _dispatcher.add("printLEDFrames", &dSiPMDevice::printLEDFrames, this);
  _dispatcher.add("storeFrames", &dSiPMDevice::storeFrames, this);
  _dispatcher.add("storeLEDFrames", &dSiPMDevice::storeLEDFrames, this);
  _dispatcher.add("printTemperatures", &dSiPMDevice::printTemperatures, this);
  _dispatcher.add("getTemperature", &dSiPMDevice::getTemperature, this);

  // Set up periphery
  _periphery.add("VDDIO3V3", carboard::PWR_OUT_1);
  _periphery.add("VDDIO1V8", carboard::PWR_OUT_2);
  _periphery.add("VDDCORE", carboard::PWR_OUT_3);
  _periphery.add("LED_PWR", carboard::PWR_OUT_4);
  _periphery.add("DS_PWR", carboard::PWR_OUT_6);

  _periphery.add("VBQ", carboard::BIAS_4);
  _periphery.add("VCLAMP", carboard::BIAS_2);
  _periphery.add("VBQ_TC", carboard::BIAS_1);
  _periphery.add("VCLAMP_TC", carboard::BIAS_3);

  _periphery.add("CMOS_OUT_1_TO_4_START", carboard::CMOS_OUT_1_TO_4);
  _periphery.add("CMOS_OUT_1_TO_4", carboard::CMOS_OUT_1_TO_4);
  _periphery.add("CMOS_OUT_5_TO_8", carboard::CMOS_OUT_5_TO_8);

  _periphery.add("TD_IN", carboard::CUR_1);
  _periphery.add("TD_OUT", carboard::VOL_IN_1);

  // Add the register definitions to the dictionary for convenient lookup of names:
  _registers.add(DSIPM_REGISTERS);

  // Add memory pages to the dictionary:
  _memory.add(DSIPM_MEMORY);

  // Reset the FPGA:
  this->setMemory("fpga_reset", 1);
  this->setMemory("fpga_reset", 0);
}

void dSiPMDevice::powerStatusLog() {
  LOG(INFO) << "Power status:";

  LOG(INFO) << "VDDIO3V3:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("VDDIO3V3") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("VDDIO3V3") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("VDDIO3V3") << "W";

  LOG(INFO) << "VDDIO1V8:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("VDDIO1V8") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("VDDIO1V8") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("VDDIO1V8") << "W";

  LOG(INFO) << "VDDCORE:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("VDDCORE") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("VDDCORE") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("VDDCORE") << "W";

  LOG(INFO) << "LED_PWR:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("LED_PWR") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("LED_PWR") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("LED_PWR") << "W";

  LOG(INFO) << "DS_PWR:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("DS_PWR") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("DS_PWR") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("DS_PWR") << "W";
}

void dSiPMDevice::configure() {

  LOG(INFO) << "Configuring";
  configureClock(_config.Get<bool>("clock_internal", true));
  // reset();
  mDelay(10);

  // Configure everything from the config files and write default values:
  CaribouDevice<carboard::Carboard, iface_mem>::configure();

  // mode_2bit (FW) and set2Bit (chip) flags should be consistent. Check:
  if(_config.Get("mode_2bit", _memory.get("mode_2bit").second.value()) ==
     _config.Get("set2bit", _registers.get("set2Bit").value())) {
    LOG(WARNING) << "Settings for 2-bit mode inconsistent between chip and firmware. Using default: OFF!";
    LOG(INFO) << "Set memory \"mode_2bit\" = " << dSiPM_mode_2bit << " (" << to_hex_string(dSiPM_mode_2bit) << ")";
    this->setMemory("mode_2bit", dSiPM_mode_2bit); // back to default
    LOG(INFO) << "Set register \"set2Bit\" = " << dSiPM_set2Bit << " (" << to_hex_string(dSiPM_set2Bit) << ")";
    this->setRegister("set2Bit", dSiPM_set2Bit);
  }

  LOG(INFO) << "Chipboard control Signals: ds_pd ON";
  this->setMemory("ds_pd", 1);

  LOG(INFO) << "HV ON";
  this->setMemory("hv_enable", 1);

  // Configure pixel matrix
  // Read matrix file from the configuration and program it:
  std::string maskFileName = _config.Get("mask_filename", "");
  if(!maskFileName.empty()) {
    LOG(INFO) << "Found mask file name in configuration: \"" << maskFileName << "\"...";
    configureMatrix(maskFileName);
  } else {
    LOG(INFO) << "No mask file name found in configuration.";
  }

  mDelay(10);
  writeConfig();
}

template <typename Enumeration> auto as_value(Enumeration const value) -> typename std::underlying_type<Enumeration>::type {
  return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

void dSiPMDevice::setSpecialRegister(const std::string& name, uintptr_t value) {
  LOG(DEBUG) << "Setting special register " << name;

  if(name == "quadrantsenable") {
    // re-program Mask
    programMask(value);
    storeQuadEnable_ = value;
    return;
  }

  LOG(WARNING) << "  Not implemented!";
  return;
}

uintptr_t dSiPMDevice::getSpecialRegister(const std::string& name) {
  LOG(DEBUG) << "Reading special register " << name;
  uintptr_t value = 0;
  if(name == "quadrantsenable") {
    value = storeQuadEnable_;
  } else {
    LOG(WARNING) << "  Not implemented!";
  }
  return value;
}

void dSiPMDevice::configureMatrix(std::string filename) {

  // Read mask from file
  if(!filename.empty()) {
    LOG(DEBUG) << "Configuring the pixel matrix from mask file \"" << filename << "\"";
    rowMasks_ = readMask(filename);
  }

  // Write mask to carboard memory (try a few times)
  int retry = 0;
  int retry_max = _config.Get<int>("retry_matrix_config", 3);
  while(true) {
    try {
      programMask(this->getRegister("quadrantsenable"));
      break;
    } catch(caribou::DataException& e) {
      LOG(ERROR) << e.what();
      if(++retry == retry_max) {
        throw CommunicationError("Matrix configuration failed");
      }
      LOG(INFO) << "Repeating configuration attempt";
    }
  }
}

void dSiPMDevice::programMask(size_t quadEnable) {

  // CLICTD does some checks here... do we need an equivalent?
  // -> Check compression?
  // -> Check for running/ stoped readout clock?
  // -> Read back settings?

  // range check
  if(quadEnable > 0x0f) {
    LOG(ERROR) << "QuadEnable > 0x0f is undefined, using mask file.";
    quadEnable = 0x0f;
  }
  // Translate quadrant enable into a mask
  size_t upperMask = 0;   // quadrant 0 and 1
  size_t lowerMask = 0;   // quadrant 2 and 3
  if(quadEnable & 0x01) { // xxxx bitwise and 0001 -> quadrant 0
    upperMask = static_cast<size_t>(0xffff);
  }
  if(quadEnable & 0x02) { // xxxx bitwise and 0010 -> quadrant 1
    upperMask += static_cast<size_t>(0xffff) << 16;
  }
  if(quadEnable & 0x04) { // xxxx bitwise and 0100 -> quadrant 2
    lowerMask = static_cast<size_t>(0xffff);
  }
  if(quadEnable & 0x08) { // xxxx bitwise and 1000 -> quadrant 3
    lowerMask += static_cast<size_t>(0xffff) << 16;
  }

  // write mask configuration row by row
  size_t row = totalNRows_;
  while(row > 0) {
    row--;
    std::string rowname("pixelEnableRow" + std::to_string(row));

    size_t rowmask;
    try { // handle reading errors
      rowmask = rowMasks_.at(row);
    } catch(const std::out_of_range& oor) {
      LOG(DEBUG) << "No mask for row " << row << ", using default";
      rowmask = _registers.get(rowname).value();
    }

    // apply enable masks
    if(row > 15) {
      rowmask = rowmask & upperMask;
    } else {
      rowmask = rowmask & lowerMask;
    }

    LOG(DEBUG) << "Writing row mask " << row << ": " << std::hex << rowmask << " to " << rowname;

    this->setRegister(rowname, rowmask);
  }

  return;
}

dSiPMDevice::rowMasks_t dSiPMDevice::readMask(std::string filename) {

  rowMasks_t rowmasks;
  size_t masked = 0;
  bool readingError = false;
  LOG(DEBUG) << "Reading mask file.";
  std::ifstream maskfile(filename);
  if(!maskfile.is_open()) {
    throw ConfigInvalid("Could not open mask file \"" + filename + "\"");
  }

  std::string line = "";
  while(std::getline(maskfile, line)) {

    // Skip empty and commented lines
    if(!line.length() || '#' == line.at(0)) {
      continue;
    }

    // read lines
    std::istringstream maskline(line);
    std::string tag;
    size_t row, val;
    size_t mask = 0;
    size_t cnt = 0;

    if(maskline >> tag >> row) {

      // check for row tag"storeFrames",
      if(std::strcmp(tag.c_str(), "row")) {
        continue;
      }

      // read mask values
      while(maskline >> val) {
        // build row mask
        mask |= static_cast<size_t>(static_cast<bool>(val) << cnt);
        // count masked pixels
        if(val == 0) {
          masked++;
        }
        cnt++;
      }

      // further format checks
      if(row > totalNRows_ - 1)
        readingError = true;
      if(cnt > totalNCols_)
        readingError = true;

      // store to mask type
      rowmasks.emplace(row, mask);

    } // maskline

  } // lines

  if(readingError) {
    LOG(ERROR) << "Wrong mask file format return empty mask";
    return rowMasks_t{};
  }

  LOG(INFO) << rowmasks.size() << " row masks cached, " << masked << " pixels are masked";

  return rowmasks;
}

dSiPMDevice::~dSiPMDevice() {
  LOG(INFO) << "Shutdown, delete device.";
  powerOff();
}

void dSiPMDevice::configureClock(bool internal) {

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
}

void dSiPMDevice::disableClock() {
  _hal->disableSI5345();
}

void dSiPMDevice::ledOn() { // arguments?? Delay, LED_TRIGGER settings etc.
  LOG(INFO) << "LED ON";

  LOG(DEBUG) << " LED_PWR: " << _config.Get("led_pwr", dSiPM_LED_PWR) << "V";
  this->setVoltage("LED_PWR", _config.Get("led_pwr", dSiPM_LED_PWR), _config.Get("led_pwr_current", dSiPM_LED_PWR_CURRENT));
  this->switchOn("LED_PWR");

  // TODO set led LED_TRIGGER and pulse delay
}

void dSiPMDevice::ledOff() {
  LOG(INFO) << "LED OFF";

  // stop led LED_TRIGGER ?

  LOG(DEBUG) << "Power off LED_PWR";
  this->switchOff("LED_PWR");
}

void dSiPMDevice::tOn() {
  LOG(INFO) << "Temperature sensor ON";

  LOG(DEBUG) << " TD_IN: " << static_cast<unsigned int>(_config.Get("td_in", dSiPM_TD_IN)) << "A";
  this->setCurrent("TD_IN", static_cast<unsigned int>(_config.Get("td_in", dSiPM_TD_IN)), 1);
  this->switchOn("TD_IN");
}

void dSiPMDevice::tOff() {
  LOG(INFO) << "Temperature sensor OFF";

  LOG(DEBUG) << "Power off TD_IN";
  this->switchOff("TD_IN");
}

void dSiPMDevice::writeConfig() {
  LOG(DEBUG) << " CMOS_OUT_1_TO_4: " << _config.Get("cmos_out_1_to_4_start_voltage", CMOS_OUT_1_TO_4_START_VOLTAGE) << "V";
  this->setVoltage("CMOS_OUT_1_TO_4_START", _config.Get("cmos_out_1_to_4_start_voltage", CMOS_OUT_1_TO_4_START_VOLTAGE));
  this->switchOn("CMOS_OUT_1_TO_4_START");

  LOG(INFO) << "Writing configuration into the chip.";
  this->setMemory("writeChipConfig", 1);

  // check for chip configuration to be completed
  size_t cnt = 0;
  while(getMemory("chipConfRunning") == 1) {
    // abort after 5 iterations
    if(cnt == 10) {
      LOG(ERROR) << "Timeout, chip configuration failed";
      return;
    }
    cnt++;
    usleep(1000);
  }

  LOG(INFO) << "Configuration written.";

  LOG(DEBUG) << " CMOS_OUT_1_TO_4: " << _config.Get("cmos_out_1_to_4_voltage", CMOS_OUT_1_TO_4_VOLTAGE) << "V";
  this->setVoltage("CMOS_OUT_1_TO_4", _config.Get("cmos_out_1_to_4_voltage", CMOS_OUT_1_TO_4_VOLTAGE));
  this->switchOn("CMOS_OUT_1_TO_4");
}

void dSiPMDevice::powerUp() {
  LOG(INFO) << "Powering up";

  LOG(DEBUG) << " VDDIO3V3: " << _config.Get("vddio3v3", dSiPM_VDDIO3V3) << "V";
  this->setVoltage(
    "VDDIO3V3", _config.Get("vddio3v3", dSiPM_VDDIO3V3), _config.Get("vddio3v3_current", dSiPM_VDDIO3V3_CURRENT));
  this->switchOn("VDDIO3V3");

  // Wait a bit
  usleep(10000);

  LOG(DEBUG) << " VDDIO1V8: " << _config.Get("vddio1v8", dSiPM_VDDIO1V8) << "V";
  this->setVoltage(
    "VDDIO1V8", _config.Get("vddio1v8", dSiPM_VDDIO1V8), _config.Get("vddio1v8_current", dSiPM_VDDIO1V8_CURRENT));
  this->switchOn("VDDIO1V8");

  // Wait a bit
  usleep(10000);

  LOG(DEBUG) << " VDDCORE: " << _config.Get("vddcore", dSiPM_VDDCORE) << "V";
  this->setVoltage("VDDCORE", _config.Get("vddcore", dSiPM_VDDCORE), _config.Get("vddcore_current", dSiPM_VDDCORE_CURRENT));
  this->switchOn("VDDCORE");

  // Wait a bit
  usleep(10000);

  LOG(DEBUG) << " DS_PWR: " << _config.Get("ds_pwr", dSiPM_DS_PWR) << "V";
  this->setVoltage("DS_PWR", _config.Get("ds_pwr", dSiPM_DS_PWR), _config.Get("ds_pwr_current", dSiPM_DS_PWR_CURRENT));
  this->switchOn("DS_PWR");

  // Wait a bit
  usleep(10000);

  LOG(DEBUG) << " CMOS_OUT_1_TO_4: " << _config.Get("cmos_out_1_to_4_voltage", CMOS_OUT_1_TO_4_VOLTAGE) << "V";
  this->setVoltage("CMOS_OUT_1_TO_4", _config.Get("cmos_out_1_to_4_voltage", CMOS_OUT_1_TO_4_VOLTAGE));
  this->switchOn("CMOS_OUT_1_TO_4");

  LOG(DEBUG) << " CMOS_OUT_5_TO_8: " << _config.Get("cmos_out_5_to_8_voltage", CMOS_OUT_5_TO_8_VOLTAGE) << "V";
  this->setVoltage("CMOS_OUT_5_TO_8", _config.Get("cmos_out_5_to_8_voltage", CMOS_OUT_5_TO_8_VOLTAGE));
  this->switchOn("CMOS_OUT_5_TO_8");

  // Wait a bit
  usleep(50000);

  /*
  DS_EQ
  DS_PE
  */

  LOG(INFO) << "Starting clock";

  // reset FPGA logic
  // this->setMemory("fpga_reset", 1);

  // configure Si5345 clock chip
  configureClock(_config.Get<bool>("clock_internal", true));

  // Wait a bit
  // usleep(50000);

  // release FPGA reset
  // this->setMemory("fpga_reset", 0);

  // Wait a bit
  usleep(50000);

  LOG(INFO) << "BIAS ON";

  LOG(DEBUG) << " VCLAMP bias: " << _config.Get("vclamp", dSiPM_VCLAMP) << "V";
  this->setVoltage("VCLAMP", _config.Get("vclamp", dSiPM_VCLAMP));
  this->switchOn("VCLAMP");

  // Wait a bit
  usleep(10000);

  LOG(DEBUG) << " VCLAMP_TC bias: " << _config.Get("vclamp_tc", dSiPM_VCLAMP_TC) << "V";
  this->setVoltage("VCLAMP_TC", _config.Get("vclamp_tc", dSiPM_VCLAMP_TC));
  this->switchOn("VCLAMP_TC");

  // Wait a bit
  usleep(10000);

  LOG(DEBUG) << " VBQ bias: " << _config.Get("vbq", dSiPM_VBQ) << "V";
  this->setVoltage("VBQ", _config.Get("vbq", dSiPM_VBQ));
  this->switchOn("VBQ");

  // Wait a bit
  usleep(10000);

  LOG(DEBUG) << " VBQ_TC bias: " << _config.Get("vbq_tc", dSiPM_VBQ_TC) << "V";
  this->setVoltage("VBQ_TC", _config.Get("vbq_tc", dSiPM_VBQ_TC));
  this->switchOn("VBQ_TC");

  // Wait a bit
  usleep(50000);

  if(_config.Get("led_on", dSiPM_LED_ON)) {
    ledOn();
    // Wait a bit
    usleep(50000);
  }

  if(_config.Get("t_on", dSiPM_T_ON)) {
    tOn();
    // Wait a bit
    usleep(50000);
  }

  return;
}

void dSiPMDevice::powerDown() {
  LOG(INFO) << "Powering down";
  LOG(INFO) << "Stop measurement -- TODO";

  tOff();

  // Wait a bit
  usleep(50000);
  ledOff();

  // Wait a bit
  usleep(50000);
  LOG(INFO) << "HV OFF";
  this->setMemory("hv_enable", 0);

  // Wait a bit
  usleep(50000);
  LOG(INFO) << "BIAS OFF";

  LOG(DEBUG) << "Power off VBQ_TC";
  this->switchOff("VBQ_TC");

  // Wait a bit
  usleep(10000);
  LOG(DEBUG) << "Power off VBQ";
  this->switchOff("VBQ");

  // Wait a bit
  usleep(10000);
  LOG(DEBUG) << "Power off VCLAMP_TC";
  this->switchOff("VCLAMP_TC");

  // Wait a bit
  usleep(10000);
  LOG(DEBUG) << "Power off VCLAMP";
  this->switchOff("VCLAMP");

  // Wait a bit
  usleep(50000);
  LOG(INFO) << "FPGA reset";
  this->setMemory("fpga_reset", 1);

  // Wait a bit
  usleep(50000);
  LOG(INFO) << "Clock OFF";
  disableClock();
  // Wait a bit
  usleep(50000);

  this->setMemory("fpga_reset", 0);

  // Wait a bit
  usleep(50000);
  LOG(INFO) << "Control Signals OFF -- TODO";

  LOG(INFO) << "Chipboard control Signals: ds_pd OFF";
  this->setMemory("ds_pd", 0);

  LOG(INFO) << "Power OFF";
  usleep(50000);
  // Wait a bit
  LOG(DEBUG) << "Power off CMOS signals level shifters";
  this->switchOff("CMOS_OUT_1_TO_4");
  this->switchOff("CMOS_OUT_5_TO_8");

  LOG(DEBUG) << "Power off DS_PWR";
  this->switchOff("DS_PWR");

  LOG(DEBUG) << "Power off VDDCORE";
  this->switchOff("VDDCORE");

  LOG(DEBUG) << "Power off VDDIO1V8";
  this->switchOff("VDDIO1V8");

  LOG(DEBUG) << "Power off VDDIO3V3";
  this->switchOff("VDDIO3V3");

  return;
}

void dSiPMDevice::daqStart() {
  // get daq mode from config file
  if(_config.Get("tlu_daq", dSiPM_TLU_DAQ)) {
    this->setMemory("daq_start_tlu", 1);
    LOG(INFO) << "DAQ started in TLU mode.";
  } else {
    this->setMemory("daq_start_stdalone", 1);
    LOG(INFO) << "DAQ started in standalone mode.";
  }
}

void dSiPMDevice::daqStop() {
  this->setMemory("daq_stop", 1);
  LOG(INFO) << "DAQ stopped.";
}

pearyRawData dSiPMDevice::getFrame() {

  LOG(INFO) << "Trying to read frame.";

  // check if we are in 2-bit mode. if so we need to read two frames!
  LOG(DEBUG) << "Two bit mode on? " << getMemory("mode_2bit");
  const unsigned int read_n_times = 1 + static_cast<unsigned int>(getMemory("mode_2bit"));

  // Check if there is something available
  unsigned int attempts = 0;
  while(!(getMemory("rd_data_available") > 0)) {
    usleep(100);
    if(attempts++ >= 10) {
      LOG(DEBUG) << "No data available in the readout memory blocks.";
      throw NoDataAvailable();
    }
  }
  // If so, continue with read out

  // we just read the 32 blocks of hit and 7 + 2 blocks of time stamp data
  constexpr unsigned int nblocks = DSIPM_N_HITDATA + DSIPM_N_TIMEDATA + DSIPM_N_DAQDATA;

  // we know how often we need to read, so resize output vector
  pearyRawData rawdata;
  const auto read_n_blocks = read_n_times * nblocks;
  rawdata.resize(read_n_blocks);
  LOG(DEBUG) << "Reading " << read_n_blocks << " words from readout memory block.";

  // read hit data and timestamps
  for(unsigned int times = 0; times < read_n_times; ++times) { // read twice if we are in 2-bit mode
    for(unsigned int block_i = 0; block_i < nblocks; ++block_i) {
      LOG(DEBUG) << "Word " << block_i << " round " << times;
      // Faster version of `rd_data_str = "rd_data_" + to_string(block_i)` using block_i < 100
      std::string rd_data_str = "rd_data_";
      if(block_i > 9) {
        // two digits
        rd_data_str.append({static_cast<char>((block_i / 10) + 48), static_cast<char>((block_i % 10) + 48)});
      } else {
        // only least significant digit
        rd_data_str.append({static_cast<char>((block_i % 10) + 48)});
      }
      // Read memory and fill array
      rawdata[times * nblocks + block_i] = getMemory(rd_data_str);
    }
  }

  // decoding is done in the frame decoder!

  return rawdata;
}

pearyRawData dSiPMDevice::getRawData() {
  return getFrame();
}

pearyRawData dSiPMDevice::getDeviceData() {
  LOG(INFO) << "dSiPMDevice::getDeviceData() not yet implemented. Output is same as getRawData.";

  // This is used to strip the timestamps in CLICTD... are we going to need this?
  // Can't we just also decode the timestamps?

  return getRawData();
}

pearydata dSiPMDevice::getData() {
  LOG(DEBUG) << "dSiPMDevice::getData(): Starting to decode frame.";
  auto rawdata = getDeviceData();

  // check if we are in 2-bit mode
  bool read2bit = getMemory("mode_2bit") ? true : false;

  return frame_decoder_.decodeFrame(rawdata, 0, read2bit);
}

double dSiPMDevice::getTemperature() {
  LOG(DEBUG) << "Getting temperature measurement.";

  // read ADC
  double temperature = getADC("TD_OUT");

  // get temperature offset
  double offset = _config.Get("temp_offset", dSiPM_TEMP_OFFSET);

  // get temperature slope
  double slope = _config.Get("temp_slope", dSiPM_TEMP_SLOPE);

  // apply nominal conversion
  temperature = (419.11 * (1.22 - temperature) - offset) / slope;

  return temperature;
}

void dSiPMDevice::printFrames(unsigned int N) {
  // get outfile name
  std::string frameFilename = _config.Get("frame_filename", dSiPM_frameFilename);
  LOG(INFO) << "Printing " << N << " frames to " << frameFilename;

  this->setMemory("daq_start_stdalone", 1);
  LOG(INFO) << "DAQ started in standalone mode.";

  // run
  for(unsigned int i = 0; i < N; i++) {
    auto frame = getData();
    frame_decoder_.printFrame(frame, frameFilename.c_str(), true);
  }

  // stop
  this->setMemory("daq_stop", 1);
}

void dSiPMDevice::printLEDFrames(unsigned int N) {
  // get outfile name
  std::string LEDframeFilename = _config.Get("LEDframe_filename", dSiPM_LEDframeFilename);
  LOG(INFO) << "Printing " << N << " frames to " << LEDframeFilename;

  this->setMemory("daq_start_stdalone", 1);

  // wait for DAQ to start
  for(unsigned int attempts = 32; attempts > 0; attempts--) {
    if(getMemory("daq_running") == 1) {
      LOG(INFO) << "DAQ started in standalone mode.";
      break;
    } else if(attempts == 1) {
      LOG(ERROR) << "Failed to start DAQ";
    }
    usleep(100);
  }

  // start the LED pulser
  this->setMemory("led_trigger", 1);
  LOG(DEBUG) << "Started LED pulser";
  unsigned int framenr = 0;
  bool ledrun = true;
  while(ledrun) {
    if(getMemory("led_trigger") == 0) {
      ledrun = false;
      // Stop the readout
      this->setMemory("daq_stop", 1);
      LOG(INFO) << "LED pulser finished, DAQ stopped, reading last data.";
    } else {
      LOG(DEBUG) << "LED pulser runs, checking for data...";
    }
    // get data from the buffer as long as there is something
    while(getMemory("rd_data_available") > 0) {
      LOG(INFO) << "Reading frame no. " << ++framenr << " from buffer";
      auto frame = getData();
      frame_decoder_.printFrame(frame, LEDframeFilename.c_str(), true);
      if((N != 0) && (framenr >= N)) {
        ledrun = false;
        // Stop the readout
        this->setMemory("daq_stop", 1);
        LOG(INFO) << "Reached maximum number of frames, DAQ stopped, readout stopped.";
        break;
      }
    }
    usleep(1000);
  }
}

void dSiPMDevice::storeFrames(unsigned int N, std::string filename) {

  LOG(STATUS) << "Storing " << N << " frames in " << filename;

  std::ofstream data_out;
  data_out.open(filename.c_str(), std::ios_base::app | std::ofstream::binary);

  if(data_out.fail()) {
    throw DeviceException("Can't open output file.\n");
  }

  this->setMemory("daq_start_stdalone", 1);
  LOG(INFO) << "DAQ started in standalone mode.";

  // run
  for(unsigned int i = 0; i < N; i++) {
    auto frame = getRawData();
    data_out.write(reinterpret_cast<const char*>(frame.data()),
                   static_cast<std::streamsize>(frame.size() * sizeof(pearyRawDataWord)));
  }

  // stop
  this->setMemory("daq_stop", 1);
  data_out.close();
  LOG(STATUS) << "Done storing frames";
}

void dSiPMDevice::storeLEDFrames(unsigned int N, std::string filename) {

  LOG(STATUS) << "Storing " << N << " frames in " << filename;

  std::ofstream data_out;
  data_out.open(filename.c_str(), std::ios_base::app | std::ofstream::binary);

  if(data_out.fail()) {
    throw DeviceException("Can't open output file.\n");
  }

  this->setMemory("daq_start_stdalone", 1);

  // wait for DAQ to start
  for(unsigned int attempts = 32; attempts > 0; attempts--) {
    if(getMemory("daq_running") == 1) {
      LOG(INFO) << "DAQ started in standalone mode.";
      break;
    } else if(attempts == 1) {
      LOG(ERROR) << "Failed to start DAQ";
    }
    usleep(100);
  }

  // start the LED pulser
  this->setMemory("led_trigger", 1);
  LOG(DEBUG) << "Started LED pulser";
  unsigned int framenr = 0;
  bool ledrun = true;
  while(ledrun) {
    if(getMemory("led_trigger") == 0) {
      ledrun = false;
      // Stop the readout
      this->setMemory("daq_stop", 1);
      LOG(INFO) << "LED pulser finished, DAQ stopped, reading last data.";
    } else {
      LOG(DEBUG) << "LED pulser runs, checking for data...";
    }
    // get data from the buffer as long as there is something
    while(getMemory("rd_data_available") > 0) {
      LOG(INFO) << "Reading frame no. " << ++framenr << " from buffer";
      auto frame = getRawData();
      data_out.write(reinterpret_cast<const char*>(frame.data()),
                     static_cast<std::streamsize>(frame.size() * sizeof(pearyRawDataWord)));

      if((N != 0) && (framenr >= N)) {
        ledrun = false;
        // Stop the readout
        this->setMemory("daq_stop", 1);
        LOG(INFO) << "Reached maximum number of frames, DAQ stopped, readout stopped.";
        break;
      }
    }
    usleep(1000);
  }

  data_out.close();
  LOG(STATUS) << "Done storing frames";
}

void dSiPMDevice::signalHandlerForPrint(int signum) {
  LOG(INFO) << "Received signal " << signum << " ending temperature measurement.";
  dSiPMDevice::endPrint_ = 1;
}

void dSiPMDevice::printTemperatures(int N) {
  // open outfile
  std::string fname = _config.Get("temperature_filename", dSiPM_temperatureFilename);
  m_outfileTemperatures.open(fname, std::ios_base::app); // append
  if(m_outfileTemperatures.is_open()) {
    LOG(INFO) << "Printing " << N << " temperature measurements to " << fname;
  } else {
    LOG(ERROR) << "Failed to open " << fname;
    return;
  }

  dSiPMDevice::endPrint_ = 0;
  signal(SIGINT, signalHandlerForPrint);

  // print date/ time to header
  time_t rawtime;
  struct tm* timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  m_outfileTemperatures << "Strarting " << asctime(timeinfo) << "Format: hh:mm:ss << temperature [C]" << std::endl;
  LOG(INFO) << "Strarting " << asctime(timeinfo) << "Format: hh:mm:ss temperature [C]";

  // write temperatures
  while(N != 0 && !dSiPMDevice::endPrint_) {
    // update time
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // temperature
    double temperature = getTemperature();

    // print
    LOG(INFO) << timeinfo->tm_hour << ":" << timeinfo->tm_min << ":" << timeinfo->tm_sec << " " << temperature
              << " C; cntrl-C to end measurement.";
    m_outfileTemperatures << timeinfo->tm_hour << ":" << timeinfo->tm_min << ":" << timeinfo->tm_sec << " " << temperature
                          << std::endl;

    if(N > 1) {
      usleep(10e6);
    }
    N--;
  }

  m_outfileTemperatures.close();
}
