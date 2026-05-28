/**
 * Caribou Serial Memory interface class implementation
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

  // Prepare value and write to to memory
  std::pair<size_t, uintptr_t> val(_register_value_write, dest.second);
  LOG(TRACE) << "Writing target value 0x" << std::hex << val.second << " to memory address 0x" << val.first << std::dec;
  iface_mem::write(val);

  // Prepare register
  std::pair<size_t, uintptr_t> reg(_register_address_write, static_cast<std::uintptr_t>(dest.first));
  LOG(TRACE) << "Writing target address 0x" << std::hex << reg.second << " to memory address 0x" << reg.first << std::dec;
  // Writing into the register will trigger the transaction:
  iface_mem::write(reg);

  // Check for transaction done status flag
  // FIXME this should provide a safe exit
  uintptr_t status = 0;
  while(status == 0) {
    status = iface_mem::readWord(_status);
  }
  LOG(TRACE) << "Retrieved transaction status from 0x" << std::hex << _status << ", value: 0x" << status << std::dec;

  // Check the transaction error flag:
  if(status > 1) {
    throw CommunicationError("Issue writing register, device error code " + std::to_string(status >> 1));
  }

  return std::pair<size_t, uintptr_t>();
}

iface_sermem::dataVector_type iface_sermem::read(const size_t& offset, const unsigned int n) {

  // Prepare register
  std::pair<size_t, uintptr_t> reg(_register_address_read, static_cast<std::uintptr_t>(offset));
  LOG(TRACE) << "Writing target address 0x" << std::hex << reg.second << " to memory address 0x" << reg.first << std::dec;
  // Writing into the register will trigger the transaction:
  iface_mem::write(reg);

  // Check for transaction done status flag
  // FIXME this should provide a safe exit
  uintptr_t status = 0;
  while(status == 0) {
    status = iface_mem::readWord(_status);
  }
  LOG(TRACE) << "Retrieved transaction status from 0x" << std::hex << _status << ", value: 0x" << status << std::dec;

  // Check the transaction error flag:
  if(status > 1) {
    throw CommunicationError("Issue writing register, device error code " + std::to_string(status >> 2));
  }

  // Now let's read - have to rethink the "n" here...
  dataVector_type values;
  for(unsigned int i = 0; i < n; i++) {
    values.push_back(iface_mem::readWord(_register_value_read));
    LOG(TRACE) << "Retrieved data word 0x" << std::hex << values.back() << " from memory address 0x" << _register_value_read
               << std::dec;
  }

  return values;
}
