/**
 * Caribou Loopback interface class implementation
 */

#include "loopback.hpp"

using namespace caribou;

iface_loopback_config::iface_loopback_config(std::string const& devpath, const uint8_t devaddress)
    : InterfaceConfiguration(devpath), _devaddress(devaddress) {}

bool iface_loopback_config::operator<(const iface_loopback_config& rhs) const {
  if(!InterfaceConfiguration::operator<(rhs) && !rhs.InterfaceConfiguration::operator<(*this)) {
    return _devaddress < rhs._devaddress;
  } else {
    return InterfaceConfiguration::operator<(rhs);
  }
}
