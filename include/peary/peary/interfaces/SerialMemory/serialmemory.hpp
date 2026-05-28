#ifndef CARIBOU_HAL_SERIALMEMORY_HPP
#define CARIBOU_HAL_SERIALMEMORY_HPP

#include <cstdint>
#include <cstdio>
#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <vector>

#include "interfaces/Memory/memory.hpp"
#include "utils/datatypes.hpp"
#include "utils/exceptions.hpp"

namespace caribou {

  class iface_sermem_config : public iface_mem_config {
  public:
    iface_sermem_config(std::string const& devicepath,
                        const memory_map& mem,
                        const size_t reg_addr_write,
                        const size_t reg_value_write,
                        const size_t reg_addr_read,
                        const size_t reg_value_read,
                        const size_t status);

    size_t _addr_write;
    size_t _val_write;
    size_t _addr_read;
    size_t _val_read;
    size_t _status;

    virtual bool operator<(const iface_sermem_config& rhs) const;
  };

  class iface_sermem : public iface_mem {
  public:
    using reg_type = size_t;
    using data_type = uintptr_t;
    using configuration_type = iface_sermem_config;
    using dataVector_type = std::vector<data_type>;

    explicit iface_sermem(const configuration_type& config);

    ~iface_sermem() = default;

    // Remove default constructor
    iface_sermem() = delete;

    GENERATE_FRIENDS()

  private:
    // Transactions are triggered by writing into the address registers!

    // Register for storing the target address and target values for write operations:
    size_t _register_address_write;
    size_t _register_value_write;
    // Register for storing the target address and retrieved values for read operations:
    size_t _register_address_read;
    size_t _register_value_read;
    // One status register that contains
    // - transaction status [0] (high when transaction done),
    // - error flag [1] (high when there was an error) and
    // - optionally the device error code [31:2]
    size_t _status;

    std::pair<size_t, uintptr_t> write(const std::pair<size_t, uintptr_t>&) override;

    dataVector_type read(const size_t&, const unsigned int) override;

    // only this function can create the interface
    friend iface_sermem& InterfaceManager::getInterface<iface_sermem>(const configuration_type&);

    // only this function can delete the interface
    friend void InterfaceManager::deleteInterface<iface_sermem>(iface_sermem*);
  };

} // namespace caribou

#endif /* CARIBOU_HAL_SERIALMEMORY_HPP */
