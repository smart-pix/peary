/**
 * Caribou I2C interface emulator
 */

#include <cerrno>
#include <cstring>

#include "utils/log.hpp"
#include "utils/utils.hpp"

#include "i2c.hpp"

using namespace caribou;

iface_i2c::iface_i2c(const configuration_type& config) : Interface(config), _devAddress(config._devaddress) {
  LOG(TRACE) << "Opened I2C device at " << devicePath();
  setAddress(devAddress());
}

iface_i2c::~iface_i2c() {
  LOG(TRACE) << "Closing I2C device " << devicePath();
}

void iface_i2c::setAddress(i2c_address_t const) {
  LOG(TRACE) << "Talking to I2C slave at address " << to_hex_string(devAddress());
}

i2c_t iface_i2c::write(const i2c_t& data) {

  LOG(TRACE) << "I2C/emu (" << devicePath() << ") address " << to_hex_string(devAddress()) << ": Writing data \""
             << to_hex_string(data) << "\"";

  return 0;
}

std::pair<i2c_reg_t, i2c_t> iface_i2c::write(const std::pair<i2c_reg_t, i2c_t>& data) {

  LOG(TRACE) << "I2C/emu (" << devicePath() << ") address " << to_hex_string(devAddress()) << ": Register "
             << to_hex_string(data.first) << " Writing data \"" << to_hex_string(data.second) << "\"";

  return std::make_pair(0, 0);
}

iface_i2c::dataVector_type iface_i2c::write(const i2c_t& reg, const dataVector_type& data) {

  LOG(TRACE) << "I2C/emu (" << devicePath() << ") address " << to_hex_string(devAddress()) << ": Register "
             << to_hex_string(reg) << "\n\t Writing block data: \"" << listVector(data, ", ", true) << "\"";

  return dataVector_type();
}

iface_i2c::dataVector_type iface_i2c::read(const unsigned int length) {

  dataVector_type data;

  if(length != 1)
    throw CommunicationError("Read operation of multiple data from the device without internal registers is not possible");

  data.resize(1);

  LOG(TRACE) << "I2C/emu (" << devicePath() << ") address " << to_hex_string(devAddress()) << ": Read data \""
             << to_hex_string(data[0]) << "\"";

  return data;
}

iface_i2c::dataVector_type iface_i2c::read(const i2c_t& reg, const unsigned int length) {

  dataVector_type data;

  data.resize(length);

  LOG(TRACE) << "I2C/emu (" << devicePath() << ") address " << to_hex_string(devAddress()) << ": Register "
             << to_hex_string(reg) << "\n\t Read block data \"" << listVector(data, ", ", true) << "\"";

  return data;
}

iface_i2c::dataVector_type iface_i2c::wordwrite(const uint16_t&, const dataVector_type&) {
  return dataVector_type();
}

iface_i2c::dataVector_type iface_i2c::wordread(const uint16_t, const unsigned int n) {
  return dataVector_type(n);
}

bool iface_i2c::probe(const i2c_reg_t&) const {
  return true;
}
