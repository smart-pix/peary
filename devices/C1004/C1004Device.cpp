/**
 * Caribou Device implementation for C1004
 */

#include "C1004Device.hpp"
#include <cmath>
#include "framedecoder/C1004_frameDecoder.hpp"
#include "utils/log.hpp"

using namespace caribou;

C1004Device::C1004Device(const caribou::Configuration config)
    : CaribouDevice(config,
                    iface_spi_bus::configuration_type(config.Get("devpath", std::string(DEFAULT_SCIPATH)), 8, 8, true, true),
                    iface_media::configuration_type(config.Get("devdripath", std::string(DEFAULT_DRIPATH)),
                                                    config.Get("devdrimedia_outputentity", DEFAULT_DRI_MEDIA_OUTPUT_ENTITY),
                                                    config.Get("drilinks", MEDIA_BALANCE_LINKS),
                                                    config.Get("driroutes", MEDIA_BALANCE_ROUTES),
                                                    config.Get("driformats", MEDIA_FORMATS))),
      _videoStreamConsumer(true), _dataProcessor(distance),
      _activeDriConfig(_driConfig), _dataProcessorRegisters{
                                      {distance, dataProcessorRegister_t("distanceProcessorRegisters")},
                                      {user, dataProcessorRegister_t("userProcessorRegisters")},
                                      {intensity, dataProcessorRegister_t("intensityProcessorRegisters")},
                                      {rawTimestamps, dataProcessorRegister_t("rawTimestampsProcessorRegisters")}} {
  // Set up periphery
  _periphery.add("vbpext_tdc", falconboard::BIAS_1);
  _periphery.add("vbpext_vco", falconboard::BIAS_2);
  _periphery.add("tdc_ref", falconboard::BIAS_3);
  _periphery.add("veb", falconboard::BIAS_4);
  _periphery.add("vq", falconboard::BIAS_5);
  _periphery.add("vpu", falconboard::BIAS_6);
  _periphery.add("vwin", falconboard::BIAS_7);
  _periphery.add("vop", falconboard::BIAS_8);
  _periphery.add("vco_bias", falconboard::BIAS_9);
  _periphery.add("vcsel", falconboard::DCDC_VCSEL);
  _periphery.add("vin_vcsel", falconboard::ADC_VIN_VCSEL);

  // Add the register definitions to the dictionary for convenient lookup of names:
  _registers.add(C1004_REGISTERS);

  // Add memory pages to the dictionary:
  _memory.add(C1004_MEMORY);

  // Add processor registers
  _dataProcessorRegisters.at(distance).add(C1004_SINGLEBALANCE_REGISTERS);
  _dataProcessorRegisters.at(intensity).add(C1004_INTENSITY_REGISTERS);
  _dataProcessorRegisters.at(rawTimestamps).add(C1004_RAWTIMESTAMPS_REGISTERS);

  // Add user processor registers
  iface_media& dri = InterfaceManager::getInterface<iface_media>(getDriConfiguration(user));
  auto subDevice = dataProcessorSubDeviceNaming[user];
  // Populate all registers which can be accessed
  try {
    for(size_t reg = 0;; reg++) {
      dri.getSubDevice(subDevice, reg * 8);
      _dataProcessorRegisters.at(user).add(std::string("user_") + std::to_string(reg),
                                           register_t<std::uintptr_t, std::uintptr_t>(reg * 8, 0xFFFFFFFF));
    }
  } catch(CommunicationError&) {
    LOG(DEBUG) << "Found " << _dataProcessorRegisters.at(user).size() << " user processor registers";
  }

  // Register custom commands
  _dispatcher.add("configureSequencer", &C1004Device::configureSequencer, this);
  _dispatcher.add("getSequencerStatus", &C1004Device::getSequencerStatus, this);
  _dispatcher.add("setLaserHV", &C1004Device::setLaserHV, this);
  _dispatcher.add("getLaserHVStatus", &C1004Device::getLaserHVStatus, this);
  _dispatcher.add("setVideoStreamConsumer", &C1004Device::setVideoStreamConsumer, this);
  _dispatcher.add("getVideoStreamConsumer", &C1004Device::getVideoStreamConsumer, this);
  _dispatcher.add("setDataProcessor", &C1004Device::setDataProcessor, this);
  _dispatcher.add("getDataProcessor", &C1004Device::getDataProcessor, this);
  _dispatcher.add("setDataProcessorRegister", &C1004Device::setDataProcessorRegister, this);
  _dispatcher.add("getDataProcessorRegister", &C1004Device::getDataProcessorRegister, this);
  _dispatcher.add("setClockSource", &C1004Device::setClockSource, this);
  _dispatcher.add("getClockSource", &C1004Device::getClockSource, this);

  // keep C1004 in reset, bypass disabled
  setMemory("reset", 1);

  // Select the internal clock source
  setMemory("clock_source", internal);
}

