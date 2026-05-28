/**
 * Caribou Falconboard HAL class implementation
 */

#include "Falconboard.hpp"
#include <cmath>
#include "interfaces/IIO/iio.hpp"

using namespace caribou;
using namespace falconboard;

Falconboard::Falconboard() {

  // Log the firmware
  LOG(STATUS) << getFirmwareVersion();

  this->Initialize();
}

std::string Falconboard::getFirmwareVersion() {

  const auto firmwareVersion = this->readMemory(firmware_mem, CARIBOU_FIRMWARE_VERSION_OFFSET);

  std::stringstream s;
  s << "Firmware version: " << to_hex_string(firmwareVersion);

  return s.str();
}

void Falconboard::generalReset() {
  // Disable DAC outputs
  LOG(DEBUG) << "Disable all DACs channels";
  powerBiasRegulator(BIAS_1, false);
  powerBiasRegulator(BIAS_2, false);
  powerBiasRegulator(BIAS_3, false);
  powerBiasRegulator(BIAS_4, false);
  powerBiasRegulator(BIAS_5, false);
  powerBiasRegulator(BIAS_6, false);
  powerBiasRegulator(BIAS_7, false);
  powerBiasRegulator(BIAS_8, false);
  powerBiasRegulator(BIAS_9, false);
  powerBiasRegulator(BIAS_10, false);
  powerBiasRegulator(BIAS_11, false);
  powerBiasRegulator(BIAS_12, false);

  LOG(DEBUG) << "Disable laser high voltage DC/DC converter";
  powerDCDCConverter(DCDC_VCSEL, false);
  setDCDCConverter(DCDC_VCSEL, 13.5); // set minimum voltage

  caribouHAL::generalReset();
}

uintptr_t Falconboard::getFirmwareRegister(uint16_t) {
  throw FirmwareException("Functionality not implemented.");
}

uint8_t Falconboard::getBoardID() {

  throw FirmwareException("Functionality not implemented.");

  // LOG(DEBUG) << "Reading board ID from CaR EEPROM";
  // iface_i2c& myi2c = InterfaceManager::getInterface<iface_i2c>(BUS_I2C0);

  // // Read one word from memory address on the EEPROM:
  // // FIXME register address not set!
  // std::vector<uint8_t> data = myi2c.wordread(ADDR_EEPROM, 0x0, 1);

  // if(data.empty())
  //   throw CommunicationError("No data returned");
  // return data.front();
}

uint8_t Falconboard::getBoardRev() {
  throw FirmwareException("Functionality not implemented.");
}

uint8_t Falconboard::readEEPROM(uint16_t) {
  throw FirmwareException("Functionality not implemented.");
}

void Falconboard::writeEEPROM(uint16_t, int) {
  throw FirmwareException("Functionality not implemented.");
}

double Falconboard::readTemperature() {
  throw FirmwareException("Functionality not implemented.");
}

void Falconboard::setBiasRegulator(const BIAS_REGULATOR_T regulator, const double voltage) {
  LOG(DEBUG) << "Setting bias " << voltage << "V "
             << "on " << regulator.name();

  // actual voltage to be set
  double voltage_ = voltage;

  // boosted DACs
  if(regulator.name() == BIAS_8.name() || regulator.name() == BIAS_12.name()) {
    if(voltage > 30 || voltage < 0) {
      throw ConfigInvalid("Trying to set bias regulator to " + std::to_string(voltage) + " V (range is 0-30 V)");
    }

    voltage_ /= DAC_BOOST_AMPLIFICATION_RATIO;

  } else // regular DAC
    if(voltage > 2.5 || voltage < 0) {
      throw ConfigInvalid("Trying to set bias regulator to " + std::to_string(voltage) + " V (range is 0-2.5 V)");
    }

  setDACVoltage(regulator, voltage_);
}

void Falconboard::powerBiasRegulator(const BIAS_REGULATOR_T regulator, const bool enable) {
  LOG(DEBUG) << (enable ? "Powering up bias " : "Powering down bias ") << regulator.name();

  powerDAC(regulator, enable);
}

void Falconboard::setVoltageRegulator(const VOLTAGE_REGULATOR_T, const double, const double) {
  throw FirmwareException("Functionality not supported.");
}

void Falconboard::powerVoltageRegulator(const VOLTAGE_REGULATOR_T, const bool) {
  throw FirmwareException("Functionality not supported.");
}

void Falconboard::setCurrentSource(const CURRENT_SOURCE_T, const unsigned int, const CURRENT_SOURCE_POLARITY_T) {

  throw FirmwareException("Functionality not supported.");
}

void Falconboard::powerCurrentSource(const CURRENT_SOURCE_T, const bool) {
  throw FirmwareException("Functionality not supported.");
}

