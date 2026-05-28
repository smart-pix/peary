/**
 * Caribou implementation for the H2M
 */

#include "H2MDevice.hpp"
#include "utils/log.hpp"

#include <fstream>

using namespace caribou;

H2MDevice::H2MDevice(const caribou::Configuration config)
    : CaribouDevice(config,
                    iface_sermem_config(MEM_PATH,
                                        {CORE_BASE_ADDRESS, 5, PROT_READ | PROT_WRITE},
                                        H2M_FPGA_REG_SC_IFACE_ADDR_WRITE,
                                        H2M_FPGA_REG_SC_IFACE_DATA_WRITE,
                                        H2M_FPGA_REG_SC_IFACE_ADDR_READ,
                                        H2M_FPGA_REG_SC_IFACE_DATA_RECEIVED,
                                        H2M_FPGA_REG_SC_IFACE_STATUS)) {

  // Regsiter device-specific commands:
  _dispatcher.add("powerStatusLog", &H2MDevice::powerStatusLog, this);
  _dispatcher.add("triggerPatternGenerator", &H2MDevice::triggerPatternGenerator, this);
  _dispatcher.add("logStatusFE", &H2MDevice::logStatusFE, this);
  _dispatcher.add("configureMatrix", &H2MDevice::configureMatrix, this);
  _dispatcher.add("programMask", &H2MDevice::programMatrix, this);
  _dispatcher.add("configureClock", &H2MDevice::configureClock, this);
  _dispatcher.add("configurePG", &H2MDevice::configurePatternGenerator, this);
  _dispatcher.add("configureTP", &H2MDevice::configureTP, this);
  _dispatcher.add("startPG", &H2MDevice::startPatternGenerator, this);
  _dispatcher.add("stopPG", &H2MDevice::stopPatternGenerator, this);
  _dispatcher.add("printFrames", &H2MDevice::printFrames, this);
  _dispatcher.add("testpulse", &H2MDevice::testpulse, this);
  _dispatcher.add("parameterScan", &H2MDevice::parameterScan, this);
  _dispatcher.add("takeFrames", &H2MDevice::takeFrames, this);
  _dispatcher.add("generateMask_human", &H2MDevice::generateMask_human, this);
  _dispatcher.add("readFIFOData", &H2MDevice::print_fifo_data, this);

  // Set up periphery
  _periphery.add("vddd", carboard::PWR_OUT_2);
  _periphery.add("vdda", carboard::PWR_OUT_3);
  _periphery.add("lvds", carboard::PWR_OUT_6); // 3.3V for LVDS buffers
  _periphery.add("v_circuit",
                 carboard::PWR_OUT_5);         // 3.3V for PWELL/SUB circuit
  _periphery.add("v_io", carboard::PWR_OUT_8); //  IO bias voltage
  _periphery.add("v_sub",
                 carboard::BIAS_3);            //  SUB control voltage (SUB voltage is -2x)
  _periphery.add("v_pwell", carboard::BIAS_4); //  PWELL control voltage (PWELL
                                               // voltage is -2x)
  _periphery.add("read_analog_out", carboard::VOL_IN_8);

  // Add the register definitions to the dictionary for convenient lookup of names:
  _registers.add(H2M_REGISTERS);

  // Add memory pages to the dictionary:
  _memory.add(H2M_MEMORY);

  // reset FPGA
  this->setMemory("fpga_reset", 1);
  mDelay(5);
}

void H2MDevice::print_fifo_data() {
  auto data = read_fifo_data(H2M_FW_EVENTSIZE);
  for(size_t i = 0; i < data.size(); i++) {
    LOG(INFO) << "Word: " << i << "\t 0x" << std::hex << data.at(i) << std::dec;
  }
}

pearyRawData H2MDevice::read_fifo_data(unsigned int wordCount) {
  pearyRawData data;
  data.reserve(wordCount);
  for(unsigned int i = 0; i < wordCount; ++i) {
    data.emplace_back(this->getMemory("data"));
  }
  return data;
}

void H2MDevice::setSpecialRegister(const std::string& name, uintptr_t value) {
  if(name == "matrix_ctrl") {
    // Resolve name against register dictionary:
    auto reg = _registers.get(name);
    if(value == 1) {
      LOG(DEBUG) << "Starting testpulse - Chip will respond again only when pulsing is finished.";
    }
    try {
      LOG(DEBUG) << "Writing special register matrix_ctrl";
      this->process_register_write(reg, value);
    } catch(CommunicationError&) {
      LOG(DEBUG) << "matrix_ctrl write status received: " << to_bit_string(this->getMemory("sc_status"), 4);
    }
  }
  return;
}