void C1004Device::configure() {
  LOG(INFO) << "Configuring";
  powerOn();
  reset();

  // set bias voltages
  LOG(INFO) << "Setting bias voltages from configuration:";
  for(const auto& biasVoltage : _periphery.getNames<caribou::BIAS_REGULATOR_T>()) {
    try {
      auto value = _config.Get<double>(biasVoltage);
      this->setVoltage(biasVoltage, value);
      LOG(INFO) << "Set bias voltage \"" << biasVoltage << "\" to " << value << " V";
    } catch(ConfigMissingKey& e) {
      continue;
    }
  }

  // configure clock source fist
  // as it may involve reset
  if(_config.Has("clock_source_name")) {
    LOG(INFO) << "Setting clock source from configuration:";
    std::istringstream clockSourceString(_config.Get<std::string>("clock_source_name"));
    clockSource_t clockSource;
    clockSourceString >> clockSource;
    setClockSource(clockSource);
  }

  // configure registers
  CaribouDevice<falconboard::Falconboard, iface_spi_bus, iface_media>::configure();

  LOG(INFO) << "Setting data processor registers from configuration:";
  for(const auto& dataProcessorRegisters : _dataProcessorRegisters) {
    auto dataProcessorRegs = dataProcessorRegisters.second.getNames();
    setDataProcessor(dataProcessorRegisters.first);
    for(const auto& i : dataProcessorRegs) {
      try {
        auto value = _config.Get<uintptr_t>(i);
        this->setDataProcessorRegister(i, value);
        LOG(INFO) << "Set " << dataProcessorRegisters.first << " processor register \"" << i
                  << "\" = " << static_cast<int>(value) << " (" << to_hex_string(value) << ")";
      } catch(ConfigMissingKey& e) {
        LOG(DEBUG) << "Could not find key \"" << i << "\" in the configuration, skipping.";
      }
    }
  }

  // select data processor
  if(!_config.Has("data_processor")) {
    LOG(WARNING) << "No 'data_processor' selected in the configuration file. Selecting " << _dataProcessor;
  } else {
    std::istringstream dataProcessorString(_config.Get<std::string>("data_processor"));
    dataProcessor_t dataProcessor;
    dataProcessorString >> dataProcessor;
    setDataProcessor(dataProcessor);
  }

  // configure sequencer
  bool direct_control = true;
  uint32_t duration = 20;
  uint32_t repetition_rate = 0;
  uint32_t latch_duration = 2;

  if(!_config.Has("falcon_sequencer_direct_control")) {
    LOG(WARNING) << "No 'falcon_sequencer_direct_control' found in the configuration file. Setting to " << direct_control;
  } else
    direct_control = _config.Get<bool>("falcon_sequencer_direct_control");

  if(!_config.Has("falcon_sequencer_duration")) {
    LOG(WARNING) << "No 'falcon_sequencer_duration' found in the configuration file. Setting to " << duration;
  } else
    duration = _config.Get<uint32_t>("falcon_sequencer_duration");

  if(!_config.Has("falcon_sequencer_repetition_rate")) {
    LOG(WARNING) << "No 'falcon_sequencer_repetition_rate' found in the configuration file. Setting to " << repetition_rate;
  } else
    repetition_rate = _config.Get<uint32_t>("falcon_sequencer_repetition_rate");

  if(!_config.Has("falcon_sequencer_latch_duration")) {
    LOG(WARNING) << "No 'falcon_sequencer_latch_duration' found in the configuration file. Setting to " << latch_duration;
  } else
    latch_duration = _config.Get<uint32_t>("falcon_sequencer_latch_duration");

  LOG(INFO) << "Configuring Falcon sequencer";
  configureSequencer(direct_control, duration, repetition_rate, latch_duration);

  if(_config.Has("laser_hv")) {
    LOG(INFO) << "Configuring laser HV";
    setLaserHV(_config.Get<double>("laser_hv"));
  }
}

