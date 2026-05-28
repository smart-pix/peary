#ifndef CARIBOU_HAL_SPI_BUS_H
#define CARIBOU_HAL_SPI_BUS_H

#include "interfaces/SPI/spi.hpp"

namespace caribou {

  /* SPI bus interface class
   * In this implementation 2 type of the SPI frames are considered: read and write frame.
   * The type of the frame is distinguished by the most significant bit of the SPI transaction on MOSI lines.
   * The polairity of this bit is distinguished by write strobe (ws) parameter of the class constructor.
   * The frame type bit in the SPI frame is followed then by address of the register to be accessed and data.
   * The maximum width of SPI frame suppoprted by the class is limited by the width of uintmax_t type.
   *
   * E.g. addressBits = 8, dataBits = 8, ws = 1
   *
   * Read operation:
   *
   *         1b    8bit     8bit
   * MOSI | !ws |  ADDR  |    -  |
   * MISO | -   |    -   |  DATA |
   *
   * Write operation:
   *
   *         1b    8bit     8bit
   * MOSI | ws  |  ADDR  |  DATA  |
   * MISO |   - |    -   |   -    |
   *
   * As most of the SPI drivers support properly on 8 bits_per_word size, the actual SPI transaction has length
   * which is multiple of 8-bits. alignMSB defines whether the read/write operatio is aligned to MSB or LSB.
   * for the previous example, alignMSB=1 would correspond to the actual SPI transaction:
   *
   * Read/write operation:
   *         1b    8bit     8bit        7bit
   * MOSI | ws  |  ADDR  |  (DATA)  | PADDING
   * MISO |   - |    -   |  (DATA)  | PADDING
   *
   * While alignMSB=0 would produce:
   *
   * Read/write operation:
   *         7b      1b    8bit      8bit
   * MOSI PADDING | ws  |  ADDR  |  (DATA)  |
   * MISO PADDING |   - |    -   |  (DATA)  |
   *
   */

  class iface_spi_bus_config : public InterfaceConfiguration {
  public:
    iface_spi_bus_config(
      std::string const& devpath, const uint8_t addressBits, const uint8_t dataBits, const bool ws, const bool alignMSB);

    uint8_t _addressBits;
    uint8_t _dataBits;
    const bool _ws;
    const bool _alignMSB;

    virtual bool operator<(const iface_spi_bus_config& rhs) const;
  };

  class iface_spi_bus : public iface_spi<> {

  public:
    // redefine the configration object type
    typedef iface_spi_bus_config configuration_type;

  protected:
    const uint8_t _addressBits;
    const uint8_t _dataBits;

    // Masks constants used by the class
    const spi_reg_t _addressMask;

    const spi_t _dataMask;

    // Minimum length of the actual SPI frame aligned to 8bits, which can accomodate data
    const size_t _length;

    const bool _ws;

    const bool _alignMSB;

    // Default constructor: private (only created by InterfaceManager)
    // It can throw DeviceException
    explicit iface_spi_bus(const configuration_type& config);

    ~iface_spi_bus() override = default;

    GENERATE_FRIENDS()

  public:
    std::pair<spi_reg_t, spi_t> write(const std::pair<spi_reg_t, spi_t>& data) override;
    std::vector<std::pair<spi_reg_t, spi_t>> write(const std::vector<std::pair<spi_reg_t, spi_t>>& data) override;
    dataVector_type read(const spi_reg_t& reg, const unsigned int length) override;

    // SPI access
    // rw = 0 read access
    // rw = 1 write acceess
    std::pair<spi_reg_t, spi_t> access(const bool rw, const std::pair<spi_reg_t, spi_t>& data);

    // Unused constructor
    iface_spi_bus() = delete;

    // only this function can create the interface
    friend iface_spi_bus& InterfaceManager::getInterface<iface_spi_bus>(const configuration_type&);

    // only this function can delete the interface
    friend void InterfaceManager::deleteInterface<iface_spi_bus>(iface_spi_bus*);

  }; // class iface_spi_bus

} // namespace caribou

#endif