void H2MDevice::configure() {

  // Clock configuration first
  configureClock(_config.Get<bool>("clock_internal", true));
  mDelay(10);

  // Configure pattern generator:
  if(_config.Has("pg_file")) {
    LOG(INFO) << "Configuring pattern generator from file \"" << _config.Get<std::string>("pg_file") << "\"";
    configurePatternGenerator(_config.Get<std::string>("pg_file"));
  } else {
    LOG(WARNING) << "Not configuring any pattern generator, no file provided in configuration (pg_file)";
  }

  LOG(INFO) << "Configuring H2M device...";
  // Make sure we are in direct SC mode before clearing the matrix
  setMemory("sc_ctrl_mode", 0);
  // Be careful here: To allow setting values from the config file as well as defaults for registers & memory, we need to
  // call the base class configure(). This method will however also set sc_ctrl_mode - which disables configuration.
  // The order is in that method is: first, registers are configured, then FPGA memory is configured (potentially setting
  // sc_ctrl_mode = 1) - with this order we are fine.

  chipReset();
  mDelay(10);

  // Configure pixel matrix
  LOG(INFO) << "\tConfiguring pixel matrix";
  // Read matrix file from the configuration and program it:
  std::string maskFileName = _config.Get("mask_filename", "");
  if(!maskFileName.empty()) {
    LOG(INFO) << "\tFound mask file name in configuration: \"" << maskFileName << "\"...";
    configureMatrix(maskFileName);
  } else {
    LOG(INFO) << "\tNo mask file name found in configuration.";
    LOG(WARNING) << "\tUsing default pixel configuration";
    h2m_pixel_conf default_config = h2m_pixel_conf();
    if(matrixMask_.empty()) {
      generateMask(default_config.GetData());
    }
    configureMatrix();
  }

  // Configure everything from the config files and write default values (if any defined):
  CaribouDevice<carboard::Carboard, iface_sermem>::configure();

  // Obtain value of "force read from fifo" flag from config file
  force_read_from_fifo_ = static_cast<bool>(_config.Get("force_read_from_fifo", 0));
  LOG(INFO) << "Force read from FIFO:\t" << force_read_from_fifo_ << ".";

  // Again forcing SC mode to zero - this should only be set to 1 when running the daq
  setMemory("sc_ctrl_mode", 0);
  LOG(INFO) << "  Done";
}

void H2MDevice::chipReset() {
  LOG(INFO) << "Resetting chip";
  this->setMemory("chip_RESETn", 0);
  mDelay(1); // at least 8*25ns (40 MHz)
  this->setMemory("chip_RESETn", 1);
  return;
}

void H2MDevice::configureTP() {

  LOG(INFO) << "Setting up for test pulse injection using configuration file:";

  setRegister("tp_polarity", _config.Get("tp_polarity", _registers.get("tp_polarity").value()));
  getRegister("tp_polarity");
  setRegister("link_shutter_tp", _config.Get("link_shutter_tp", static_cast<uintptr_t>(1)));
  setRegister("shutter_tp_lat", _config.Get("shutter_tp_lat", _registers.get("shutter_tp_lat").value()));
  size_t shutter_tp_lat = getRegister("shutter_tp_lat");
  getRegister("shutter_tp_lat");
  setRegister("tp_num", _config.Get("tp_num", _registers.get("tp_num").value()));
  size_t tp_num = getRegister("tp_num");
  setRegister("tp_on", _config.Get("tp_on", _registers.get("tp_on").value()));
  size_t tp_on = getRegister("tp_on");
  setRegister("tp_off", _config.Get("tp_off", _registers.get("tp_off").value()));
  size_t tp_off = getRegister("tp_off");
  setRegister("dac_vtpulse", _config.Get("dac_vtpulse", _registers.get("dac_vtpulse").value()));
  getRegister("dac_vtpulse");
  setRegister("acq_mode", _config.Get("acq_mode", _registers.get("acq_mode").value()));
  getRegister("acq_mode");

  // check if the configured test-pulse sequence fits into the shutter
  // test-pulse duration is in slow control clock cycles (25 ns)
  // shutter duration is in daq clock cycles (10 ns)
  size_t shutter_duration = _config.Get("shutter", _memory.get("shutter").second.value());
  if((shutter_tp_lat + (tp_on + tp_off) * tp_num) * 25 > shutter_duration * 10) {
    LOG(WARNING) << "test-pulse sequence longer than shutter duration (in ns) "
                 << (shutter_tp_lat + (tp_on + tp_off) * tp_num) * 25 << " > " << shutter_duration * 10;
  }
}

