#ifndef CARIBOU_HAL_INTERFACE_HPP
#define CARIBOU_HAL_INTERFACE_HPP

#define FRIEND(FN) friend class FN;
#define TEMPLATEFRIEND(FN) template <typename B, typename SCI, typename DRI> friend class FN;

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "interfaceConfiguration.hpp"
#include "utils/configuration.hpp"
#include "utils/exceptions.hpp"
#include "utils/log.hpp"
#include "utils/utils.hpp"

// forward declaration required by the GENERATE_FRIENDS macro
namespace caribou {
  template <typename B, typename SCI, typename DRI> class CaribouDevice;
  class caribouHAL;
  namespace carboard {
    class Carboard;
  }
  namespace falconboard {
    class Falconboard;
  }
} // namespace caribou
#define GENERATE_FRIENDS()                                                                                                  \
  TEMPLATEFRIEND(caribou::CaribouDevice)                                                                                    \
  FRIEND(caribou::caribouHAL) FRIEND(caribou::carboard::Carboard) FRIEND(caribou::falconboard::Falconboard)

namespace caribou {

  // Abstract class for all interfaces
  //@param ADDRESS_T : type for a device address
  //@param REG_T : type for register addresses
  //@param DATA_T : type for data
  template <typename REG_T = uint8_t, typename DATA_T = REG_T, typename CONFIG_T = InterfaceConfiguration> class Interface {
  public:
    using reg_type = REG_T;
    using data_type = DATA_T;
    using configuration_type = CONFIG_T;
    using dataVector_type = std::vector<data_type>;

    std::string devicePath() const { return _devicePath; }

  private:
    // Path of the device
    const std::string _devicePath;

  protected:
    explicit Interface(const caribou::InterfaceConfiguration& config) : _devicePath(config._devpath){};
    virtual ~Interface() = default;

    //////////////////////
    // Write operations
    //////////////////////

    // Write data to a device which does not contain internal register
    // If readout is intergralpart of write operations, the read values a returned by function.
    virtual DATA_T write(const DATA_T&) { throw CommunicationError("Functionality not provided by this interface"); };

    // Write data to a device which does not contain internal register
    // If readout is intergralpart of write operations, the read values a returned by function.
    virtual dataVector_type write(const dataVector_type&) {
      throw CommunicationError("Functionality not provided by this interface");
    };

    // Write data to a device containing internal registers
    // If readout is intergralpart of write operations, the read values a returned by function.
    virtual std::pair<REG_T, DATA_T> write(const std::pair<REG_T, DATA_T>&) {
      throw CommunicationError("Functionality not provided by this interface");
    };

    // Write data to a device containing internal registers
    // If readout is intergralpart of write operations, the read values a returned by function.
    virtual dataVector_type write(const REG_T&, const dataVector_type&) {
      throw CommunicationError("Functionality not provided by this interface");
    };

    // Write data to a device containing internal registers
    // If readout is intergralpart of write operations, the read values a returned by function.
    virtual std::vector<std::pair<REG_T, DATA_T>> write(const std::vector<std::pair<REG_T, DATA_T>>&) {
      throw CommunicationError("Functionality not provided by this interface");
    };

    //////////////////////
    // Read operations
    //////////////////////

    // Read single data word form the given device
    virtual DATA_T read() { throw CommunicationError("Functionality not provided by this interface"); };

    // Read number of data words form the given device
    virtual dataVector_type read(const unsigned int) {
      throw CommunicationError("Functionality not provided by this interface");
    };

    // Read number of data words form a register of the given device
    virtual dataVector_type read(const REG_T&, const unsigned int) {
      throw CommunicationError("Functionality not provided by this interface");
    };
  };

} // namespace caribou
#endif
