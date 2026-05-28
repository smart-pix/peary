#ifndef CARIBOU_HAL_LOOPBACK_HPP
#define CARIBOU_HAL_LOOPBACK_HPP

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "interfaces/Interface.hpp"
#include "interfaces/InterfaceManager.hpp"
#include "utils/exceptions.hpp"

namespace caribou {

  class iface_loopback_config : public InterfaceConfiguration {
  public:
    iface_loopback_config(std::string const& devpath, const uint8_t devaddress);

    uint8_t _devaddress;

    virtual bool operator<(const iface_loopback_config& rhs) const;
  };

  class iface_loopback : public Interface<uint8_t, uint8_t, iface_loopback_config> {

  private:
    /* Default constructor: private (only created by InterfaceManager)
     *
     * Throws caribou::DeviceException in case no connection can be established
     */
    explicit iface_loopback(const configuration_type& config);

    ~iface_loopback() override;

    // Protects access to the bus
    std::mutex mutex;

    GENERATE_FRIENDS()

  public:
    uint8_t devAddress() const { return _devAddress; }

    // Unused constructor
    iface_loopback() = delete;

  private:
    const uint8_t _devAddress;

    uint8_t write(const uint8_t& data) override;
    dataVector_type write(const dataVector_type& data) override;
    std::pair<uint8_t, uint8_t> write(const std::pair<uint8_t, uint8_t>& data) override;
    dataVector_type write(const uint8_t& reg, const dataVector_type& data) override;
    std::vector<std::pair<uint8_t, uint8_t>> write(const std::vector<std::pair<uint8_t, uint8_t>>& data) override;
    dataVector_type read(const unsigned int length) override;
    dataVector_type read(const uint8_t& reg, const unsigned int length) override;

    // only this function can create the interface
    friend iface_loopback& InterfaceManager::getInterface<iface_loopback>(const configuration_type&);

    // only this function can delete the interface
    friend void InterfaceManager::deleteInterface<iface_loopback>(iface_loopback*);
  };

} // namespace caribou

#endif /* CARIBOU_HAL_LOOPBACK_HPP */
