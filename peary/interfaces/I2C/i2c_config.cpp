/**
 * Caribou I2C interface class implementation
 */

#include "i2c.hpp"

using namespace caribou;

iface_i2c_config::iface_i2c_config(std::string const& devpath, const i2c_address_t devaddress)
    : InterfaceConfiguration(devpath), _devaddress(devaddress) {}

bool iface_i2c_config::operator<(const iface_i2c_config& rhs) const {
  if(!InterfaceConfiguration::operator<(rhs) && !rhs.InterfaceConfiguration::operator<(*this)) {
    return _devaddress < rhs._devaddress;
  } else {
    return InterfaceConfiguration::operator<(rhs);
  }
}