void C1004Device::reset() {
  LOG(INFO) << "Resetting";

  // assert reset:
  setMemory("reset", 1);
  usleep(1);
  // deny reset:
  setMemory("reset", 0);
  mDelay(25); // supverisory device on PCB
}

C1004Device::~C1004Device() {
  LOG(INFO) << "Shutdown, delete device.";

  daqStop();
  powerOff();
}

void C1004Device::powerUp() {
  LOG(INFO) << "Powering up";

  LOG(DEBUG) << "VBPEXT_TDC";
  this->setVoltage("vbpext_tdc", _config.Get("vbpext_tdc", C1004_VBPEXT_TDC));
  this->switchOn("vbpext_tdc");

  LOG(DEBUG) << "VBPEXT_VCO";
  this->setVoltage("vbpext_vco", _config.Get("vbpext_vco", C1004_VBPEXT_VCO));
  this->switchOn("vbpext_vco");

  LOG(DEBUG) << "TDC_REF";
  this->setVoltage("tdc_ref", _config.Get("tdc_ref", C1004_TDC_REF));
  this->switchOn("tdc_ref");

  LOG(DEBUG) << "VEB";
  this->setVoltage("veb", _config.Get("veb", C1004_VEB));
  this->switchOn("veb");

  LOG(DEBUG) << "VQ";
  this->setVoltage("vq", _config.Get("vq", C1004_VQ));
  this->switchOn("vq");

  LOG(DEBUG) << "VPU";
  this->setVoltage("vpu", _config.Get("vpu", C1004_VPU));
  this->switchOn("vpu");

  LOG(DEBUG) << "VWIN";
  this->setVoltage("vwin", _config.Get("vwin", C1004_VWIN));
  this->switchOn("vwin");

  LOG(DEBUG) << "VOP";
  this->setVoltage("vop", _config.Get("vop", C1004_VOP));
  this->switchOn("vop");

  LOG(DEBUG) << "VCO_BIAS";
  this->setVoltage("vco_bias", _config.Get("vco_bias", C1004_VCO_BIAS));
  this->switchOn("vco_bias");

  LOG(DEBUG) << "Setting laser HV to 12 V";
  this->switchOff("vcsel");
  this->setVoltage("vcsel", 13.5); // set minimum voltage

  mDelay(25); // supverisory device on PCB
}

void C1004Device::powerDown() {
  LOG(INFO) << "Power off";

  LOG(DEBUG) << "Setting laser HV to 12 V";
  this->switchOff("vcsel");
  this->setVoltage("vcsel", 13.5); // set minimum voltage

  LOG(DEBUG) << "Power off VBPEXT_TDC";
  this->switchOff("vbpext_tdc");

  LOG(DEBUG) << "Power off VBPEXT_VCO";
  this->switchOff("vbpext_vco");

  LOG(DEBUG) << "Power off TDC_REF";
  this->switchOff("tdc_ref");

  LOG(DEBUG) << "Power off VEB";
  this->switchOff("veb");

  LOG(DEBUG) << "Power off VQ";
  this->switchOff("vq");

  LOG(DEBUG) << "Power off VPU";
  this->switchOff("vpu");

  LOG(DEBUG) << "Power off VWIN";
  this->switchOff("vwin");

  LOG(DEBUG) << "Power off VOP";
  this->switchOff("vop");

  LOG(DEBUG) << "Power off VCO_BIAS";
  this->switchOff("vco_bias");
}

