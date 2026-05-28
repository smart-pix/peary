/**
 * Caribou IIO interface class implementation
 */

#include "iio.hpp"
#include <cmath>

using namespace caribou;

struct iioChannelPrivateData {
  /** The channel is enabled
   */
  bool on = true;

  /** Voltage/Current of a given channel in SI Volts
   */
  double value = 0;
};

iface_iio::iface_iio(const configuration_type& config)
    : Interface(config), _converter(config._converter), _chantype(config._chantype), _direction(config._direction),
      _isOn(false), _voltage(0) {

  // Initial IIO context
  LOG(TRACE) << "Creating IIO context";
  iio_ctx = iio_create_context_from_uri(devicePath().c_str());

  if(iio_ctx == nullptr) {
    throw DeviceException("Failed to create IIO context");
  }

  LOG(DEBUG) << "IIO context contains " << iio_context_get_devices_count(iio_ctx) << " devices";

  getIIOChannel();
}

iface_iio::~iface_iio() {
  LOG(TRACE) << "Destroying IIO context";
  iio_context_destroy(iio_ctx);
}

iio_t iface_iio::write(const iio_t& data) {

  if(direction() != iface_iio_direction_t::write) {
    throw DeviceException(converter().name() + " doesn't support write operation.");
  }

  switch(data.cmd) {
  case iio_t::SET:
    _voltage = data.value;
    if(_isOn)
      setIIOChannel();
    break;

  case iio_t::ENABLE:
    if(data.enable)
      setIIOChannel();
    else
      powerDownIIOChannel();
    break;
  default:
    throw CommunicationError("Unknown IIO command");
  }

  return {};
}

iio_t iface_iio::read() {

  double scale = NAN;
  double value = NAN;
  iio_t result;

  if(direction() != iface_iio_direction_t::read) {
    throw DeviceException(converter().name() + " doesn't support read operation.");
  }

  if(iio_channel_attr_read_double(_channel, "scale", &scale) != 0) {
    throw DeviceException("Failed to obtain " + chanType() + " scale for " + converter().name() + "(" +
                          std::strerror(errno) + ")");
  }

  if(iio_channel_attr_read_double(_channel, "raw", &value) != 0) {
    throw DeviceException("Failed to read " + converter().name() + " " + chanType() + " (" + std::strerror(errno) + ")");
  }

  result.value = value * scale / 1000;

  LOG(TRACE) << "Read channel " << iio_channel_get_id(_channel) << " at "
             << to_hex_string(converter().address()) + " with raw value " << value << " coresponding to " << result.value
             << "V";

  return result;
}

void iface_iio::getIIOChannel() {
  const std::string iio_id = "iio:device" + std::to_string(static_cast<int>(converter().address()));
  const std::string chan_id = chanType() + std::to_string(static_cast<int>(converter().channel()));
  const iio_device* iio_dev = iio_context_find_device(iio_ctx, iio_id.c_str());

  if(iio_dev == nullptr) {
    throw ConfigInvalid("Device " + iio_id + " not found");
  }

  _channel = iio_device_find_channel(iio_dev, chan_id.c_str(), direction() != 0);
  if(_channel == nullptr) {
    throw ConfigInvalid("Channel " + std::string(chan_id) + " on device " + iio_id + " not found");
  }
}

void iface_iio::setIIOChannel() {
  double scale = NAN;
  double value = NAN;

  if(iio_channel_attr_read_double(_channel, "scale", &scale) != 0) {
    throw DeviceException("Failed to obtain " + chanType() + " scale for " + converter().name() + "(" +
                          std::strerror(errno) + ")");
  }

  value = _voltage * 1000 / scale;

  LOG(DEBUG) << "Powering up channel " << iio_channel_get_id(_channel) << " at "
             << to_hex_string(converter().address()) + " with raw value " << value;

  if(iio_channel_attr_write_double(_channel, "raw", value) != 0) {
    throw DeviceException("Failed to set " + converter().name() + " " + chanType() + " (" + std::strerror(errno) + ")");
  }

  _isOn = true;
}

void iface_iio::powerDownIIOChannel() {
  LOG(DEBUG) << "Powering down channel " << iio_channel_get_id(_channel) << " at " << to_hex_string(converter().address());
  if(iio_channel_attr_write_bool(_channel, "powerdown", true) != 0) {
    throw DeviceException("Failed to powerdown " + converter().name() + "(" + std::strerror(errno) + ")");
  }

  _isOn = false;
}
