/**
 * Caribou implementation for the dSiPM chip
 */

#ifndef DEVICE_DSIPM_H
#define DEVICE_DSIPM_H

#include "device/CaribouDevice.hpp"
#include "hardware_abstraction/carboard/Carboard.hpp"

#include "clockgenerator/Si5345-RevB-dSiPM-Registers.h"
#include "dSiPMDefaults.hpp"
#include "dSiPMFrameDecoder.hpp"
#include "dSiPMPixels.hpp"

#include <csignal>
#include <fstream>
#include <time.h>

namespace caribou {

  /** dSiPM Device class definition
   */
  class dSiPMDevice : public CaribouDevice<carboard::Carboard, iface_mem> {

  public:
    dSiPMDevice(const caribou::Configuration config);
    ~dSiPMDevice();

    /** Initializer function for dSiPM
     */
    void configure() override;

    /** Configure and disable clock
     */
    void configureClock(bool internal = 1);
    void disableClock();

    /** Start and stop daq
     */
    void daqStart() override;
    void daqStop() override;

    /** Switch power on and off, list power status
     */
    void powerUp() override;
    void powerDown() override;
    void ledOn();
    void ledOff();
    void tOn();
    void tOff();
    void powerStatusLog();

    /** Configure pixel matrix from mask file
     */
    void configureMatrix(std::string filename);

    /** Special registers are accessible like normal registers,
     *  but allow to implement aditional functionality.
     */
    void setSpecialRegister(const std::string& name, uintptr_t value) override;
    uintptr_t getSpecialRegister(const std::string& name) override;

    /* Returns data as retrieved from chip
     */
    pearyRawData getRawData() override;
    /* Returns data, striped from timestamp
     */
    pearyRawData getDeviceData();
    /* Retruns data, converted into peary data, using frame decoder
     */
    pearydata getData() override;
    pearydata getFakeData();
    /* Prints N frames of data to ascii
     */
    void printFrames(unsigned int N = 1);
    void printLEDFrames(unsigned int N = 1);
    void storeFrames(unsigned int N = 1, std::string filename = dSiPM_BINframeFilename);
    void storeLEDFrames(unsigned int N = 1, std::string filename = dSiPM_BINframeFilename);

    /* Prints N temperature readings to ascii.
     * N < 0 prints untill 'enter' pressed.
     */
    void printTemperatures(int N = 1);

  private:
    /* Configure the pixel matrix (masking)
     */
    void programMask(size_t quadEnable = 0x0f);

    void writeConfig();

    /* Store pixel mask values as register mask type for each row.
     * This conveniently fits the way the registers are defined on
     * the chip.
     */
    using rowMasks_t = std::map<size_t, size_t>;
    rowMasks_t rowMasks_{};
    rowMasks_t readMask(std::string filename);
    size_t storeQuadEnable_ = 0x0F;

    /* Retrieves data from chip
     */
    pearyRawData getFrame();
    double getTemperature();

    /* Print some temperature measurements for easy assesment
     */
    std::ofstream m_outfileTemperatures;

    /* Handle SIGINT for printTemperatures
     */
    static void signalHandlerForPrint(int signum);
    static sig_atomic_t endPrint_;

    /* Class to handle conversion of pearyrawdata to pearydata
     */
    dSiPMFrameDecoder frame_decoder_;

    /* Chip parameters
     */
    const size_t totalNRows_ = 32;
    const size_t totalNCols_ = 32;
  };

} // namespace caribou

#endif /* DEVICE_DSIPM_H */