template <typename Enumeration> auto as_value(Enumeration const value) -> typename std::underlying_type<Enumeration>::type {
  return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

void H2MDevice::startPatternGenerator(uint32_t runs) {
  LOG(DEBUG) << "Setting number of PG runs to " << runs;
  setMemory("pg_runs", runs);

  LOG(DEBUG) << "Starting pattern generator.";
  setMemory("pg_start", 1);
}

void H2MDevice::stopPatternGenerator() {
  LOG(DEBUG) << "Stopping pattern generator.";
  setMemory("pg_stop", 1);
}

void H2MDevice::triggerPatternGenerator(bool sleep) {

  LOG(DEBUG) << "Triggering pattern generator once.";

  // Enable FIFO readout mode:
  setMemory("sc_ctrl_mode", 1);
  startPatternGenerator(1);

  if(sleep) {
    while(getMemory("pg_running")) {
      usleep(100);
    }
  }
  // Disable FIFO readout mode:
  setMemory("sc_ctrl_mode", 0);
}

void H2MDevice::configurePatternGenerator(std::string filename) {
  LOG(DEBUG) << "Resetting pattern generator configuration";
  setMemory("pg_rst", 0x1);

  LOG(DEBUG) << "Programming pattern generator";
  std::vector<uint32_t> patterns;

  std::ifstream pgfile(filename);
  if(!pgfile.is_open()) {
    LOG(ERROR) << "Could not open pattern generator configuration file \"" << filename << "\"";
    throw ConfigInvalid("Could not open pattern generator configuration file \"" + filename + "\"");
  }

  enum class TriggerConditionGlobal : uint8_t {
    OR = 0b001,
    NOR = 0b101,
    AND = 0b011,
    NAND = 0b111,
    XOR = 0b010,
    XNOR = 0b110,
    TRUE = 0b000
  };

  enum class TriggerConditionLocal : uint8_t {
    HIGH = 0b100,
    LOW = 0b101,
    RISING = 0b001,
    FALLING = 0b010,
    EDGE = 0b011,
    ALWAYS = 0b111,
    NEVER = 0b000
  };

  std::string line = "";
  while(std::getline(pgfile, line)) {
    if(!line.length() || '#' == line.at(0))
      continue;
    std::istringstream pgline(line);
    std::string triggerconditions;
    std::string condition;
    uint32_t triggertimeout;
    std::string signals;
    uint32_t duration;
    if(pgline >> triggerconditions >> condition >> triggertimeout >> signals >> duration) {

      uint8_t output = 0;
      std::stringstream ss(signals);
      while(ss.good()) {
        std::string substr;
        getline(ss, substr, ',');
        if(substr == "RO") {
          output |= H2M_PG_RO;
        } else if(substr == "SH") {
          output |= H2M_PG_SH;
        } else if(substr == "AQ") {
          output |= H2M_PG_AQ;
        } else if(substr == "BS") {
          output |= H2M_PG_BS;
        } else if(substr == "NONE") {
        } else {
          LOG(ERROR) << "Unrecognized pattern for pattern generator: " << substr << " - ignoring.";
        }
      }

      uint32_t triggers = 0;
      if(condition == "OR") {
        triggers = as_value(TriggerConditionGlobal::OR);
      } else if(condition == "NOR") {
        triggers = as_value(TriggerConditionGlobal::NOR);
      } else if(condition == "AND") {
        triggers = as_value(TriggerConditionGlobal::AND);
      } else if(condition == "NAND") {
        triggers = as_value(TriggerConditionGlobal::NAND);
      } else if(condition == "XOR") {
        triggers = as_value(TriggerConditionGlobal::XOR);
      } else if(condition == "XNOR") {
        triggers = as_value(TriggerConditionGlobal::XNOR);
      } else if(condition == "TRUE") {
        triggers = as_value(TriggerConditionGlobal::TRUE);
      } else {
        LOG(ERROR) << "Unrecognized global trigger condition for pattern generator: " << condition;
        throw ConfigInvalid("Invalid global trigger condition");
      }

      size_t n_triggers = 0;
      std::stringstream ss2(triggerconditions);
      while(ss2.good()) {
        std::string substr;
        getline(ss2, substr, ',');
        if(substr == "HIGH" || substr == "H") {
          triggers |= static_cast<uint32_t>(as_value(TriggerConditionLocal::HIGH) << (n_triggers + 1) * 3);
        } else if(substr == "LOW" || substr == "L") {
          triggers |= static_cast<uint32_t>(as_value(TriggerConditionLocal::LOW) << (n_triggers + 1) * 3);
        } else if(substr == "RISING" || substr == "R") {
          triggers |= static_cast<uint32_t>(as_value(TriggerConditionLocal::RISING) << (n_triggers + 1) * 3);
        } else if(substr == "FALLING" || substr == "F") {
          triggers |= static_cast<uint32_t>(as_value(TriggerConditionLocal::FALLING) << (n_triggers + 1) * 3);
        } else if(substr == "EDGE" || substr == "E") {
          triggers |= static_cast<uint32_t>(as_value(TriggerConditionLocal::EDGE) << (n_triggers + 1) * 3);
        } else if(substr == "ALWAYS" || substr == "A") {
          triggers |= static_cast<uint32_t>(as_value(TriggerConditionLocal::ALWAYS) << (n_triggers + 1) * 3);
        } else if(substr == "NEVER" || substr == "N") {
          triggers |= static_cast<uint32_t>(as_value(TriggerConditionLocal::NEVER) << (n_triggers + 1) * 3);
        } else {
          LOG(ERROR) << "Unrecognized signal trigger condition for pattern generator: " << substr << " - setting to NEVER.";
          triggers |= static_cast<uint32_t>(as_value(TriggerConditionLocal::NEVER) << (n_triggers + 1) * 3);
        }
        n_triggers++;
      }

      LOG(DEBUG) << "PG: setting pattern timeout " << triggertimeout << " clk";
      setMemory("pg_timeout_pattern", triggertimeout);
      LOG(DEBUG) << "PG: setting duration " << duration << " clk";
      setMemory("pg_timer_pattern", duration);
      LOG(DEBUG) << "PG: setting output " << to_bit_string(output, 8, true);
      setMemory("pg_output_pattern", output);
      LOG(DEBUG) << "PG: setting trigger conditions "
                 << to_bit_string(triggers, static_cast<int>((n_triggers + 1) * 3), true);
      setMemory("pg_triggers_pattern", triggers);

      // Trigger the write:
      setMemory("pg_write_pattern", 0x1);

      auto remaining = getMemory("pg_mem_remaining");
      LOG(INFO) << "Added slot with " << n_triggers << " trigger signals to pattern generator, " << remaining
                << " slots remaining";
    }
  }

  LOG(INFO) << "Done configuring pattern generator.";
}

void H2MDevice::configureMatrix(std::string filename) {

  // Update pixel config from mask file, if filename given,
  // otherwise only re-configure.
  if(!filename.empty()) {
    LOG(DEBUG) << "Configuring the pixel matrix from mask file \"" << filename << "\"";
    try {
      readMask(filename);
    } catch(caribou::ConfigInvalid& e) {
      LOG(WARNING) << e.what();
      LOG(WARNING) << "\tUsing default pixel configuration";
      h2m_pixel_conf default_config = h2m_pixel_conf();
      generateMask(default_config.GetData());
    }
  }

  // Write mask to carboard memory
  try {
    programMatrix();
  } catch(caribou::DataException& e) {
    LOG(ERROR) << e.what();
    throw CommunicationError("Matrix configuration failed");
  }

  // Clear the pixel matrix once, read the config back:
  auto rawdata = getRawDataSC();
  auto matrix_config = frame_decoder_.decodeFrame(rawdata, 0x0, false);
  // Validate that this is correct and matches what we have written, not decoding LFSR:
  for(size_t col = 0; col < H2M_NCOL; col++) {
    for(size_t row = 0; row < H2M_NROW; row++) {
      int cfg_in = static_cast<h2m_pixel*>(matrixMask_[std::make_pair(col, row)].get())->GetData();
      int cfg_out = static_cast<h2m_pixel*>(matrix_config[std::make_pair(col, row)].get())->GetData();
      if(cfg_in != cfg_out) {
        throw CommunicationError("Mismatch in matrix configuration readback, px " + std::to_string(static_cast<int>(col)) +
                                 "," + std::to_string(static_cast<int>(row)) + ": " + std::to_string(cfg_in) +
                                 " != " + std::to_string(cfg_out));
      }
    }
  }
}

void H2MDevice::readMask(std::string filename) {
  matrixMask_.clear();

  // to check if we have configured all pixels
  bool configured[H2M_NCOL][H2M_NROW] = {{0}};
  uint16_t n_configured = 0;

  // try to open matrix file
  std::ifstream maskfile(filename);
  if(!maskfile.is_open()) {
    throw ConfigInvalid("Could not open mask file \"" + filename + "\"");
  }

  size_t masked_pixels = 0;

  // getting lines from the matrix file, each should correspond to one pixel
  std::string line = "";
  while(std::getline(maskfile, line)) {

    // Skip empty and commented lines
    if(!line.length() || '#' == line.at(0)) {
      continue;
    }

    // read lines
    std::istringstream maskline(line);
    std::string tag;
    uint16_t col, row, tp_enable, mask, tuning_dac, not_top_pixel;

    if(maskline >> col >> row >> tp_enable >> mask >> tuning_dac >> not_top_pixel) {

      // check boundaries of column and row indices
      if(col >= H2M_NCOL) {
        throw ConfigInvalid("Trying to configure non-existing column " + to_string(col));
      }
      if(row >= H2M_NROW) {
        throw ConfigInvalid("Trying to configure non-existing row " + to_string(row));
      }

      // mark configured pixels to check completeness of configuration later
      configured[col][row] = true;
      // also count
      n_configured++;

      LOG(DEBUG) << "Read pixel configuration for col, row: " << col << ", " << row;
      LOG(DEBUG) << "\ttp_enable " << static_cast<bool>(tp_enable);
      LOG(DEBUG) << "\tmask " << static_cast<bool>(mask);
      LOG(DEBUG) << "\tnot_top_pixel " << static_cast<bool>(not_top_pixel);
      LOG(DEBUG) << "\ttuning duck " << tuning_dac;

      masked_pixels += (mask ? 1 : 0);
      // Fill data structure;
      // everything that is not null will be interpreted as true
      matrixMask_[std::make_pair(col, row)] = std::make_unique<h2m_pixel_conf>(
        static_cast<bool>(tp_enable), static_cast<bool>(mask), tuning_dac, static_cast<bool>(not_top_pixel));

    } // masklines
    else {
      throw ConfigInvalid("Unexpected line in mask file:/n" + line);
    }

  } // lines

  // to check if we have configured all pixels, and did not attempt to configure
  // too many
  for(uint16_t col = 0; col < H2M_NCOL; col++) {
    for(uint16_t row = 0; row < H2M_NROW; row++) {
      if(!configured[col][row]) {
        throw ConfigInvalid("Pixel " + to_string(col) + ", " + to_string(row) + " (col, row) not configured!");
      }
    }
  }
  if(n_configured != H2M_NCOL * H2M_NROW) {
    throw ConfigInvalid("Trying to configure  " + to_string(n_configured) + "pixels, expecting " +
                        to_string(H2M_NCOL * H2M_NROW) + "!");
  }

  LOG(INFO) << "Successfully read matrix file, have " << masked_pixels << " masked pixels";
  return;
}

void H2MDevice::generateMask(uint16_t pix_mask) {

  LOG(INFO) << "Writing " << static_cast<int>(pix_mask) << " to all pixels!";

  matrixMask_.clear();
  for(uint16_t col = 0; col < H2M_NCOL; col++) {
    for(uint16_t row = 0; row < H2M_NROW; row++) {
      // Fill data structure.
      matrixMask_[std::make_pair(col, row)] = std::make_unique<h2m_pixel_conf>(static_cast<uint8_t>(pix_mask));
    }
  }
}

void H2MDevice::generateMask_human(bool tp_enable, bool mask, uint16_t tdac_cli, bool not_top_pix) {

  auto tdac = static_cast<uint8_t>(tdac_cli);
  LOG(INFO) << "Writing " << h2m_pixel_conf(tp_enable, mask, tdac, not_top_pix).GetData() << " to all pixels!";

  matrixMask_.clear();
  for(uint16_t col = 0; col < H2M_NCOL; col++) {
    for(uint16_t row = 0; row < H2M_NROW; row++) {
      // Fill data structure.
      matrixMask_[std::make_pair(col, row)] = std::make_unique<h2m_pixel_conf>(tp_enable, mask, tdac, not_top_pix);
    }
  }
}

void H2MDevice::programMatrix() {

  // Get ready for configuration
  setRegister("conf", 1);
  setRegister("acq_start", 0);

  // Loop over all columns
  for(uint col = 0; col < H2M_NCOL; ++col) {
    std::string address = "px_cnt_w_" + std::to_string(col);
    uint32_t configWord = 0x0;
    for(int row = H2M_NROW - 1; row >= 0; --row) {
      auto px_config_value = reinterpret_cast<h2m_pixel*>(matrixMask_[std::make_pair(col, row)].get());
      configWord |= static_cast<uint32_t>(px_config_value->GetData()) << (8 * (row % H2M_NPXGROUP));
      if(row % H2M_NPXGROUP == 0) {
        this->setRegister(address, configWord);
        configWord = 0x0;
      }
    }
  }

  // Latch configuration to pixels
  setRegister("conf", 0);
  return;
}

void H2MDevice::powerUp() {
  LOG(INFO) << "Powering up";

  LOG(DEBUG) << " VDDA";
  this->setVoltage("vdda", _config.Get("vdda", H2M_VDDA), _config.Get("vdda_current", H2M_VDDA_CURRENT));
  this->switchOn("vdda");
  LOG(DEBUG) << "\tBus voltage: " << this->getVoltage("vdda") << "V";
  LOG(DEBUG) << "\tBus current: " << this->getCurrent("vdda") << "A";
  LOG(DEBUG) << "\tBus power  : " << this->getPower("vdda") << "W";

  LOG(DEBUG) << " VDDD";
  this->setVoltage("vddd", _config.Get("vddd", H2M_VDDD), _config.Get("vddd_current", H2M_VDDD_CURRENT));
  this->switchOn("vddd");
  LOG(DEBUG) << "\tBus voltage: " << this->getVoltage("vddd") << "V";
  LOG(DEBUG) << "\tBus current: " << this->getCurrent("vddd") << "A";
  LOG(DEBUG) << "\tBus power  : " << this->getPower("vddd") << "W";

  LOG(DEBUG) << " LVDS";
  this->setVoltage("lvds", _config.Get("lvds", H2M_LVDS), _config.Get("lvds_current", H2M_LVDS_CURRENT));
  this->switchOn("lvds");
  LOG(DEBUG) << "\tBus voltage: " << this->getVoltage("lvds") << "V";
  LOG(DEBUG) << "\tBus current: " << this->getCurrent("lvds") << "A";
  LOG(DEBUG) << "\tBus power  : " << this->getPower("lvds") << "W";

  LOG(DEBUG) << " V_CIRCUIT";
  this->setVoltage(
    "v_circuit", _config.Get("v_circuit", H2M_VCIRCUIT), _config.Get("v_circuit_current", H2M_VCIRCUIT_CURRENT));
  this->switchOn("v_circuit");
  LOG(DEBUG) << "\tBus voltage: " << this->getVoltage("v_circuit") << "V";
  LOG(DEBUG) << "\tBus current: " << this->getCurrent("v_circuit") << "A";
  LOG(DEBUG) << "\tBus power  : " << this->getPower("v_circuit") << "W";

  LOG(DEBUG) << " V_IO";
  this->setVoltage("v_io", _config.Get("v_io", H2M_VIO), _config.Get("v_io_current", H2M_VIO_CURRENT));
  this->switchOn("v_io");
  LOG(DEBUG) << "\tBus voltage: " << this->getVoltage("v_io") << "V";
  LOG(DEBUG) << "\tBus current: " << this->getCurrent("v_io") << "A";
  LOG(DEBUG) << "\tBus power  : " << this->getPower("v_io") << "W";

  LOG(DEBUG) << " V_SUB"; // first power on V_sub, and then V_Pwell
  double vsub = 0;
  this->setVoltage("v_sub", vsub);
  this->switchOn("v_sub");
  while(vsub > _config.Get("v_sub", H2M_VSUB)) {
    vsub -= 0.1;
    LOG(INFO) << "V_SUB: " << vsub;
    this->setVoltage("v_sub", vsub / (-3.0));
    mDelay(100);
  }

  LOG(DEBUG) << " V_PWELL";
  double vpwell = 0;
  this->setVoltage("v_pwell", vpwell);
  this->switchOn("v_pwell");
  while(vpwell > _config.Get("v_pwell", H2M_VPWELL)) {
    vpwell -= 0.1;
    LOG(INFO) << "V_PWELL: " << vpwell;
    this->setVoltage("v_pwell", vpwell / (-3.0));
    mDelay(100);
  }

  setOutputCMOSLevel(_config.Get("lvds", H2M_LVDS));

  // enable LVDS buffers
  this->setMemory("buffer_LVDS1_pwr", 1);
  this->setMemory("buffer_LVDS2_pwr", 1);
}

void H2MDevice::powerDown() {

  // disable LVDS buffers
  this->setMemory("buffer_LVDS1_pwr", 0);
  this->setMemory("buffer_LVDS2_pwr", 0);

  LOG(DEBUG) << "Power off VDDD";
  this->switchOff("vddd");

  LOG(DEBUG) << "Power off VDDA";
  this->switchOff("vdda");

  LOG(DEBUG) << "Power off LVDS";
  this->switchOff("lvds");

  LOG(DEBUG) << "Power off V_CIRCUIT";
  this->switchOff("v_circuit");

  LOG(DEBUG) << "Power off V_IO";
  this->switchOff("v_io");

  LOG(DEBUG) << "Power off V_PWELL"; // to be tested in the lab!
  double vpwell = _config.Get("v_pwell", H2M_VPWELL);
  while(vpwell < 0) {
    LOG(INFO) << "V_PWELL: " << vpwell;
    this->setVoltage("v_pwell", vpwell / (-2.0));
    mDelay(100);
    vpwell += 0.1;
  }
  this->switchOff("v_pwell");

  LOG(DEBUG) << "Power off V_SUB"; // to be tested in the lab!
  double vsub = _config.Get("v_sub", H2M_VSUB);
  while(vsub < 0) {
    LOG(INFO) << "V_SUB: " << vsub;
    this->setVoltage("v_sub", vsub / (-2.0));
    mDelay(100);
    vsub += 0.1;
  }
  this->switchOff("v_sub");
}

void H2MDevice::powerStatusLog() {
  LOG(INFO) << "Power status:";

  LOG(INFO) << "VDDA:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("vdda") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("vdda") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("vdda") << "W";

  LOG(INFO) << "VDDD:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("vddd") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("vddd") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("vddd") << "W";

  LOG(INFO) << "LVDS:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("lvds") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("lvds") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("lvds") << "W";

  LOG(INFO) << "V_CIRCUIT:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("v_circuit") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("v_circuit") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("v_circuit") << "W";

  LOG(INFO) << "V_IO:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("v_io") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("v_io") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("v_io") << "W";
}

void H2MDevice::logStatusFE() {

  setRegister("analog_out_ctrl", 1);
  double value = getADC("read_analog_out");
  LOG(INFO) << "dac_ibias: " << value << " V";

  setRegister("analog_out_ctrl", 2);
  value = getADC("read_analog_out");
  LOG(INFO) << "dac_itrim: " << value << " V";

  setRegister("analog_out_ctrl", 3);
  value = getADC("read_analog_out");
  LOG(INFO) << "dac_ikrum: " << value << " V";

  setRegister("analog_out_ctrl", 4);
  value = getADC("read_analog_out");
  LOG(INFO) << "dac_vthr: " << value << " V";

  setRegister("analog_out_ctrl", 5);
  value = getADC("read_analog_out");
  LOG(INFO) << "dac_vref: " << value << " V";

  setRegister("analog_out_ctrl", 6);
  value = getADC("read_analog_out");
  LOG(INFO) << "dac_vtpulse: " << value << " V";

  return;
}

void H2MDevice::configureClock(bool internal) {

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
  LOG(INFO) << "Configured the clock source " << (internal ? "INTERNAL" : "EXTERNAL");
}

void H2MDevice::disableClock() {
  _hal->disableSI5345();
}

void H2MDevice::daqStart() {

  // Enable FIFO readout mode:
  setMemory("sc_ctrl_mode", 1);

  // Configure endless pattern generator
  setMemory("pg_runs", 0);

  // Reset all timers and the T0 flag:
  setMemory("timer_start", 1);
  setMemory("timer_start", 0);

  LOG(DEBUG) << "Starting data acquisition, pattern generator in infinite loop";
  // Start pattern generator
  setMemory("pg_start", 1);
}

void H2MDevice::daqStop() {
  LOG(DEBUG) << "Stopping data acquisition, pattern generator stopped.";
  setMemory("pg_stop", 1);

  // Disable FIFO readout:
  setMemory("sc_ctrl_mode", 0);
}

void H2MDevice::printFrames(unsigned int N, std::string filename) {
  std::string frameFilename;
  if(!filename.empty()) {
    frameFilename = filename;
  } else {
    frameFilename = _config.Get("frame_filename", h2m_frameFilename);
  }
  LOG(INFO) << "Printing " << N << " frames to " << frameFilename;

  // run
  for(unsigned int i = 0; i < N; i++) {
    auto frame = getData();
    frame_decoder_.printFrame(frame, frameFilename.c_str(), true);
  }
}

void H2MDevice::occupancy_initialize() {
  matrixOccupancy_.clear();
  matrixOccupancy_.resize(H2M_NCOL, std::vector<uint16_t>(H2M_NROW, 0));
}

void H2MDevice::occupancy_add(const pearydata& frame) {

  int scan_n = _config.Get("scan_n", h2m_scan_n);

  for(const auto& pixel_ptr : frame) {
    auto pixel = dynamic_cast<caribou::h2m_pixel_readout*>(pixel_ptr.second.get());

    if(!pixel->GetData()) {
      continue;
    }

    // different cases:
    // count when we record several frames per print
    // for single frames we can pass TOA or TOT
    int occupancy = 0;
    if(scan_n == 1) {
      occupancy = static_cast<int>(pixel->GetData());
    } else {
      occupancy = pixel->GetMode() == ACQ_MODE_CNT ? static_cast<int>(pixel->GetData()) : 1;
    }

    try {
      // I hate this but we need to avoid compiler warnings for adding uint16_t's
      int current = matrixOccupancy_.at(pixel_ptr.first.first).at(pixel_ptr.first.second);
      matrixOccupancy_.at(pixel_ptr.first.first).at(pixel_ptr.first.second) = static_cast<uint16_t>(occupancy + current);
    } catch(const std::out_of_range& e) {
      LOG(ERROR) << "Range error in occupancy_add.";
    }
  }
}

void H2MDevice::occupancy_print(
  std::string filename, std::string scan_dac, int scan_start, int scan_end, int scan_step, unsigned int scan_n) {

  // output file
  std::ofstream binary_file;

  int scan_dac_i = h2m_dac_index.find(scan_dac) != h2m_dac_index.end() ? (*h2m_dac_index.find(scan_dac)).second : 0;

  // use binary for further development
  binary_file.open(filename.append(".bin"), std::ios_base::app | std::ofstream::binary);
  if(binary_file.is_open()) {

    uint16_t write_map[H2M_NCOL * H2M_NROW + 6];
    // ToDo: make this better
    for(size_t col = 0; col < H2M_NCOL; col++) {
      for(size_t row = 0; row < H2M_NROW; row++) {
        write_map[col + row * H2M_NCOL] = matrixOccupancy_[col][row];
      }
    }
    // std::vector is not stored in a consecutive memory block
    // memcpy(write_map, &matrixOccupancy_[0][0], H2M_NCOL * H2M_NROW);

    write_map[H2M_NCOL * H2M_NROW] = static_cast<uint16_t>(getRegister(scan_dac));
    write_map[H2M_NCOL * H2M_NROW + 1] = static_cast<uint16_t>(scan_start);
    write_map[H2M_NCOL * H2M_NROW + 2] = static_cast<uint16_t>(scan_step);
    write_map[H2M_NCOL * H2M_NROW + 3] = static_cast<uint16_t>(scan_end);
    write_map[H2M_NCOL * H2M_NROW + 4] = static_cast<uint16_t>(scan_n);
    write_map[H2M_NCOL * H2M_NROW + 5] = static_cast<uint16_t>(scan_dac_i);

    binary_file.write(reinterpret_cast<const char*>(write_map),
                      static_cast<std::streamsize>((H2M_NCOL * H2M_NROW + 6) * sizeof(uint16_t)));
  } else {
    throw DeviceException("Can't open output file.\n");
  }
  binary_file.close();
}

void H2MDevice::testpulse(unsigned int N, bool write) {

  // get outfile name
  std::string testpulseFilename = _config.Get("testpulse_filename", h2m_testpulseFilename);
  if(write) {
    LOG(INFO) << "Printing " << N << " frames to " << testpulseFilename;
  }

  // prepare data structure for occupancy map
  occupancy_initialize();

  // run loop
  for(unsigned int i = 0; i < N; i++) {

    // prepare for acquisition
    setRegister("acq_start", 1);
    setRegister("matrix_ctrl", 1);
    setMemory("shutter", _config.Get("shutter", _memory.get("shutter").second.value()));

    // wait until the shutter closes before we read data
    LOG(DEBUG) << "Waiting for shutter to close...";
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    while(getMemory("shutter")) {
      auto dur = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start);
      if(dur.count() > 42) // You can go up to ~40 seconds, maximum value is 0xFFFFFFFF (4294967295)
        throw DataException("Reached shutter timeout. Something went wrong!");
    }
    setRegister("acq_start", 0);

    // TODO: check if the test-pulse sequence really finished (getMemory("sc_status");

    // add data to occupancy map and write to file if requested
    auto frame = getData();

    occupancy_add(frame);

    if(write) {
      frame_decoder_.printFrame(frame, testpulseFilename.c_str(), true);
    }
  }
}

