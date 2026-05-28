/**
 * Caribou Device implementation for CLICpix2
 */

#ifndef DEVICE_CLICPIX2_H
#define DEVICE_CLICPIX2_H

#include <string>
#include <vector>
#include "device/CaribouDevice.hpp"
#include "hardware_abstraction/carboard/Carboard.hpp"
#include "interfaces/SPI_CLICpix2/spi_CLICpix2.hpp"
#include "utils/configuration.hpp"

#include "clicpix2_defaults.hpp"
#include "clicpix2_pixels.hpp"
#include "clockgenerator/Si5345-RevB-CLICpix2-Registers.h"
#include "clockgenerator/Si5345-RevB-CLICpix2-Registers_freeRunningMode.h"
#include "framedecoder/clicpix2_frameDecoder.hpp"

namespace caribou {

  class pixelConfig;

  /** CLICpix2 Device class definition
   *
   *  this class implements the required functionality to operate CLICpix2 chips via the
   *  Caribou device class interface.
   */
  class CLICpix2Device : public CaribouDevice<carboard::Carboard, iface_spi_CLICpix2> {

  public:
    CLICpix2Device(const caribou::Configuration config);
    ~CLICpix2Device();

    /** Initializer function for CLICpix2
     */
    void configure() override;

    /** Turn on the power supply for the CLICpix2 chip
     */
    void powerUp() override;

    /** Turn off the CLICpix2 power
     */
    void powerDown() override;

    /** Start the data acquisition
     */
    void daqStart() override;

    /** Stop the data acquisition
     */
    void daqStop() override{};

    /** Report power status
     */
    void powerStatusLog();

    void exploreInterface();

    void configureMatrix(std::string filename = std::string());
    void configurePatternGenerator(std::string filename);
    void triggerPatternGenerator(bool sleep);
    void clearTimestamps();

    // The functions sets clocks required by CLICpix2 to operate
    void configureClock(bool internal);

    void getMem(std::string);
    void setMem(std::string, uint32_t value);

    /**
     * Reading raw data from CLICpix2. This function returns both the pixel data frame and the frame timestamps from the
     * pattern generator. The first word contains the number of timestamp words to follow. Then, the timestamps are written,
     * followed by the frame data from the device.
     * @warning This function automatically triggers the Pattern Generator!
     * @return Block of raw data as described above
     */
    pearyRawData getRawData() override;

    /**
     * Reading one decoded data frame from CLICpix2. This function returns a fully decoded data frame
     * @warning This function does NOT trigger the Pattern Generator! It needs to be done manually before calling getData()
     * @return Pearydata with pixel hits from one frame
     */
    pearydata getData() override;

    void setSpecialRegister(const std::string& name, uintptr_t value) override;

    // Reset the chip
    // The reset signal is asserted for ~1us
    void reset() override;

    /**
     * @brief Set correct channel for output multiplexer
     */
    void setOutputMultiplexer(std::string name);

  private:
    /* Routine to program the pixel matrix via the SPI interface
     *
     * This routine produces a bit matrix (using STL vector<bool>) which can
     * directly be sent to the ASIC via the SPI interface in blocks of 8bit words.
     * Interleaved flipflops for superpixels and column-end interfaces are
     * accounted for.
     * The endianness of the SPI interface is obeyed when sending the data and the
     * columns are flipped accordingly.
     */
    void programMatrix();

    /**
     * Method to return timestamps from the device pattern generator
     * The timestamps are 64bit words, split into MSB and LSB part. The MSB part is shipped **first**.
     * @return List of timestamp words.
     */
    std::vector<uint32_t> getTimestamps();

    /* Map of pixelConfigs for configuration storage (row/col)
     */
    std::map<std::pair<uint8_t, uint8_t>, pixelConfig> pixelsConfig{};

    // Retrieve frame from device
    std::vector<uint32_t> getFrame();

    // Methods decodes frame
    pearydata decodeFrame(const std::vector<uint32_t>& frame);

    // Total pattern generator length
    uint32_t pg_total_length;
  };

} // namespace caribou

#endif /* DEVICE_CLICPIX2_H */
