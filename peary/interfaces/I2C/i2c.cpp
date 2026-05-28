/**
 * Caribou I2C interface class implementation
 */

#include <cerrno>
#include <cstring>

// OS I2C support
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

extern "C" {
#include <i2c/smbus.h>
#include <linux/i2c-dev.h>
}

#include "utils/log.hpp"
#include "utils/utils.hpp"

#include "i2c.hpp"

using namespace caribou;

iface_i2c::iface_i2c(const configuration_type& config) : Interface(config), i2cDesc(), _devAddress(config._devaddress) {
  if((i2cDesc = open(devicePath().c_str(), O_RDWR)) < 0) {
    throw DeviceException("Open " + devicePath() + " device failed. " + std::strerror(i2cDesc));
  }
  LOG(TRACE) << "Opened I2C device at " << devicePath();

  setAddress(devAddress());
}

iface_i2c::~iface_i2c() {
  LOG(TRACE) << "Closing I2C device " << devicePath();
  close(i2cDesc);
}

void iface_i2c::setAddress(i2c_address_t const address) {
  if(ioctl(i2cDesc, I2C_SLAVE, address) < 0) {
    throw CommunicationError("Failed to acquire bus access and/or talk to slave (" + to_hex_string(address) + ") on " +
                             devicePath() + ": " + std::strerror(errno));
  }
}

i2c_t iface_i2c::write(const i2c_t& data) {

  LOG(TRACE) << "I2C (" << devicePath() << ") address " << to_hex_string(devAddress()) << ": Writing data \""
             << to_hex_string(data) << "\"";

  if(i2c_smbus_write_byte(i2cDesc, data) != 0) {
    throw CommunicationError("Failed to write slave (" + to_hex_string(devAddress()) + ") on " + devicePath() + ": " +
                             std::strerror(errno));
  }

  return 0;
}

std::pair<i2c_reg_t, i2c_t> iface_i2c::write(const std::pair<i2c_reg_t, i2c_t>& data) {

  LOG(TRACE) << "I2C (" << devicePath() << ") address " << to_hex_string(devAddress()) << ": Register "
             << to_hex_string(data.first) << " Writing data \"" << to_hex_string(data.second) << "\"";

  if(i2c_smbus_write_byte_data(i2cDesc, data.first, data.second) != 0) {
    throw CommunicationError("Failed to write slave (" + to_hex_string(devAddress()) + ") on " + devicePath() + ": " +
                             std::strerror(errno));
  }

  return std::make_pair(0, 0);
}

iface_i2c::dataVector_type iface_i2c::write(const i2c_t& reg, const dataVector_type& data) {

  LOG(TRACE) << "I2C (" << devicePath() << ") address " << to_hex_string(devAddress()) << ": Register " << to_hex_string(reg)
             << "\n\t Writing block data: \"" << listVector(data, ", ", true) << "\"";

  if(i2c_smbus_write_i2c_block_data(i2cDesc, reg, static_cast<uint8_t>(data.size()), data.data()) != 0) {
    throw CommunicationError("Failed to block write slave (" + to_hex_string(devAddress()) + ") on " + devicePath() + ": " +
                             std::strerror(errno));
  }

  return dataVector_type();
}

iface_i2c::dataVector_type iface_i2c::read(const unsigned int length) {
  dataVector_type data;

  if(length != 1) {
    throw CommunicationError("Read operation of multiple data from the device without internal registers is not possible");
  }

  int temp = i2c_smbus_read_byte(i2cDesc);
  if(temp < 0) {
    throw CommunicationError("Failed to read slave (" + to_hex_string(devAddress()) + ") on " + devicePath() + ": " +
                             std::strerror(errno));
  }

  data.push_back(static_cast<i2c_t>(temp));

  LOG(TRACE) << "I2C (" << devicePath() << ") address " << to_hex_string(devAddress()) << ": Read data \""
             << to_hex_string(data.front()) << "\"";
  return data;
}

iface_i2c::dataVector_type iface_i2c::read(const i2c_reg_t& reg, const unsigned int length) {

  dataVector_type data;

  data.resize(length);
  if(i2c_smbus_read_i2c_block_data(i2cDesc, reg, static_cast<uint8_t>(length), data.data()) != static_cast<int>(length)) {
    throw CommunicationError("Failed to read slave (" + to_hex_string(devAddress()) + ") on " + devicePath() + ": " +
                             std::strerror(errno));
  }

  LOG(TRACE) << "I2C (" << devicePath() << ") address " << to_hex_string(devAddress()) << ": Register " << to_hex_string(reg)
             << "\n\t Read block data \"" << listVector(data, ", ", true) << "\"";
  return data;
}

iface_i2c::dataVector_type iface_i2c::wordwrite(const uint16_t& reg, const dataVector_type& data) {

  dataVector_type d;

  d.push_back(static_cast<i2c_t>(reg >> 8));
  d.push_back(static_cast<i2c_t>(reg & 0xFF));
  d.insert(d.end(), data.begin(), data.end());

  if(::write(i2cDesc, d.data(), d.size()) != static_cast<long int>(d.size())) {
    throw CommunicationError("Failed to write slave (" + to_hex_string(devAddress()) + ") on " + devicePath() + ": " +
                             std::strerror(errno));
  }

  return dataVector_type();
}

iface_i2c::dataVector_type iface_i2c::wordread(const uint16_t reg, const unsigned int length) {

  i2c_msg msg[2];
  i2c_rdwr_ioctl_data msgset;

  dataVector_type data;
  data.resize(length);

  i2c_t addr[2] = {static_cast<i2c_t>(reg >> 8), static_cast<i2c_t>(reg & 0xFF)};

  msg[0].addr = devAddress();
  msg[0].len = 2;
  msg[0].buf = addr;
  msg[0].flags = 0;

  msg[1].addr = devAddress();
  msg[1].len = static_cast<uint16_t>(length);
  msg[1].buf = data.data();
  msg[1].flags = I2C_M_RD;

  msgset.msgs = msg;
  msgset.nmsgs = 2;

  if(ioctl(i2cDesc, I2C_RDWR, &msgset) < 0) {
    throw CommunicationError("Failed to read slave (" + to_hex_string(devAddress()) + ") on " + devicePath() + ": " +
                             std::strerror(errno));
  }

  return data;
}

bool iface_i2c::probe(const i2c_reg_t&) const {
  return i2c_smbus_read_byte(i2cDesc) >= 0;
}