void C1004Device::daqStart() {
  iface_media& dri = InterfaceManager::getInterface<iface_media>(_activeDriConfig);
  if(dri.isStreaming()) {
    LOG(WARNING) << "The media interface is streamiming already";
  } else {
    if(_videoStreamConsumer) {
      dri.readStart();
    } else {
      dri.configureMediaPipeline();
    }
  }

  // check reference trigger conditions
  if(getMemory("ref_tdc_trig_en") && !getMemory("sequencer_direct_control"))
    throw ConfigInvalid("Reference TDC trigger requires sequencer to run in the direct control mode");

  // bypass C1004's sequencer
  this->setRegister("seq_seq_en", !getMemory("sequencer_direct_control"));
  try {
    enableSequencer(true);
  } catch(caribouException const& e) {
    // Problem with enabling the sequencer
    if(dri.isStreaming())
      dri.readStop();
    throw;
  }

  // Enable pixel array
  setMemory("pixel_array_en", 1);
}

void C1004Device::daqStop() {

  // Disable pixel array
  setMemory("pixel_array_en", 0);

  enableSequencer(false);

  iface_media& dri = InterfaceManager::getInterface<iface_media>(_activeDriConfig);
  if(dri.isStreaming())
    dri.readStop();
}

pearydataVector C1004Device::decodeFrame(const typename iface_media::dataVector_type& frames) {
  C1004_frameDecoder decoder;
  pearydataVector decodedData;
  for(const auto& rawData : frames) {
    decoder.decode(rawData);
    LOG(DEBUG) << "Decoded frame [row][column]:\n" << decoder;
    decodedData.push_back(decoder.getFrame());
  }

  return decodedData;
}

pearydata C1004Device::getData() {
  return std::move(decodeFrame(iface_media::dataVector_type{getRawData()}).front());
}

pearydataVector C1004Device::getData(const unsigned int noFrames) {
  return decodeFrame(getRawData(noFrames));
}

pearyRawData C1004Device::getRawData() {
  return InterfaceManager::getInterface<iface_media>(_activeDriConfig).read();
}

pearyRawDataVector C1004Device::getRawData(const unsigned int noFrames) {
  return InterfaceManager::getInterface<iface_media>(_activeDriConfig).read(noFrames);
}

void C1004Device::setSpecialRegister(const std::string& name, uintptr_t value) {
  if(name == "ren") {
    uint8_t msb;
    uint8_t lsb;

    if(value > static_cast<uintptr_t>(pow(2, 10) - 1)) {
      LOG(WARNING) << "ren is limited to " << pow(2, 10) - 1 << " value, skipping";
      return;
    }
    msb = static_cast<uint8_t>(value >> 8);
    lsb = value & 0xFF;

    LOG(DEBUG) << "ren lookup: " << value << " = " << static_cast<int>(msb) << "-" << static_cast<int>(lsb);
    // Set the values
    this->setRegister("ren_lsb", msb);
    this->setRegister("ren_msb", lsb);
  }

  else if(name == "readout_select_column") {
    if(value > static_cast<uintptr_t>(pow(2, 32) - 1)) {
      LOG(WARNING) << "readout_select_column is limited to " << pow(2, 32) - 1 << " value, skipping";
      return;
    }

    LOG(DEBUG) << "readout_select_column: " << value;

    // Set the values
    this->setRegister("readout_select_column_0", value & 0xFF);
    this->setRegister("readout_select_column_1", static_cast<uint8_t>((value >> 8) & 0xFF));
    this->setRegister("readout_select_column_2", static_cast<uint8_t>((value >> 16) & 0xFF));
    this->setRegister("readout_select_column_3", static_cast<uint8_t>((value >> 24) & 0xFF));
  } else {
    throw RegisterInvalid("Unknown register with \"special\" flag: " + name);
  }
}

