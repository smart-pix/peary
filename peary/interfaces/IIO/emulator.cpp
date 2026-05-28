/**
 * Caribou IIO interface class implementation
 */

#include "iio.hpp"

caribou::iface_iio::iface_iio(const configuration_type& config)
    : Interface(config), _converter(config._converter), _chantype(config._chantype), _direction(config._direction) {}

caribou::iface_iio::~iface_iio() {}

caribou::iio_t caribou::iface_iio::write(const iio_t& data) {

  if(direction() != iface_iio_direction_t::write)
    throw DeviceException(converter().name() + " doesn't support write operation.");

  switch(data.cmd) {
  case iio_t::ENABLE:
    LOG(DEBUG) << "(emulator) Powering" << (data.enable ? " up " : " down ") << converter().name();
    break;

  case iio_t::SET:
    LOG(DEBUG) << "(emulator) At next power up " + chanType() + " of " + converter().name() + " will be set to value " +
                    to_string(data.value);
    break;
  default:
    break;
  }

  return iio_t();
}

caribou::iio_t caribou::iface_iio::read() {

  if(direction() != iface_iio_direction_t::read)
    throw DeviceException(converter().name() + " doesn't support read operation.");

  LOG(DEBUG) << "(emulator) Read channel " << to_hex_string(converter().address()) << " of " << converter().name();
  return iio_t();
}
