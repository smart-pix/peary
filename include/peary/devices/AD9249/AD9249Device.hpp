/** Example caribou device
 *
 * Use this class as a starting point for a device that does not rely on
 * an underlying hardware abstraction layer.
 */

#ifndef EXAMPLE_AD9249Device_H
#define EXAMPLE_AD9249Device_H

#include <fstream>
#include <thread>

#include "AD9249_defaults.hpp"
#include "device/CaribouDevice.hpp"
#include "hardware_abstraction/carboard/Carboard.hpp"
#include "interfaces/SPI/spi.hpp"
#include "utils/configuration.hpp"

#include "clockgenerator/Si5345-RevB-APTS-Registers.h"

namespace caribou {
  class AD9249Device : public CaribouDevice<carboard::Carboard, iface_spi<uint16_t>> {
  public:
    AD9249Device(caribou::Configuration config);
    ~AD9249Device();

    std::string getFirmwareVersion() override;
    std::string getType() override;
    std::string getName() override { return getType(); }

    // Controll the device
    void reset() override{};
    void powerUp() override{};
    void powerDown() override{};
    /** Start the data acquisition
     */
    void daqStart() override;

    /** Stop the data acquisition
     */
    void daqStop() override;

    // Start and stop DAQ writing data to file
    void fileDaqStart();
    void fileDaqStop();

    // Retrieve data
    pearyRawData getRawData() override;
    pearydata getData() override;

    // Look into data
    void printFrame();
    void printFrameFile(std::string filename);
    void decodeChannel(const size_t adc,
                       const std::vector<uint8_t>& data,
                       size_t size,
                       size_t offset,
                       std::vector<std::vector<uint16_t>>& waveforms,
                       uint64_t& timestamp);

    void configureClock(bool internal);
    void configure() override;
    void resetAdc();
    void trigger(uintptr_t t = 1);
    void dump();
    void setDelay(int ch, int delay);
    void setDelay2(int ch, int delay);
    void calibDelay();
    void setPattern(uint16_t v);
    void setBurst(uintptr_t len);
    void setThreshold(uintptr_t low, uintptr_t high);
    void setDataDelay(uintptr_t delay);
    void enableTrigger(uintptr_t ch0, uintptr_t ch1);
    void setTriggerMask(uintptr_t ch0, uintptr_t ch1);
    void resetCounters();
    void testPattern();

  private:
    pearyRawData readChannel(uintptr_t ch, uintptr_t& prev_addr, uintptr_t curr_addr, uint16_t* base);

    int _memfd;
    uint16_t *adc0_readout, *adc1_readout;
    std::ofstream data_out;

    uintptr_t burst_length = 1;
    uint32_t poll_interval;
    uintptr_t trigger_adc0, trigger_adc1;

    std::atomic_flag _daqContinue{};
    std::thread _daqThread;

    bool daqRunning = false;

    uintptr_t prev_addr_ch0;
    uintptr_t prev_addr_ch1;
    uint64_t trigger_;

    // Print some frames for easy assesment
    std::ofstream m_outfileFrames;

    void runDaq();
  };

} // namespace caribou

#endif /* EXAMPLE_AD9249_H */