void H2MDevice::parameterScan(
  std::string scan_dac, int scan_start, int scan_end, int scan_step, unsigned int scan_n, std::string occupancyFilename) {

  int scan = scan_start;

  while(scan <= scan_end) {
    setRegister(scan_dac, static_cast<uintptr_t>(scan));

    LOG(STATUS) << "Scanning " << scan_dac << " at " << scan;
    testpulse(scan_n, 0);
    occupancy_print(occupancyFilename, scan_dac, scan_start, scan_end, scan_step, scan_n);

    scan += scan_step;
  }
}

void H2MDevice::takeFrames(unsigned int N) {

  std::string occupancyFilename = _config.Get("occupancy_filename", h2m_occupancyFilename);
  std::string scan_dac = _config.Get("scan_dac", h2m_scan_dac);
  int scan_start = _config.Get("scan_start", h2m_scan_start);
  int scan_step = _config.Get("scan_step", h2m_scan_step);
  int scan_end = _config.Get("scan_end", h2m_scan_end);
  auto scan_n = static_cast<uint16_t>(_config.Get("scan_n", h2m_scan_n));

  for(unsigned int i = 0; i < N; i++) {
    if(i % 1000 == 0) {
      LOG(STATUS) << "Taking frame " << i;
    }
    // ToDo: scan_n needs to be 1 for this to work. Fix...
    testpulse(scan_n, 0);
    occupancy_print(occupancyFilename, scan_dac, scan_start, scan_end, scan_step, scan_n);
  }
}

