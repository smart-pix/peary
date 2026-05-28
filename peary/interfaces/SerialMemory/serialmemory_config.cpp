/**
 * Caribou Serial Memory config class implementation
 */

#include "interfaces/Memory/memory.hpp"
#include "serialmemory.hpp"

using namespace caribou;

iface_sermem_config::iface_sermem_config(std::string const& devicepath,
                                         const memory_map& mem,
                                         const size_t reg_addr_write,
                                         const size_t reg_value_write,
                                         const size_t reg_addr_read,
                                         const size_t reg_value_read,
                                         const size_t status)
    : iface_mem_config(devicepath, mem), _addr_write(reg_addr_write), _val_write(reg_value_write), _addr_read(reg_addr_read),
      _val_read(reg_value_read), _status(status){};

bool iface_sermem_config::operator<(const iface_sermem_config& rhs) const {
  if(!iface_mem_config::operator<(rhs) && !rhs.iface_mem_config::operator<(*this)) {
    if(_addr_write == rhs._addr_write) {
      return _val_write < rhs._val_write;
    } else {
      return _addr_write < rhs._addr_write;
    }
  } else {
    return iface_mem_config::operator<(rhs);
  }
}