uintptr_t C1004Device::getSpecialRegister(const std::string& name) {
  if(name == "ren") {
    uint8_t msb = static_cast<uint8_t>(this->getRegister("ren_msb"));
    uint8_t lsb = static_cast<uint8_t>(this->getRegister("ren_lsb"));
    return static_cast<uintptr_t>((msb << 8) | lsb);
  } else if(name == "readout_select_column") {
    return static_cast<uintptr_t>(static_cast<uint8_t>(this->getRegister("readout_select_column_3")) << 24 |
                                  static_cast<uint8_t>(this->getRegister("readout_select_column_2")) << 16 |
                                  static_cast<uint8_t>(this->getRegister("readout_select_column_1")) << 8 |
                                  static_cast<uint8_t>(this->getRegister("readout_select_column_0")));
  } else {
    throw RegisterInvalid("Unknown register with \"special\" flag: " + name);
  }
}

void C1004Device::configureSequencer(const bool directControl,
                                     const uint32_t duration,
                                     const uint32_t repetition_rate,
                                     const uint32_t latch_duration) {
  LOG(DEBUG) << "Setting sequencer " << (directControl ? "in direct control mode" : "") << " to "
             << duration * C1004_SEQUENCER_CLOCK_PERIOD_NS << " ns duration (raw: " << duration << "), repetition rate of "
             << 1000 / (repetition_rate * C1004_SEQUENCER_CLOCK_PERIOD_NS) << " MHz (raw: " << repetition_rate << ")"
             << (directControl
                   ? " and " + std::to_string(latch_duration * C1004_SEQUENCER_CLOCK_PERIOD_NS) + " ns of latch duration"
                   : "");

  if(getMemory("sequencer_en"))
    throw ConfigInvalid("The sequener is enabled - can not apply the new configuration");

  if((repetition_rate != 0) && (duration > repetition_rate))
    throw ConfigInvalid("Repetition rate (unles set to 0) can't be lower than duration.");

  if(directControl && (latch_duration < 1 || latch_duration > 8))
    throw ConfigInvalid("Wrong latch duration. Valid range is 1-8.");

  auto reg = getMemory("sequencer_direct_control") & ~C1004_SEQUENCER_DIRECT_CONTROL_MASK;
  if(directControl)
    reg |= C1004_SEQUENCER_DIRECT_CONTROL_MASK;

  _hal->writeMemory(reg_sequencer, C1004_SEQUENCER_DIRECT_CONTROL_OFFSET, reg);
  _hal->writeMemory(reg_sequencer, C1004_SEQUENCER_DURATION_OFFSET, duration & C1004_SEQUENCER_DURATION_MASK);
  _hal->writeMemory(
    reg_sequencer, C1004_SEQUENCER_REPETITION_RATE_OFFSET, repetition_rate & C1004_SEQUENCER_REPETITION_RATE_MASK);
  _hal->writeMemory(
    reg_sequencer, C1004_SEQUENCER_LATCH_DURATION_OFFSET, latch_duration & C1004_SEQUENCER_LATCH_DURATION_MASK);
}

// This method enables/disables the sequecner
void C1004Device::enableSequencer(const bool enable) {
  LOG(DEBUG) << (enable ? "Enabling " : "Disabling ") << " sequencer.";

  auto reg = getMemory("sequencer_en") & ~C1004_SEQUENCER_EN_MASK;
  if(enable) {
    if(getMemory("laser_status"))
      throw DeviceException("Aborting sequencer enable: laser errors!");

    checkLaserHV();
    reg = reg | C1004_SEQUENCER_EN_MASK;
  }
  _hal->writeMemory(reg_sequencer, C1004_SEQUENCER_EN_OFFSET, reg);

  LOG(DEBUG) << "Sequencer " << (enable ? "enabled." : "disabled.");
}

