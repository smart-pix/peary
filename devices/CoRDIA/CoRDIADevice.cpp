/**
 * Caribou implementation for CoRDIA_02
 */

#include "CoRDIADevice.hpp"
#include "utils/log.hpp"

using namespace caribou;

CoRDIADevice::CoRDIADevice(const caribou::Configuration config)
    : CaribouDevice(config, iface_mem::configuration_type(MEM_PATH, CORDIA_FPGA_MEM)) {

  _dispatcher.add("powerStatusLog", &CoRDIADevice::powerStatusLog, this);
  _dispatcher.add("writeConfig", &CoRDIADevice::writeConfig, this);
  _dispatcher.add("writePG", &CoRDIADevice::configurePatternGenerator, this);
  _dispatcher.add("startPG", &CoRDIADevice::startPatternGenerator, this);
  _dispatcher.add("stopPG", &CoRDIADevice::stopPatternGenerator, this);
  _dispatcher.add("storeFrames", &CoRDIADevice::storeFrames, this);
  _dispatcher.add("printFrames", &CoRDIADevice::printFrames, this);
  _dispatcher.add("setFreq", &CoRDIADevice::setFreq, this);

  // Set up periphery
  _periphery.add("VDDA", carboard::PWR_OUT_1);
  _periphery.add("VDDPA", carboard::PWR_OUT_2);
  _periphery.add("VDDD", carboard::PWR_OUT_3);
  _periphery.add("VDD33A", carboard::PWR_OUT_4);
  _periphery.add("VDD33B", carboard::PWR_OUT_5);

  _periphery.add("COMP_VREF", carboard::BIAS_1);
  _periphery.add("BPLATE1", carboard::BIAS_2);
  _periphery.add("BPLATE2", carboard::BIAS_3);
  _periphery.add("VREF_P", carboard::BIAS_4);
  _periphery.add("VREF_M", carboard::BIAS_5);

  _periphery.add("ISOURCE", carboard::CUR_1);
  _periphery.add("COMP_ITAIL", carboard::CUR_2);
  _periphery.add("CDS_IBP", carboard::CUR_3);

  _periphery.add("PULSEC", carboard::INJ_1);

  _periphery.add("CMOS_OUT_1_TO_4", carboard::CMOS_OUT_1_TO_4);
  _periphery.add("CMOS_OUT_5_TO_8", carboard::CMOS_OUT_5_TO_8);
  _periphery.add("CMOS_IN_1_TO_4", carboard::CMOS_IN_1_TO_4);
  _periphery.add("CMOS_IN_5_TO_8", carboard::CMOS_IN_5_TO_8);
  _periphery.add("CMOS_IN_9_TO_12", carboard::CMOS_IN_9_TO_12);
  _periphery.add("CMOS_IN_13_TO_14", carboard::CMOS_IN_13_TO_14);

  // Add the register definitions to the dictionary for convenient lookup of names:
  _registers.add(CORDIA_CHIP_REGS);

  // Add memory pages to the dictionary:
  _memory.add(CORDIA_FPGA_REGS);
  _memory.add(CORDIA_PG_REGS);

  // Reset the PG and SDI:
  this->setMemory("pg_cmd_rst", 1);
  this->setMemory("sdi_cmd_rst", 1);
}

void CoRDIADevice::powerStatusLog() {
  LOG(INFO) << "Power status:";

  LOG(INFO) << "VDDA:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("VDDA") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("VDDA") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("VDDA") << "W";

  LOG(INFO) << "VDDPA:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("VDDPA") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("VDDPA") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("VDDPA") << "W";

  LOG(INFO) << "VDDD:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("VDDD") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("VDDD") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("VDDD") << "W";

  LOG(INFO) << "VDD33A:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("VDD33A") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("VDD33A") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("VDD33A") << "W";

  LOG(INFO) << "VDD33B:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("VDD33B") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("VDD33B") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("VDD33B") << "W";
}

