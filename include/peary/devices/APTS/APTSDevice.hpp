/**
 * Caribou implementation for the APTS chip
 */

#ifndef DEVICE_APTS_H
#define DEVICE_APTS_H

#include "device/CaribouDevice.hpp"
#include "hardware_abstraction/carboard/Carboard.hpp"

#include "APTSDefaults.hpp"

namespace caribou {

  class DummyInterface : public Interface<> {
  private:
    DummyInterface(const configuration_type& config) : Interface(config){};
    GENERATE_FRIENDS()
    friend DummyInterface& InterfaceManager::getInterface<DummyInterface>(const configuration_type&);
  };

  /** APTS Device class definition
   */
  class APTSDevice : public CaribouDevice<carboard::Carboard, DummyInterface> {

  public:
    APTSDevice(const caribou::Configuration config);
    ~APTSDevice();

    /** Initializer function for APTS
     */
    void configure() override;

    /** Turn on the power supply for the APTS chip
     */
    void powerUp() override;

    /** Turn off the APTS power
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

    // Reset the chip
    void reset() override;

    void pulse();

  private:
  };

} // namespace caribou

#endif /* DEVICE_APTS_H */
