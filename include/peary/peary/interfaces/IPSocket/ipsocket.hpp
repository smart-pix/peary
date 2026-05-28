#ifndef CARIBOU_HAL_IPSOCKET_HPP
#define CARIBOU_HAL_IPSOCKET_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "interfaces/Interface.hpp"
#include "interfaces/InterfaceManager.hpp"
#include "utils/exceptions.hpp"

namespace caribou {

  using ipsocket_port_t = uint32_t;
  using ipsocket_t = std::string;

  class iface_ipsocket : public Interface<ipsocket_t, ipsocket_t> {

  private:
    // Default constructor: private (only created by InterfaceManager)
    //
    // It can throw DeviceException
    explicit iface_ipsocket(const configuration_type& config);

    ~iface_ipsocket() override;

    GENERATE_FRIENDS()

  public:
    ipsocket_t write(const ipsocket_t& payload) override;

    dataVector_type read(const ipsocket_t& query, const unsigned int length) override;
    std::vector<uint8_t> read_binary(const ipsocket_t& query, const unsigned int length);

    // Unused constructor
    iface_ipsocket() = delete;

  private:
    // Split the devicePath into port and IP address:
    std::pair<std::string, uint16_t> split_ip_address(const std::string& address);

    // Called from read() and write() while holding the lock
    ipsocket_t write_nolock(const ipsocket_t& payload);

    std::string trim(const std::string& str, const std::string& delims = " \t\n\r\v");
    std::string cleanCommandString(std::string& command);

    // Remote socket to connect to
    int mysocket_;

    // only this function can create the interface
    friend iface_ipsocket& InterfaceManager::getInterface<iface_ipsocket>(const configuration_type&);

    // only this function can delete the interface
    friend void InterfaceManager::deleteInterface<iface_ipsocket>(iface_ipsocket*);
  };

} // namespace caribou

#endif /* CARIBOU_HAL_IPSOCKET_HPP */