void CoRDIADevice::configure() {

  LOG(INFO) << "Configuring";

  // Reset the PG and SDI:
  this->setMemory("pg_cmd_rst", 1);
  this->setMemory("sdi_cmd_rst", 1);
  usleep(1000);

  // Configure everything from the config files and write default values:
  CaribouDevice<carboard::Carboard, iface_mem>::configure();
  LOG(DEBUG) << "Writing configuration bits to the chip";
  writeConfig();

  // Read pattern generator from the configuration and program it:
  std::string pg = _config.Get("patterngenerator", "");
  if(!pg.empty()) {
    LOG(INFO) << "Found pattern generator file in configuration: \"" << pg << "\"...";
    configurePatternGenerator(pg);
  } else {
    LOG(INFO) << "No pattern generator file found in configuration.";
  }
}

template <typename Enumeration> auto as_value(Enumeration const value) -> typename std::underlying_type<Enumeration>::type {
  return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

void CoRDIADevice::setSpecialRegister(const std::string& name, uintptr_t value) {
  LOG(DEBUG) << "Setting special register \"" << name << "\" with \"" << value << "\"";

  if(name == "pg_freq") {
    setFreq(value);
    return;
  }

  // if(name == "") {
  //   return;
  // }

  LOG(WARNING) << "Setting of special register \"" << name << "\" is not implemented!";
  return;
}

uintptr_t CoRDIADevice::getSpecialRegister(const std::string& name) {
  LOG(DEBUG) << "Reading special register \"" << name << "\"";
  uintptr_t value = 0;
  // if(name == "") {
  //   return value;
  // }
  LOG(WARNING) << "Reading of special register \"" << name << "\" is not implemented!";
  return value;
}

CoRDIADevice::~CoRDIADevice() {
  LOG(INFO) << "Shutdown, delete device.";
  powerOff();
}

void CoRDIADevice::setFreq(const uint64_t freq) {
  if(freq > 200000000) {
    LOG(ERROR) << "Maximum allowed frequency for CoRDIA pattern generator is 200MHz. Can not set " << freq << "Hz.";
    return;
  }

  LOG(DEBUG) << "Unbinding Linux driver for Si570";
  std::ofstream dfout;
  dfout.open("/sys/bus/i2c/drivers/si570/unbind", std::ios_base::app);
  if(dfout.fail()) {
    LOG(INFO) << "Can not unbind driver. Maybe it is not needed.";
  } else {
    dfout.write("1-005d", 6);
    dfout.close();
  }

  LOG(INFO) << "Disabling clock buffer.";
  setMemory("pg_clk_ce", 0);
  _hal->setUSRCLK(freq);
  LOG(INFO) << "Enabling clock buffer.";
  setMemory("pg_clk_ce", 1);
}

void CoRDIADevice::writeConfig() {
  LOG(INFO) << "Writing configuration into the chip.";
  this->setMemory("sdi_cmd_write_config", 1);

  // check for chip configuration to be completed
  size_t cnt = 0;
  usleep(1000);
  while(getMemory("sdi_stat_write_done") == 0) {
    // abort after 5 iterations
    if(cnt == 5) {
      LOG(ERROR) << "Timeout, chip configuration failed!";
      return;
    }
    cnt++;
    usleep(1000);
  }
  LOG(INFO) << "Configuration written.";
  if(getMemory("sdi_err_sanity_check") == 1) {
    LOG(WARNING) << "Configuration readout sanity check failed!";
  }
}

void CoRDIADevice::powerUp() {
  LOG(INFO) << "Powering up";

  LOG(DEBUG) << "Switching CMOS signals level shifters ON";
  LOG(DEBUG) << " CMOS_OUT_1_TO_4: " << _config.Get("cmos_out_1_to_4_voltage", CoRDIA_CMOS_OUT_1_TO_4) << "V";
  this->setVoltage("CMOS_OUT_1_TO_4", _config.Get("cmos_out_1_to_4_voltage", CoRDIA_CMOS_OUT_1_TO_4));
  this->switchOn("CMOS_OUT_1_TO_4");

  LOG(DEBUG) << " CMOS_OUT_5_TO_8: " << _config.Get("cmos_out_5_to_8_voltage", CoRDIA_CMOS_OUT_5_TO_8) << "V";
  this->setVoltage("CMOS_OUT_5_TO_8", _config.Get("cmos_out_5_to_8_voltage", CoRDIA_CMOS_OUT_5_TO_8));
  this->switchOn("CMOS_OUT_5_TO_8");

  LOG(DEBUG) << " CMOS_IN_1_TO_4: " << _config.Get("cmos_in_1_to_4_voltage", CoRDIA_CMOS_IN_1_TO_4) << "V";
  this->setVoltage("CMOS_IN_1_TO_4", _config.Get("cmos_in_1_to_4_voltage", CoRDIA_CMOS_IN_1_TO_4));
  this->switchOn("CMOS_IN_1_TO_4");

  LOG(DEBUG) << " CMOS_IN_5_TO_8: " << _config.Get("cmos_in_5_to_8_voltage", CoRDIA_CMOS_IN_5_TO_8) << "V";
  this->setVoltage("CMOS_IN_5_TO_8", _config.Get("cmos_in_5_to_8_voltage", CoRDIA_CMOS_IN_5_TO_8));
  this->switchOn("CMOS_IN_5_TO_8");

  LOG(DEBUG) << " CMOS_IN_9_TO_12: " << _config.Get("cmos_in_9_to_12_voltage", CoRDIA_CMOS_IN_9_TO_12) << "V";
  this->setVoltage("CMOS_IN_9_TO_12", _config.Get("cmos_in_9_to_12_voltage", CoRDIA_CMOS_IN_9_TO_12));
  this->switchOn("CMOS_IN_9_TO_12");

  LOG(DEBUG) << " CMOS_IN_13_TO_14: " << _config.Get("cmos_in_13_to_14_voltage", CoRDIA_CMOS_IN_13_TO_14) << "V";
  this->setVoltage("CMOS_IN_13_TO_14", _config.Get("cmos_in_13_to_14_voltage", CoRDIA_CMOS_IN_13_TO_14));
  this->switchOn("CMOS_IN_13_TO_14");

  LOG(INFO) << "Switching power supplies ON";

  LOG(DEBUG) << " VDDA: " << _config.Get("vdda", CoRDIA_VDDA) << "V";
  this->setVoltage("VDDA", _config.Get("vdda", CoRDIA_VDDA), _config.Get("vdda_current", CoRDIA_VDDA_CURRENT));
  this->switchOn("VDDA");

  LOG(DEBUG) << " VDDPA: " << _config.Get("vddpa", CoRDIA_VDDPA) << "V";
  this->setVoltage("VDDPA", _config.Get("vddpa", CoRDIA_VDDPA), _config.Get("vddpa_current", CoRDIA_VDDPA_CURRENT));
  this->switchOn("VDDPA");

  LOG(DEBUG) << " VDDD: " << _config.Get("vddd", CoRDIA_VDDD) << "V";
  this->setVoltage("VDDD", _config.Get("vddd", CoRDIA_VDDD), _config.Get("vddd_current", CoRDIA_VDDD_CURRENT));
  this->switchOn("VDDD");

  LOG(DEBUG) << " VDD33A: " << _config.Get("vdd33a", CoRDIA_VDD33A) << "V";
  this->setVoltage("VDD33A", _config.Get("vdd33a", CoRDIA_VDD33A), _config.Get("vdd33a_current", CoRDIA_VDD33A_CURRENT));
  this->switchOn("VDD33A");

  LOG(DEBUG) << " VDD33B: " << _config.Get("vdd33b", CoRDIA_VDD33B) << "V";
  this->setVoltage("VDD33B", _config.Get("vdd33b", CoRDIA_VDD33B), _config.Get("vdd33b_current", CoRDIA_VDD33B_CURRENT));
  this->switchOn("VDD33B");

  LOG(DEBUG) << " VDDPA: " << _config.Get("vddpa", CoRDIA_VDDPA) << "V";
  this->setVoltage("VDDPA", _config.Get("vddpa", CoRDIA_VDDPA), _config.Get("vddpa_current", CoRDIA_VDDPA_CURRENT));
  this->switchOn("VDDPA");

  LOG(DEBUG) << " VDDPA: " << _config.Get("vddpa", CoRDIA_VDDPA) << "V";
  this->setVoltage("VDDPA", _config.Get("vddpa", CoRDIA_VDDPA), _config.Get("vddpa_current", CoRDIA_VDDPA_CURRENT));
  this->switchOn("VDDPA");

  LOG(INFO) << "Switching BIAS voltages ON";

  LOG(DEBUG) << "COMP_VREF bias voltage: " << _config.Get("comp_vref", CoRDIA_COMP_VREF) << "V";
  this->setVoltage("COMP_VREF", _config.Get("comp_vref", CoRDIA_COMP_VREF));
  this->switchOn("COMP_VREF");

  LOG(DEBUG) << "BPLATE1 bias voltage: " << _config.Get("bplate1", CoRDIA_BPLATE1) << "V";
  this->setVoltage("BPLATE1", _config.Get("bplate1", CoRDIA_BPLATE1));
  this->switchOn("BPLATE1");

  LOG(DEBUG) << "BPLATE2 bias voltage: " << _config.Get("bplate2", CoRDIA_BPLATE2) << "V";
  this->setVoltage("BPLATE2", _config.Get("bplate2", CoRDIA_BPLATE2));
  this->switchOn("BPLATE2");

  LOG(DEBUG) << "VREF_P bias voltage: " << _config.Get("vref_p", CoRDIA_VREF_P) << "V";
  this->setVoltage("VREF_P", _config.Get("vref_p", CoRDIA_VREF_P));
  this->switchOn("VREF_P");

  LOG(DEBUG) << "VREF_M bias voltage: " << _config.Get("vref_m", CoRDIA_VREF_M) << "V";
  this->setVoltage("VREF_M", _config.Get("vref_m", CoRDIA_VREF_M));
  this->switchOn("VREF_M");

  LOG(INFO) << "Switching BIAS currents ON";

  LOG(DEBUG) << "ISOURCE bias current: " << static_cast<unsigned int>(_config.Get("isource", CoRDIA_ISOURCE))
             << "A, direction " << static_cast<unsigned int>(_config.Get("isource_dir", CoRDIA_ISOURCE_DIR));
  this->setCurrent("ISOURCE",
                   static_cast<unsigned int>(_config.Get("isource", CoRDIA_ISOURCE)),
                   static_cast<unsigned int>(_config.Get("isource_dir", CoRDIA_ISOURCE_DIR)));
  this->switchOn("ISOURCE");

  LOG(DEBUG) << "COMP_ITAIL bias current: " << static_cast<unsigned int>(_config.Get("comp_itail", CoRDIA_COMP_ITAIL))
             << "A, direction " << static_cast<unsigned int>(_config.Get("comp_itail_dir", CoRDIA_COMP_ITAIL_DIR));
  this->setCurrent("COMP_ITAIL",
                   static_cast<unsigned int>(_config.Get("comp_itail", CoRDIA_COMP_ITAIL)),
                   static_cast<unsigned int>(_config.Get("comp_itail_dir", CoRDIA_COMP_ITAIL_DIR)));
  this->switchOn("COMP_ITAIL");

  LOG(DEBUG) << "CDS_IBP bias current: " << static_cast<unsigned int>(_config.Get("cds_ibp", CoRDIA_CDS_IBP))
             << "A, direction " << static_cast<unsigned int>(_config.Get("cds_ibp_dir", CoRDIA_CDS_IBP_DIR));
  this->setCurrent("CDS_IBP",
                   static_cast<unsigned int>(_config.Get("cds_ibp", CoRDIA_CDS_IBP)),
                   static_cast<unsigned int>(_config.Get("cds_ibp_dir", CoRDIA_CDS_IBP_DIR)));
  this->switchOn("CDS_IBP");

  LOG(INFO) << "Setting pulser voltage";

  LOG(DEBUG) << "PULSEC voltage: " << _config.Get("pulsec", CoRDIA_PULSEC) << "V";
  this->setVoltage("PULSEC", _config.Get("pulsec", CoRDIA_PULSEC));
  // this->switchOn("PULSEC");

  return;
}

void CoRDIADevice::powerDown() {
  LOG(INFO) << "Powering down";

  LOG(INFO) << "Switching pulser OFF";
  this->switchOff("PULSEC");

  LOG(INFO) << "Switching BIAS currents OFF";
  this->switchOff("CDS_IBP");
  this->switchOff("COMP_ITAIL");
  this->switchOff("ISOURCE");

  LOG(INFO) << "Switching BIAS voltages OFF";
  this->switchOff("VREF_M");
  this->switchOff("VREF_P");
  this->switchOff("BPLATE2");
  this->switchOff("BPLATE1");
  this->switchOff("COMP_VREF");

  LOG(INFO) << "Switching power supplies OFF";
  this->switchOff("VDD33B");
  this->switchOff("VDD33A");
  this->switchOff("VDDD");
  this->switchOff("VDDPA");
  this->switchOff("VDDA");

  LOG(DEBUG) << "Switching CMOS signals level shifters OFF";
  this->switchOff("CMOS_OUT_1_TO_4");
  this->switchOff("CMOS_OUT_5_TO_8");
  this->switchOff("CMOS_IN_1_TO_4");
  this->switchOff("CMOS_IN_5_TO_8");
  this->switchOff("CMOS_IN_9_TO_12");
  this->switchOff("CMOS_IN_13_TO_14");

  return;
}

void CoRDIADevice::daqStart() {
  LOG(INFO) << "Starting DAQ: Enabling readout and pattern generator.";
  setMemory("data_fifo_rst", 0x1);
  setMemory("pg_cmd_rst", 0x1);
  setMemory("pg_conf_number_of_runs", 0x0);
  setMemory("readout_en", 0x1);
  setMemory("pg_cmd_start", 0x1);
  LOG(INFO) << "DAQ started.";
}

void CoRDIADevice::daqStop() {

  setMemory("pg_cmd_stop", 0x1);
  unsigned int attempts = 0;
  while(getMemory("pg_stat_running") == 1) {
    if(attempts++ >= 10) {
      LOG(WARNING) << "Timeout waiting for pattern generator to finish the cycle.";
      setMemory("pg_cmd_rst", 0x1);
    }
    usleep(100);
  }
  setMemory("readout_en", 0x0);
  LOG(INFO) << "DAQ stopped.";
}

pearyRawData CoRDIADevice::getFrame() {
  pearyRawData rawdata;

  // data structure:
  // {1'(data_valid), 2'b00, 1'(is_meta), 1'(gn_sample), 11'(ADC_data)}
  // 0b V00M GAAA AAAA AAAA

  uintptr_t data;
  while(1) {
    unsigned int attempts = 0;
    // Check if there is something available
    while(((data = getMemory("data_out")) == 0)) {
      if(attempts++ >= 10) {
        LOG(DEBUG) << "No data available in the readout memory.";
        // if something is already in the frame
        if(rawdata.size() > 0) {
          // insert artificial frame end
          data = 0x9001;
          break;
        } else {
          throw NoDataAvailable();
        }
      }
      usleep(100);
    }
    if(data & 0x1000) {
      // end of frame
      LOG(DEBUG) << "End of frame detected.";
      break;
    } else {
      rawdata.push_back((data & 0x7FFF));
    }
  }
  LOG(DEBUG) << "Read " << rawdata.size() << " pixels.";
  return rawdata;
}

pearyRawData CoRDIADevice::getRawData() {
  return getFrame();
}

pearydata CoRDIADevice::getData() {
  LOG(INFO) << "CoRDIADevice::getData() not implemented.";
  pearydata data;
  return data;
}

void CoRDIADevice::storeFrames(uint32_t N, std::string filename) {

  // reset readout
  this->setMemory("readout_en", 0);
  usleep(10);

  if(N == 0) {
    LOG(ERROR) << "Number of requested frames must be greater than 0.";
    return;
  }

  LOG(STATUS) << "Running pattern generator " << N << " times and storing data frames into \"" << filename << "\"";

  std::ofstream data_out;
  data_out.open(filename.c_str(), std::ios_base::out | std::ofstream::binary);

  if(data_out.fail()) {
    throw DeviceException("Can't open output file.\n");
  }

  this->setMemory("readout_en", 1);
  LOG(DEBUG) << "Readout enabled.";
  startPatternGenerator(N);

  uint32_t framecount = 0;

  // get data
  while(true) {
    try {
      auto frame = getFrame();
      uint16_t length = static_cast<uint16_t>(frame.size() * 2);
      LOG(DEBUG) << "Writing frame with " << frame.size() << " pixels.";
      data_out.write(reinterpret_cast<const char*>(&length), 2);
      for(const auto& pixel : frame) {
        LOG(DEBUG) << to_hex_string(pixel, 4, true);
        data_out.write(reinterpret_cast<const char*>(&pixel), 2);
      }
      framecount++;
    } catch(caribou::NoDataAvailable& e) {
      LOG(STATUS) << framecount << " frames were stored to file \"" << filename << "\"";
      break;
    }
  }

  // stop
  this->setMemory("readout_en", 0);
  data_out.close();
}

void CoRDIADevice::printFrames(uint32_t N) {

  // reset readout
  this->setMemory("readout_en", 0);
  usleep(10);

  if(N == 0) {
    LOG(ERROR) << "Number of requested frames must be greater than 0.";
    return;
  }

  LOG(STATUS) << "Running pattern generator " << N << " times and printing frames.";

  this->setMemory("readout_en", 1);
  LOG(DEBUG) << "Readout enabled.";
  startPatternGenerator(N);

  uint32_t framecount = 0;

  // get data
  while(true) {
    try {
      auto frame = getFrame();
      LOG(INFO) << "Frame no. " << ++framecount << ", size " << frame.size() << ".";
      for(const auto& pixel : frame) {
        LOG(INFO) << to_bit_string(pixel, 12, true);
      }
    } catch(caribou::NoDataAvailable& e) {
      LOG(STATUS) << "Read " << framecount << " frames.";
      break;
    }
  }

  // stop
  this->setMemory("readout_en", 0);
}

void CoRDIADevice::startPatternGenerator(uint32_t runs) {
  if(runs > CORDIA_PG_PATTERN_MAX_RUNS) {
    LOG(WARNING) << "Maximum number of PG runs is " << CORDIA_PG_PATTERN_MAX_RUNS << ", requested was " << runs
                 << ". Trimming to max.";
    runs = CORDIA_PG_PATTERN_MAX_RUNS;
  }
  if(runs) {
    LOG(DEBUG) << "Setting pattern generator for " << runs << " runs.";
  } else {
    LOG(DEBUG) << "Setting pattern generator for infinite running.";
  }
  setMemory("pg_conf_number_of_runs", runs);

  LOG(INFO) << "Starting pattern generator";
  setMemory("pg_cmd_start", 0x1);
}

void CoRDIADevice::stopPatternGenerator() {
  LOG(INFO) << "Stopping pattern generator";
  setMemory("pg_cmd_stop", 0x1);
}

void CoRDIADevice::configurePatternGenerator(std::string filename) {

  LOG(DEBUG) << "Reading pattern generator configuration file \"" << filename << "\"";

  std::ifstream pgfile(filename);
  if(!pgfile.is_open()) {
    LOG(ERROR) << "Could not open pattern generator configuration file \"" << filename << "\"";
    throw ConfigInvalid("Could not open pattern generator configuration file \"" + filename + "\"");
  }

  std::vector<uint32_t> patterns;
  std::string line = "";
  uint32_t lineNum = 0;
  bool header = true;

  // read line by line
  while(std::getline(pgfile, line)) {
    lineNum++;
    // ignore empty lines and comments
    if(!line.length() || '#' == line.at(0)) {
      LOG(DEBUG) << "Skipping comment or empty line no. " << lineNum;
      continue;
    }
    // iterate through characters on a line
    uint32_t patternLine = 0;
    uint8_t patternLength = 0;
    for(char const& c : line) {
      if(c == '0') {
        patternLength++;
        patternLine = patternLine << 1;
      } else if(c == '1') {
        patternLength++;
        patternLine = (patternLine << 1) | 0x00000001;
      } else if(c == ' ' || c == ';' || c == ',' || c == '\t') {
        continue;
      } else {
        // this line is considered to be a header
        patternLength = 0;
        break;
      }
    }

    // we ignore table header, but if it was already read and we have unexpected characters, it's suspicious
    if(!patternLength) {
      LOG(DEBUG) << "Found header on line " << lineNum;
      if(!header) {
        LOG(WARNING) << "The pattern generator file \"" << filename << "\" seems to have wrong format on line " << lineNum;
      }
      continue;
    }
    if(patternLength != CORDIA_PG_PATTERN_WIDTH) {
      LOG(ERROR) << "Error parsing pattern generator configuration file on line " << lineNum << ": Pattern line length ("
                 << patternLength << ") does not match the expected length (" << CORDIA_PG_PATTERN_WIDTH << ").";
      LOG(ERROR) << "Pattern generator not configured";
      return;
    }
    header = false;
    LOG(DEBUG) << "Read pattern " << to_bit_string(patternLine, CORDIA_PG_PATTERN_WIDTH, true) << " on line " << lineNum;
    patterns.push_back(patternLine);
  }
  // end reading configuration file
  // check what we've read
  if(patterns.size() == 0) {
    LOG(ERROR) << "Pattern generator file does not contain any valid pattern.";
    LOG(ERROR) << "Pattern generator not configured";
    return;
  }
  if(patterns.size() > CORDIA_PG_PATTERN_MAX_LENGTH) {
    LOG(ERROR) << "Pattern generator file is too long. Read " << patterns.size() << " patterns from file, HW capacity is "
               << CORDIA_PG_PATTERN_MAX_LENGTH << ". Surplus patterns will be stripped.";
    patterns.resize(CORDIA_PG_PATTERN_MAX_LENGTH);
  }

  LOG(DEBUG) << "Resetting pattern generator";
  setMemory("pg_cmd_rst", 0x1);

  LOG(INFO) << "Loading pattern generator memory with " << patterns.size() << " patterns from file \"" << filename << "\"";

  for(uint32_t i = 0; i < patterns.size(); i++) {
    LOG(DEBUG) << "Writing position " << i << " with pattern \""
               << to_bit_string(patterns.at(i), CORDIA_PG_PATTERN_WIDTH, true) << "\"";
    setMemory("pg_mem_base", (i << CORDIA_CORE_LSB), patterns.at(i));
  }

  LOG(DEBUG) << "Setting pattern size to " << patterns.size();
  setMemory("pg_conf_pattern_length", patterns.size() - 1);

  LOG(INFO) << "Pattern generator loaded.";
  return;
}
