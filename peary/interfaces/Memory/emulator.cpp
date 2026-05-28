/**
 * Caribou Memory interface class emulator
 */

#include "utils/log.hpp"
#include "utils/utils.hpp"

#include "memory.hpp"

using namespace caribou;

iface_mem::iface_mem(const configuration_type& config)
    : Interface(config), _memfd(), _mappedMemory(nullptr), _mem(config._mem) {
  LOG(TRACE) << "Opened emulated memory device at " << devicePath();
}

iface_mem::~iface_mem() {}

std::pair<size_t, uintptr_t> iface_mem::write(const std::pair<size_t, uintptr_t>& dest) {
  LOG(TRACE) << "MEM/emu Writing to mapped memory at " << std::hex << mem().getBaseAddress() << ", offset " << dest.first
             << std::dec << ": " << dest.second;
  return std::pair<size_t, uintptr_t>();
}

uintptr_t iface_mem::readWord(const size_t offset) const {
  LOG(TRACE) << "MEM/emu Reading from mapped memory at " << std::hex << mem().getBaseAddress() << ", offset " << offset
             << std::dec;
  return 0;
}

iface_mem::dataVector_type iface_mem::read(const size_t& offset, const unsigned int n) {
  dataVector_type values;
  for(unsigned int i = 0; i < n; i++) {
    values.push_back(this->readWord(offset));
  }
  return values;
}
