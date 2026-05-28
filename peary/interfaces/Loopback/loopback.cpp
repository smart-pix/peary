/**
 * Caribou Loopback interface class implementation
 */

#include "loopback.hpp"

#include "utils/log.hpp"
#include "utils/utils.hpp"

using namespace caribou;

iface_loopback::iface_loopback(const configuration_type& config) : Interface(config), _devAddress(config._devaddress) {
  LOG(TRACE) << "Opened LOOPBACK device at " << devicePath();
}

iface_loopback::~iface_loopback() {
  LOG(TRACE) << "Closed LOOPBACK device at " << devicePath();
}

uint8_t iface_loopback::write(const uint8_t& data) {
  std::lock_guard<std::mutex> lock(mutex);

  LOG(TRACE) << std::hex << "LOOPBACK (" << devicePath() << ") : Writing data \"" << static_cast<int>(data)
             << "\" at address " << to_hex_string(devAddress()) << std::dec;

  return data;
}

iface_loopback::dataVector_type iface_loopback::write(const dataVector_type& data) {
  std::lock_guard<std::mutex> lock(mutex);

  LOG(TRACE) << std::hex << "LOOPBACK (" << devicePath() << ") : Writing data \"" << listVector(data) << "\" at address "
             << to_hex_string(devAddress()) << std::dec;

  return data;
}

std::pair<uint8_t, uint8_t> iface_loopback::write(const std::pair<uint8_t, uint8_t>& data) {

  std::lock_guard<std::mutex> lock(mutex);

  LOG(TRACE) << std::hex << "LOOPBACK (" << devicePath() << ") : Writing data \"" << static_cast<int>(data.second)
             << "\" to register " << to_hex_string(data.first) << " at address " << to_hex_string(devAddress()) << std::dec;

  return data;
}

iface_loopback::dataVector_type iface_loopback::write(const uint8_t& reg, const dataVector_type& data) {

  std::lock_guard<std::mutex> lock(mutex);

  LOG(TRACE) << std::hex << "LOOPBACK (" << devicePath() << ") : Writing block data: \"" << listVector(data) << "\"";
  LOG(TRACE) << " to register " << to_hex_string(reg) << " at address " << to_hex_string(devAddress()) << std::dec;
  return data;
}

std::vector<std::pair<uint8_t, uint8_t>> iface_loopback::write(const std::vector<std::pair<uint8_t, uint8_t>>& data) {

  std::lock_guard<std::mutex> lock(mutex);

  LOG(TRACE) << std::hex << "LOOPBACK (" << devicePath() << ") : Writing block data to registers:";
  for(const auto& i : data) {
    LOG(TRACE) << to_hex_string(i.first) << " | " << static_cast<int>(i.second);
  }
  LOG(TRACE) << "at address " << to_hex_string(devAddress()) << std::dec;

  return data;
}

iface_loopback::dataVector_type iface_loopback::read(const unsigned int length) {
  std::lock_guard<std::mutex> lock(mutex);
  dataVector_type data;

  LOG(TRACE) << std::hex << "LOOPBACK (" << devicePath() << ") address " << to_hex_string(devAddress())
             << ": Read data  - returning address." << std::dec;

  for(unsigned int i = 0; i < length; i++) {
    data.push_back(devAddress());
  }
  return data;
}

iface_loopback::dataVector_type iface_loopback::read(const uint8_t& reg, const unsigned int length) {

  std::lock_guard<std::mutex> lock(mutex);
  dataVector_type data;

  LOG(TRACE) << std::hex << "LOOPBACK (" << devicePath() << ") address " << to_hex_string(devAddress())
             << ": Read data from register " << to_hex_string(reg) << " - returning address." << std::dec;

  for(unsigned int i = 0; i < length; i++) {
    data.push_back(devAddress());
  }
  return data;
}
