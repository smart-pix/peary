#ifndef CARIBOU_HAL_SPI_CLICPIX2_H
#define CARIBOU_HAL_SPI_CLICPIX2_H

#include "interfaces/SPI/spi.hpp"

namespace caribou {

  /* SPI command interface class
   */
  class iface_spi_CLICpix2 : public iface_spi<> {

  protected:
    // Default constructor: private (only created by InterfaceManager)
    // It can throw DeviceException
    explicit iface_spi_CLICpix2(const Interface<>::configuration_type& config) : iface_spi<>(config){};

    ~iface_spi_CLICpix2() override = default;

    GENERATE_FRIENDS()

    std::pair<iface_spi<>::spi_reg_t, spi_t> write(const std::pair<iface_spi<>::spi_reg_t, spi_t>& data) override;
    std::vector<std::pair<iface_spi<>::spi_reg_t, spi_t>>
    write(const std::vector<std::pair<iface_spi<>::spi_reg_t, spi_t>>& data) override;

    // only this function can create the interface
    friend iface_spi_CLICpix2&
    InterfaceManager::getInterface<iface_spi_CLICpix2>(const Interface<>::configuration_type& config);

    // only this function can delete the interface
    friend void InterfaceManager::deleteInterface<iface_spi_CLICpix2>(iface_spi_CLICpix2*);

  public:
    // Unused constructor
    iface_spi_CLICpix2() = delete;

  }; // class iface_spi_CLICpix2

} // namespace caribou

#endif /* CARIBOU_HAL_SPI_CLICPIX2_H */
