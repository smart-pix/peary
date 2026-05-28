/**
 * Caribou IIO interface class implementation
 */

#include "iio.hpp"

using namespace caribou;

iface_iio_config::iface_iio_config(const std::string& devpath, std::string chantype, const component_dac_t& dac)
    : InterfaceConfiguration(devpath), _converter(dac), _chantype(std::move(chantype)), _direction(write) {}
iface_iio_config::iface_iio_config(const std::string& devpath, std::string chantype, const component_adc_t& adc)
    : InterfaceConfiguration(devpath), _converter(adc), _chantype(std::move(chantype)), _direction(read) {}

bool iface_iio_config::operator<(const iface_iio_config& rhs) const {
  if(!InterfaceConfiguration::operator<(rhs) && !rhs.InterfaceConfiguration::operator<(*this)) {
    if(_converter.address() == rhs._converter.address()) {
      if(_converter.channel() == rhs._converter.channel()) {
        if(_chantype != rhs._chantype) {
          return _direction < rhs._direction;
        } else {
          return _chantype < rhs._chantype;
        }
      } else {
        return _converter.channel() < rhs._converter.channel();
      }
    } else {
      return _converter.address() < rhs._converter.address();
    }
  } else {
    return InterfaceConfiguration::operator<(rhs);
  }
}