void C1004Device::getSequencerStatus() {
  const auto en = getMemory("sequencer_en");
  const auto directControl = getMemory("sequencer_direct_control");
  const auto duration = getMemory("sequencer_duration");
  const auto repetition_rate = getMemory("sequencer_repetition_rate");
  const auto latch_duration = getMemory("sequencer_latch_duration");

  LOG(INFO) << "Sequencer is " << ((en) ? "enabled" : "disabled") << " " << (directControl ? "in direct control mode" : "")
            << " and set to " << static_cast<double>(duration) * C1004_SEQUENCER_CLOCK_PERIOD_NS
            << " ns duration (raw: " << duration << "), repetition rate of "
            << 1000 / (static_cast<double>(repetition_rate) * C1004_SEQUENCER_CLOCK_PERIOD_NS)
            << " MHz (raw: " << repetition_rate << ")"
            << (directControl
                  ? " and " + std::to_string(static_cast<double>(latch_duration) * C1004_SEQUENCER_CLOCK_PERIOD_NS) +
                      " ns of latch duration"
                  : "");
}

void C1004Device::getLaserHVStatus() {
  LOG(INFO) << "The laser HV DC/DC converter module is " << (getMemory("laser_en") ? "enabled" : "disabled")
            << (getMemory("laser_status") ? " (there are HV ERRORS!)" : " (no errors)");
}

void C1004Device::checkLaserHV() {
  const auto voltage = this->getADC("vin_vcsel");
  const auto repetitionRate = getMemory("sequencer_repetition_rate");
  const auto duration = getMemory("sequencer_duration");
  const double maxFreq = (repetitionRate != 0)
                           ? (1000 / (static_cast<double>(repetitionRate) * C1004_SEQUENCER_CLOCK_PERIOD_NS))
                           : (1000 / (static_cast<double>(duration) * C1004_SEQUENCER_CLOCK_PERIOD_NS));
  try {
    const auto safeFreq = SAFE_REPETITION_RATE.at(static_cast<unsigned int>(::ceill(voltage)));

    if(safeFreq < maxFreq)
      throw ConfigInvalid("Laser can not be opperated with the current settings.\n"
                          "Sequencer is configured to pulse the laser with the maximum frequency of " +
                          std::to_string(maxFreq) + " MHz,\n" + "while for the current VIN_VCSEL laser voltage (" +
                          std::to_string(voltage) + "V) allows the maximum safe frequency of " + std::to_string(safeFreq) +
                          " MHz.");

  } catch(const std::out_of_range&) {
    throw ConfigInvalid("The current VIN_VCSEL laser voltage (" + std::to_string(voltage) + "V) is not supported.");
  }
}

void C1004Device::setLaserHV(const double voltage) {

  const double DOUBLE_PRECISION_EPSILON = 0.001;

  LOG(DEBUG) << "Setting " << voltage << "V "
             << "on laser high voltage (HV) module";

  if((voltage > 80.0 || voltage < 13.5) && fabs(voltage - 12) > DOUBLE_PRECISION_EPSILON)
    throw ConfigInvalid("Trying to set laser high voltage (HV) module to " + std::to_string(voltage) +
                        " V (valid settings are 12 V and 13.5-80 V range)");

  if(getMemory("sequencer_en"))
    throw ConfigInvalid("Can't set the laser high voltage (HV), when the sequencer is enabled");

  ///////////////////////////
  // Set the DC/DC converter
  ///////////////////////////
  this->switchOff("vcsel");

  if(voltage > 13.5 - DOUBLE_PRECISION_EPSILON) {
    this->setVoltage("vcsel", voltage);
    this->switchOn("vcsel");
  } else
    this->setVoltage("vcsel", 13.5); // set minimum voltage

  ////////////////////////////////////////////////
  // Confirm the DC/DC converter was set properly
  ////////////////////////////////////////////////
  unsigned short int measurementTriesCounter = 1;
  const unsigned short int measurementTriesLimit = 5;
  do {
    mDelay(2000); // in oder to completely discharge the capacitors bank

    const double measuredVoltage = this->getADC("vin_vcsel");

    LOG(DEBUG) << "The measured voltage on laser high voltage (HV) module is " << measuredVoltage << " V (try "
               << measurementTriesCounter << ")";

    // if measured voltage differs less than 5% exit the measurement tries loop
    if(fabs(voltage - measuredVoltage) < 0.05 * voltage)
      break;

    measurementTriesCounter++;

  } while(measurementTriesCounter <= measurementTriesLimit);

  // All measurement tries have been used, raise the error
  if(measurementTriesCounter > measurementTriesLimit) {
    this->switchOff("vcsel");
    throw DeviceException("The setup of the laser high voltage (HV) module failed. The DC/DC converter has been disabled.");
  }
}

