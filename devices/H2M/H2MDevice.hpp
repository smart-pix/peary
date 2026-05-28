/**
 * Caribou implementation for the H2M chip
 */

#pragma once

#include <fstream>
#include <vector>

#include "device/CaribouDevice.hpp"
#include "hardware_abstraction/carboard/Carboard.hpp"
#include "interfaces/SerialMemory/serialmemory.hpp"
#include "utils/datatypes.hpp"
#include "utils/log.hpp"

#include "H2MDefaults.hpp"
#include "H2MFrameDecoder.hpp"
#include "clockgenerator/Si5345-RevB-H2M-Registers.h"
#include "h2m_pixels.hpp"

namespace caribou {

  /** H2M Device class definition
   */
  class H2MDevice : public CaribouDevice<carboard::Carboard, iface_sermem> {

  public:
    H2MDevice(const caribou::Configuration config);
    ~H2MDevice();

    /** Initializer function for H2M
     */
    void configure() override;

    /** Configure and disable clock
     */
    void configureClock(bool internal = 1);
    void disableClock();

    void startPatternGenerator(uint32_t runs);
    void stopPatternGenerator();
    void configurePatternGenerator(std::string filename);

    void triggerPatternGenerator(bool sleep);

    void setSpecialRegister(const std::string& name, uintptr_t value) override;

    /** Reset required after power on
     */
    void chipReset();

    /** Turn on the power supply for the H2M chip
     */
    void powerUp() override;

    /** Turn off the H2M power
     */
    void powerDown() override;

    /** Start the data acquisition
     */
    void daqStart() override;

    /** Stop the data acquisition
     */
    void daqStop() override;

    /** Report power status
     */
    void powerStatusLog();

    /** Report front-end operation parameters
     */
    void logStatusFE();

    /** Configure test-pulse injection
     */
    void configureTP();

    /** generate a mask with the same configuration for every pixel
     */
    void generateMask_human(bool, bool, uint16_t, bool);

    /** Configure pixel matrix from mask file
     */
    void configureMatrix(std::string filename = "");

    pearydata getData() override;
    pearyRawData getRawData() override;
    pearyRawData getRawDataSC();

    void printFrames(unsigned int N = 1, std::string filename = "frame_filename.txt");
    void testpulse(unsigned int N = 1, bool write = 0);

    /** perform a parameter scan in scan_dac, starting at scan until scan_end with step size scan_step
     */
    void parameterScan(std::string, int, int, int, unsigned int, std::string);
    void takeFrames(unsigned int N = 1);

    // read one data event in the automated readout
    inline pearyRawData read_fifo_data(unsigned int wordCount = 262);
    void print_fifo_data();
    /* Class to handle conversion of pearyrawdata to pearydata
     */
    H2MFrameDecoder frame_decoder_;

  private:
    /** H2M readout configuration: force read from fifo
     */
    bool force_read_from_fifo_;

    /** Write matrixMask_ to registers
     */
    void programMatrix();

    /** generate a mask with the same configuration for every pixel
     */
    void generateMask(uint16_t);

    /** Pearydata is exactly the container we want for pixel configuration data
     */
    pearydata matrixMask_;

    /** Reading the mask file into matrixMask_
     */
    void readMask(std::string filename);

    /** For handling of occupancy measurements
     */
    std::vector<std::vector<uint16_t>> matrixOccupancy_;

    /** Handling of matrixOccupancy_ container
     */
    void occupancy_initialize();
    void occupancy_add(const pearydata& frame);
    void occupancy_print(std::string, std::string, int, int, int, unsigned int);
  };

} // namespace caribou
