/**
 * Caribou Memory interface class implementation
 */

#include "memory.hpp"
using namespace caribou;

iface_mem_config::iface_mem_config(std::string const& devpath, const memory_map& mem)
    : InterfaceConfiguration(devpath), _mem(mem) {}

bool iface_mem_config::operator<(const iface_mem_config& rhs) const {
  if(!InterfaceConfiguration::operator<(rhs) && !rhs.InterfaceConfiguration::operator<(*this)) {
    return _mem < rhs._mem;
  } else {
    return InterfaceConfiguration::operator<(rhs);
  }
}