void C1004Device::setVideoStreamConsumer(bool videoStreamConsumer) {
  _videoStreamConsumer = videoStreamConsumer;
}

bool C1004Device::getVideoStreamConsumer() {
  if(_videoStreamConsumer) {
    LOG(INFO) << "The device is using the video stream";
  } else {
    LOG(INFO) << "The device is not using the video stream";
  }

  return _videoStreamConsumer;
}

iface_media::configuration_type C1004Device::getDriConfiguration(const dataProcessor_t dataProcessor) {
  switch(dataProcessor) {
  case distance:
    return _driConfig;
    break;

  case user:
    return iface_media::configuration_type(_driConfig._devpath,
                                           _driConfig._outputEntity,
                                           MEDIA_USER_LINKS,
                                           MEDIA_USER_ROUTES,
                                           _driConfig._formats,
                                           _driConfig._timeout,
                                           _driConfig._noBuffers);
    break;
  case intensity:
    return iface_media::configuration_type(_driConfig._devpath,
                                           _driConfig._outputEntity,
                                           MEDIA_INTENSITY_LINKS,
                                           MEDIA_INTENSITY_ROUTES,
                                           _driConfig._formats,
                                           _driConfig._timeout,
                                           _driConfig._noBuffers);
    break;
  case rawTimestamps:
    return iface_media::configuration_type(_driConfig._devpath,
                                           _driConfig._outputEntity,
                                           MEDIA_RAWTIMESTAMPS_LINKS,
                                           MEDIA_RAWTIMESTAMPS_ROUTES,
                                           _driConfig._formats,
                                           _driConfig._timeout,
                                           _driConfig._noBuffers);
    break;
  default:
    throw std::invalid_argument(
      "Unknown C1004 readout data procssor. Valid data procssors: distance, user, intensity, rawTimestamps.");
  }
}

void C1004Device::setDataProcessor(const dataProcessor_t dataProcessor) {
  iface_media& dri = InterfaceManager::getInterface<iface_media>(_activeDriConfig);
  if(dri.isStreaming())
    throw DeviceException("Can't change data procssor when device is streaming.");

  _activeDriConfig = getDriConfiguration(dataProcessor);
  _dataProcessor = dataProcessor;
}

C1004Device::dataProcessor_t C1004Device::getDataProcessor() {
  LOG(INFO) << "Active data procssor: " << _dataProcessor;
  return _dataProcessor;
}

namespace caribou {
  std::istream& operator>>(std::istream& is, C1004Device::dataProcessor_t& dataProcessor) {
    std::string arg;
    is >> arg;
    if(!arg.compare("distance"))
      dataProcessor = C1004Device::distance;
    else if(!arg.compare("user"))
      dataProcessor = C1004Device::user;
    else if(!arg.compare("intensity"))
      dataProcessor = C1004Device::intensity;
    else if(!arg.compare("rawTimestamps"))
      dataProcessor = C1004Device::rawTimestamps;
    else
      throw std::invalid_argument(
        "Unknown C1004 data processore. Valid data processor: distance, user, intensity, rawTimestamps.");
    return is;
  }

