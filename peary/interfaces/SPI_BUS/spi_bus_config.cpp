/**
 * Caribou SPI interface class implementation
 */

#include "spi_bus.hpp"

using namespace caribou;

iface_spi_bus_config::iface_spi_bus_config(
  std::string const& devpath, const uint8_t addressBits, const uint8_t dataBits, const bool ws, const bool alignMSB)
    : InterfaceConfiguration(devpath), _addressBits(addressBits), _dataBits(dataBits), _ws(ws), _alignMSB(alignMSB) {}

bool iface_spi_bus_config::operator<(const iface_spi_bus_config& rhs) const {
  if(!InterfaceConfiguration::operator<(rhs) && !rhs.InterfaceConfiguration::operator<(*this)) {
    if(_addressBits == rhs._addressBits) {
      if(_dataBits == rhs._dataBits) {
        if(_ws == rhs._ws) {
          return static_cast<int>(_alignMSB) < static_cast<int>(rhs._alignMSB);
        } else {
          return static_cast<int>(_ws) < static_cast<int>(rhs._ws);
        }
      } else {
        return _dataBits < rhs._dataBits;
      }
    } else {
      return _addressBits < rhs._addressBits;
    }
  } else {
    return InterfaceConfiguration::operator<(rhs);
  }
}
