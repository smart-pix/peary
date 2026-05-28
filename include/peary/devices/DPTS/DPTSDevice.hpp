/**
 * Caribou implementation for the DPTS chip
 */

#ifndef DEVICE_DPTS_H
#define DEVICE_DPTS_H

#include "device/CaribouDevice.hpp"
#include "hardware_abstraction/carboard/Carboard.hpp"

#include "DPTSDefaults.hpp"

namespace caribou {

  class DummyInterface : public Interface<> {
  private:
    DummyInterface(const configuration_type& config) : Interface(config){};
    GENERATE_FRIENDS()
    friend DummyInterface& InterfaceManager::getInterface<DummyInterface>(const configuration_type&);
  };

  /** DPTS Device class definition
   */
  class DPTSDevice : public CaribouDevice<carboard::Carboard, DummyInterface> {

  public:
    DPTSDevice(const caribou::Configuration config);
    ~DPTSDevice();

    /** Initializer function for DPTS
     */
    void configure();

    /** Turn on the power supply for the DPTS chip
     */
    void powerUp();

    /** Turn off the DPTS power
     */
    void powerDown();

    /** Start the data acquisition
     */
    void daqStart();

    /** Stop the data acquisition
     */
    void daqStop();

    /** Report power status
     */
    void powerStatusLog();

    // Reset the chip
    void reset();

    void setVDD(double vdd);
    void writeSR(uint32_t mr, uint32_t cs, uint32_t md, uint32_t mc, uint32_t rs);
    void enablePulser(uint32_t x, uint32_t y);
    void writeSR2(uint32_t mr1,
                  uint32_t mr2,
                  uint32_t mr3,
                  uint32_t cs1,
                  uint32_t cs2,
                  uint32_t cs3,
                  uint32_t md1,
                  uint32_t md2,
                  uint32_t md3,
                  uint32_t mc1,
                  uint32_t mc2,
                  uint32_t mc3,
                  uint32_t rs1,
                  uint32_t rs2,
                  uint32_t rs3);

    void clearConfig();
    void writeConfig();
    void maskColumn(int i);
    void maskRow(int i);
    void maskDiagonal(int i);
    void maskPixel(int x, int y);

    void pulse();
    void toggle(int i);
    void set(int i);
    void clear(int i);

  private:
    void writeReg(uint32_t r, bool reverse = false);
    void writeReg2(uint32_t r);
    void writeBit(uint8_t b);

    uint32_t column_mask = 0;
    uint32_t row_mask = 0;
    uint32_t diagonal_mask = 0;
    uint32_t column_pulser = 0;
    uint32_t row_pulser = 0;
  };

} // namespace caribou

#endif /* DEVICE_DPTS_H */