  std::ostream& operator<<(std::ostream& os, C1004Device::dataProcessor_t const& dataProcessor) {
    switch(dataProcessor) {
    case C1004Device::distance:
      os << "distance";
      break;
    case C1004Device::user:
      os << "user";
      break;
    case C1004Device::intensity:
      os << "intensity";
      break;
    case C1004Device::rawTimestamps:
      os << "rawTimestamps";
      break;
    default:
      throw std::invalid_argument(
        "Unknown C1004 data processor. Valid data processor: distance, user, intensity, rawTimestamps.");
    }
    return os;
  }

  void C1004Device::setDataProcessorRegister(const std::string& name, uintptr_t value) {
    auto reg = _dataProcessorRegisters.at(_dataProcessor).get(name);

    if(!reg.writable()) {
      throw caribou::RegisterTypeMismatch("Trying to write to data processor register with \"nowrite\" flag: " + name);
    }

    iface_media& dri = InterfaceManager::getInterface<iface_media>(_activeDriConfig);

    auto subDevice = dataProcessorSubDeviceNaming[_dataProcessor];
    auto current_value = dri.getSubDevice(subDevice, reg.address());
    value = obey_mask_write<uintptr_t, uintptr_t>(reg, value, current_value);
    LOG(DEBUG) << "Register to be set: " << name << " (" << to_hex_string(reg.address()) << ")";
    dri.setSubDevice(subDevice, std::make_pair(reg.address(), value));
  }

  uintptr_t C1004Device::getDataProcessorRegister(const std::string& name) {
    auto reg = _dataProcessorRegisters.at(_dataProcessor).get(name);

    if(!reg.readable()) {
      // This register cannot be read back from the device:
      throw caribou::RegisterTypeMismatch("Trying to read data processor register with \"noread\" flag: " + name);
    }

    LOG(DEBUG) << "Register to be read: " << name << " (" << to_hex_string(reg.address()) << ")";

    iface_media& dri = InterfaceManager::getInterface<iface_media>(_activeDriConfig);
    auto subDevice = dataProcessorSubDeviceNaming[_dataProcessor];

    auto value = obey_mask_read<uintptr_t, uintptr_t>(reg, dri.getSubDevice(subDevice, reg.address()));

    LOG(INFO) << name << " = " << value;
    return value;
  }

} // namespace caribou

void C1004Device::setClockSource(const clockSource_t source) {
  if((getMemory("clock_source") ? C1004Device::external : C1004Device::internal) == source) {
    LOG(INFO) << "The C1004 clock source is already set to: " << source;
    return;
  }

  auto reset = getMemory("reset");

  if(reset == 0) {
    LOG(INFO) << "Asserting reset";
    setMemory("reset", 1);
  }

  setMemory("clock_source", source);

  if(reset == 0) {
    LOG(INFO) << "Releasing reset";
    setMemory("reset", 0);
    mDelay(25);

    if(!getMemory("clock_locked"))
      LOG(WARNING) << "The PLL can not lock to the configured clock source. Check the clock source.";
  }
}

C1004Device::clockSource_t C1004Device::getClockSource() {
  C1004Device::clockSource_t source = getMemory("clock_source") ? C1004Device::external : C1004Device::internal;
  LOG(INFO) << "Current C1004 clock source: " << source;
  if(!getMemory("clock_locked"))
    LOG(WARNING) << "The PLL is not locked to the configured clock source. Check the clock source or release reset.";
  return source;
}

namespace caribou {
  std::istream& operator>>(std::istream& is, C1004Device::clockSource_t& source) {
    std::string arg;
    is >> arg;
    if(!arg.compare("internal"))
      source = C1004Device::internal;
    else if(!arg.compare("external"))
      source = C1004Device::external;
    else
      throw std::invalid_argument("Unknown C1004 clock source. Valid clock sources: internal, external.");
    return is;
  }

  std::ostream& operator<<(std::ostream& os, C1004Device::clockSource_t const& source) {
    switch(source) {
    case C1004Device::internal:
      os << "internal";
      break;
    case C1004Device::external:
      os << "external";
      break;
    default:
      throw std::invalid_argument("Unknown C1004 clock source. Valid clock sources: internal, external.");
    }
    return os;
  }
} // namespace caribou
