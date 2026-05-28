/**
 * Caribou implementation for the CoRDIA_02 chip
 */

#ifndef DEVICE_CORDIA_H
#define DEVICE_CORDIA_H

#include "device/CaribouDevice.hpp"
#include "hardware_abstraction/carboard/Carboard.hpp"

#include "CoRDIADefaults.hpp"

#include <fstream>

namespace caribou {

  /** CoRDIA Device class definition
   */
  class CoRDIADevice : public CaribouDevice<carboard::Carboard, iface_mem> {

  public:
    CoRDIADevice(const caribou::Configuration config);
    ~CoRDIADevice();

    /** Initializer function for CoRDIA
     */
    void configure() override;

    /** Start and stop daq
     */
    void daqStart() override;
    void daqStop() override;

    /** Switch power on and off, list power status
     */
    void powerUp() override;
    void powerDown() override;
    void powerStatusLog();

    /** Special registers are accessible like normal registers,
     *  but allow to implement aditional functionality.
     */
    void setSpecialRegister(const std::string& name, uintptr_t value) override;
    uintptr_t getSpecialRegister(const std::string& name) override;

    /* Returns data as retrieved from chip
     */
    pearyRawData getRawData() override;
    /* Retruns data, converted into peary data, using frame decoder
     */
    pearydata getData() override;

    /* Reads pattern generator configuration from a file and loads it into FPGA BRAM.
     */
    void configurePatternGenerator(std::string filename);
    void startPatternGenerator(uint32_t runs);
    void stopPatternGenerator();

    void setFreq(const uint64_t freq);

    void storeFrames(uint32_t N, std::string filename);
    void printFrames(uint32_t N);

  private:
    /* Writes configuration bits to the chip
     */
    void writeConfig();

    /* Retrieves data from chip
     */
    pearyRawData getFrame();

    /* Class to handle conversion of pearyrawdata to pearydata
     */
    // dSiPMFrameDecoder frame_decoder_;

    /* Chip parameters
     */
    // const size_t totalNRows_ = 32;
    // const size_t totalNCols_ = 32;
  };

} // namespace caribou

#endif /* DEVICE_CORDIA_H */
