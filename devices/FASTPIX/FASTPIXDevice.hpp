/**
 * Caribou implementation for the FASTPIX chip
 */

#ifndef DEVICE_FASTPIX_H
#define DEVICE_FASTPIX_H

#include "device/CaribouDevice.hpp"
#include "hardware_abstraction/carboard/Carboard.hpp"

#include "FASTPIXDefaults.hpp"

namespace caribou {

  class DummyInterface : public Interface<> {
  private:
    DummyInterface(const configuration_type& config) : Interface(config){};
    GENERATE_FRIENDS()
    friend DummyInterface& InterfaceManager::getInterface<DummyInterface>(const configuration_type&);
  };

  /** FASTPIX Device class definition
   */
  class FASTPIXDevice : public CaribouDevice<carboard::Carboard, DummyInterface> {

  public:
    FASTPIXDevice(const caribou::Configuration config);
    ~FASTPIXDevice();

    /** Initializer function for FASTPIX
     */
    void configure();

    /** Turn on the power supply for the FASTPIX chip
     */
    void powerUp();

    /** Turn off the FASTPIX power
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

    void setMB(uint32_t mb1, uint32_t mb2);
    void setPRE(uint32_t pre);
    void setHBRIDGE(uint32_t h);
    void setPB(uint32_t pb);
    void setCMFB(uint32_t cmfb);

    void setVDD(double vdd);

  private:
  };

} // namespace caribou

#endif /* DEVICE_FASTPIX_H */