void Falconboard::disableSI5345() {
  throw FirmwareException("Functionality not supported.");
}

void Falconboard::configureSI5345(SI5345_REG_T const* const, const size_t) {
  throw FirmwareException("Functionality not supported.");
}

bool Falconboard::isLockedSI5345() {
  throw FirmwareException("Functionality not supported.");
}

void Falconboard::configurePulser(unsigned, const bool, const bool, const bool) {
  throw FirmwareException("Functionality not supported.");
}

void Falconboard::startPulser(unsigned) {
  throw FirmwareException("Functionality not supported.");
}

void Falconboard::enablePulser(unsigned) {
  throw FirmwareException("Functionality not supported.");
}

void Falconboard::disablePulser(unsigned) {
  throw FirmwareException("Functionality not supported.");
}

uint32_t Falconboard::getPulserRunning() {
  throw FirmwareException("Functionality not supported.");
}

uint32_t Falconboard::getPulseCount(const uint32_t) {
  throw FirmwareException("Functionality not supported.");
}

void Falconboard::configurePulseParameters(const unsigned, const uint32_t, const uint32_t, const uint32_t, const double) {
  throw FirmwareException("Functionality not supported.");
}

void Falconboard::setDACVoltage(const component_dac_t& device, const double voltage) {
  LOG(DEBUG) << "Setting " << voltage << "V on " << device.name();

  auto& iio = InterfaceManager::getInterface<iface_iio>(iface_iio::configuration_type("local:", "voltage", device));
  iio_t iio_data;
  iio_data.cmd = iio_t::SET;
  iio_data.value = voltage;
  iio.write(iio_data);
}

void Falconboard::powerDAC(const component_dac_t& device, const bool enable) {
  LOG(DEBUG) << (enable ? "Powering up " : "Powering down ") << device.name();

  auto& iio = InterfaceManager::getInterface<iface_iio>(iface_iio::configuration_type("local:", "voltage", device));

  iio_t iio_data;
  iio_data.cmd = iio_t::ENABLE;
  iio_data.enable = enable;
  iio.write(iio_data);
}

void Falconboard::setCurrentMonitor(const uint8_t, const double, bool) {
  throw FirmwareException("Functionality not supported.");
}

void Falconboard::setDCDCConverter(const DCDC_CONVERTER_T converter, const double voltage) {
  LOG(DEBUG) << "Setting " << voltage << "V "
             << "on " << converter.name();

  setDACVoltage(converter, ((1.6 - (voltage * 0.01977)) / 0.3296));
  powerDAC(converter, true);
}

void Falconboard::powerDCDCConverter(const DCDC_CONVERTER_T converter, const bool enable) {
  LOG(DEBUG) << (enable ? "Enabling " : "Disabling ") << converter.name();

  // Falcon has only one converter
  auto reg = this->readMemory(reg_laser_en, FALCON_LASER_HV_ENABLE_OFFSET) & ~FALCON_LASER_HV_ENABLE_MASK;
  if(enable) {
    reg = reg | FALCON_LASER_HV_ENABLE_MASK;
  }
  this->writeMemory(reg_laser_en, FALCON_LASER_HV_ENABLE_OFFSET, reg);
}

double Falconboard::measureCurrent(const VOLTAGE_REGULATOR_T) {
  throw FirmwareException("Functionality not supported.");
}

double Falconboard::measurePower(const VOLTAGE_REGULATOR_T) {
  throw FirmwareException("Functionality not supported.");
}

double Falconboard::measureVoltage(const VOLTAGE_REGULATOR_T) {
  throw FirmwareException("Functionality not supported.");
}

double Falconboard::readSlowADC(const SLOW_ADC_CHANNEL_T channel) {

  LOG(DEBUG) << "Sampling " << channel.name() << " voltage on channel " << static_cast<int>(channel.channel())
             << " of ADC TI122S051 at iio address " << to_hex_string(channel.address());

  auto& iio = InterfaceManager::getInterface<iface_iio>(iface_iio::configuration_type("local:", "voltage", channel));

  if(channel.name() == ADC_VIN_VCSEL.name()) {
    return iio.read().value / 0.02299;
  } else {
    return iio.read().value;
  }
}

void Falconboard::setInputCMOSLevel(double) {
  throw FirmwareException("Functionality not implemented.");
}

void Falconboard::setOutputCMOSLevel(double) {
  throw FirmwareException("Functionality not implemented.");
}

void Falconboard::monitorAlert() {
  throw FirmwareException("Functionality not implemented.");
}

bool Falconboard::getAlertStatus(const GPIO_T) {
  throw FirmwareException("Functionality not implemented.");
}