pearydata H2MDevice::getData() {

  uint8_t mode;
  if(getMemory("sc_ctrl_mode") == 0x1) {
    this->setMemory("sc_ctrl_mode", 0);
    mode = static_cast<uint8_t>(getRegister("acq_mode"));
    this->setMemory("sc_ctrl_mode", 1);
  } else {
    mode = static_cast<uint8_t>(getRegister("acq_mode"));
  }
  LOG(DEBUG) << "H2MDevice::getData(): Acquisition mode: " << mode;

  LOG(DEBUG) << "H2MDevice::getData(): Starting to decode frame.";

  pearyRawData rawdata;
  if(force_read_from_fifo_ || getMemory("sc_ctrl_mode") == 0x1) {
    rawdata = getRawData();
  } else {
    rawdata = getRawDataSC();
  }

  // Print the header:
  if(rawdata.size() == H2M_FW_EVENTSIZE) {
    LOG(DEBUG) << "H2M FW Header:";
    for(size_t i = 0; i < 6; i++) {
      LOG(DEBUG) << "0x" << std::hex << rawdata.at(i) << std::dec;
    }

    auto [ts_trig, ts_sh_open, ts_sh_close, frame, length, t0] = frame_decoder_.decodeHeader(rawdata);
    LOG(DEBUG) << "Frame #" << frame << std::endl
               << "Data length: " << length << std::endl
               << "TS Trigger:  " << ts_trig << std::endl
               << "TS Sh Open:  " << ts_sh_open << std::endl
               << "TS Sh Close: " << ts_sh_close << std::endl
               << "T0:          " << t0;

    // If we want to decode here directly, we need to drop the header.
    rawdata.erase(rawdata.begin(), rawdata.begin() + 6);

    LOG(DEBUG) << "H2M Frame:";
  }

  return frame_decoder_.decodeFrame(rawdata, mode);
}

pearyRawData H2MDevice::getRawDataSC() {
  // Fetching the data via direct register access
  // We have 64 columns and need to read 4 times to get the 128 bit per column.
  pearyRawData rawdata;
  uintptr_t data_group;
  for(unsigned int col = 0; col < H2M_NCOL; col++) {
    for(unsigned int i_in_group = 0; i_in_group < H2M_NPXGROUP; i_in_group++) {

      data_group = getRegister("px_cnt_r_" + to_string(col)); // this will retun 32 bits
      LOG(DEBUG) << "Data for column " << col << " reading " << i_in_group << ": " << std::hex << data_group << std::dec;
      rawdata.push_back(data_group);
    }
  }
  return rawdata;
}

pearyRawData H2MDevice::getRawData() {

  // Check buffer status
  if(getMemory("fifoStatus") >= H2M_FW_EVENTSIZE) {
    return read_fifo_data(H2M_FW_EVENTSIZE);
  }

  throw NoDataAvailable();
}

H2MDevice::~H2MDevice() {
  LOG(INFO) << "Shutdown, delete device.";
  powerOff();
}
