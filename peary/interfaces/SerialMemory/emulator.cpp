/**
 * Caribou Memory interface class emulator
 */

#include "utils/log.hpp"
#include "utils/utils.hpp"

#include "serialmemory.hpp"

using namespace caribou;

iface_sermem::iface_sermem(const configuration_type& config) : iface_mem(static_cast<const iface_mem_config>(config)) {
  _register_address_write = config._addr_write;
  _register_value_write = config._val_write;
  _register_address_read = config._addr_read;
  _register_value_read = config._val_read;
  _status = config._status;
}

std::pair<size_t, uintptr_t> iface_sermem::write(const std::pair<size_t, uintptr_t>& dest) {

  std::pair<size_t, uintptr_t> val(_register_value_write, dest.second);
  LOG(TRACE) << "SERMEM/emu Writing target value 0x" << std::hex << val.second << " to memory address 0x" << val.first
             << std::dec;

  std::pair<size_t, uintptr_t> reg(_register_address_write, static_cast<std::uintptr_t>(dest.first));
  LOG(TRACE) << "SERMEM/emu Writing target address 0x" << std::hex << reg.second << " to memory address 0x" << reg.first
             << std::dec;

  // Check for transaction done status flag
  LOG(TRACE) << "SERMEM/emu Retrieved transaction status from 0x" << std::hex << _status << ", value: " << 0 << std::dec;

  return std::pair<size_t, uintptr_t>();
}

iface_sermem::dataVector_type iface_sermem::read(const size_t& offset, const unsigned int n) {
  dataVector_type values(n);

  // Prepare register
  std::pair<size_t, uintptr_t> reg(_register_address_read, static_cast<std::uintptr_t>(offset));
  LOG(TRACE) << "SERMEM/emu Writing target address 0x" << std::hex << reg.second << " to memory address 0x" << reg.first
             << std::dec;

  LOG(TRACE) << "SERMEM/emu Retrieved transaction status from 0x" << std::hex << _status << ", value: " << 0 << std::dec;

  for(size_t i = 0; i < n; i++) {
    LOG(TRACE) << "Retrieved data word 0x" << std::hex << 0 << " from memory address 0x" << _register_value_read << std::dec;
  }

  return values;
}