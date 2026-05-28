#ifndef CARIBOU_HAL_MEMORY_HPP
#define CARIBOU_HAL_MEMORY_HPP

#include <cstdint>
#include <cstdio>
#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <vector>

#include "interfaces/Interface.hpp"
#include "interfaces/InterfaceManager.hpp"
#include "utils/datatypes.hpp"
#include "utils/exceptions.hpp"

namespace caribou {

  class iface_mem_config : public InterfaceConfiguration {
  public:
    iface_mem_config(std::string const& devicepath, const memory_map& mem);

    memory_map _mem;

    virtual bool operator<(const iface_mem_config& rhs) const;
  };

  class iface_mem : public Interface<size_t, uintptr_t, iface_mem_config> {

  private:
    // Default constructor: private (only created by InterfaceManager)
    //
    // It can throw DeviceException
  public:
    explicit iface_mem(const configuration_type& config);

    ~iface_mem() override;

    // Access to FPGA memory mapped registers
    int _memfd;

    // Buffer
    void* _mappedMemory;

    GENERATE_FRIENDS()

    memory_map mem() const { return _mem; }

    // Remove default constructor
    iface_mem() = delete;

  protected:
    std::pair<size_t, uintptr_t> write(const std::pair<size_t, uintptr_t>&) override;
    uintptr_t readWord(const size_t) const;
    dataVector_type read(const size_t&, const unsigned int) override;
    dataVector_type read_opt(const size_t& offset, const unsigned int n);

  private:
    const memory_map _mem;

    void mapMemory(const memory_map&);

    // only this function can create the interface
    friend iface_mem& InterfaceManager::getInterface<iface_mem>(const configuration_type&);

    // only this function can delete the interface
    friend void InterfaceManager::deleteInterface<iface_mem>(iface_mem*);
  };

} // namespace caribou

#endif /* CARIBOU_HAL_MEMORY_HPP */
