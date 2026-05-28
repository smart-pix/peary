#ifndef CARIBOU_HAL_SPI_H
#define CARIBOU_HAL_SPI_H

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "interfaces/Interface.hpp"
#include "interfaces/InterfaceManager.hpp"
#include "utils/exceptions.hpp"

namespace caribou {

  using spi_t = uint8_t;

  /* SPI command interface class
   * Template parameter corresponds to the register address width
   */
  template <typename REG_T = uint8_t> class iface_spi : public Interface<REG_T, spi_t> {
  public:
    using spi_reg_t = REG_T;

  protected:
    // Default constructor: private (only created by InterfaceManager)
    // It can throw DeviceException
    explicit iface_spi(const Interface<>::configuration_type& config);

    ~iface_spi() override;

    // Descriptor of the device
    int spiDesc;

    // Protects access to the bus
    std::mutex mutex;

    // SPI bits per word
    const uint32_t bits_per_word = 8;

    GENERATE_FRIENDS()

    std::pair<REG_T, spi_t> write(const std::pair<REG_T, spi_t>& data) override;
    Interface<>::dataVector_type write(const REG_T& reg, const Interface<>::dataVector_type& data) override;
    std::vector<std::pair<REG_T, spi_t>> write(const std::vector<std::pair<REG_T, spi_t>>& data) override;
    Interface<>::dataVector_type read(const REG_T& reg, const unsigned int length) override;

    // only this function can create the interface
    friend iface_spi& InterfaceManager::getInterface<iface_spi<REG_T>>(const Interface<>::configuration_type&);

    // only this function can delete the interface
    friend void InterfaceManager::deleteInterface<iface_spi<REG_T>>(iface_spi<REG_T>*);

  public:
    // Unused constructor
    iface_spi() = delete;

  }; // class iface_spi

} // namespace caribou

#ifdef SPI_EMULATED
#include "emulator.tcc"
#else
#include "spi.tcc"
#endif

#endif /* CARIBOU_HAL_SPI_H */
