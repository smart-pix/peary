/**
 * Header file for the ZCU102_LED_Demo C++ Caribou Device
 */

#ifndef DEVICE_ZCU102_LED_DEMO_H
#define DEVICE_ZCU102_LED_DEMO_H

#include "device/CaribouDevice.hpp"
#include "hardware_abstraction/carboard/Carboard.hpp"

#include "ZCU102_LED_DemoDefaults.hpp"

#include <fstream>

namespace caribou {

  /** SParkDream Device class definition
   */
  class ZCU102_LED_DemoDevice : public CaribouDevice<carboard::Carboard, iface_mem> {

  public:
    ZCU102_LED_DemoDevice(const caribou::Configuration config);
    ~ZCU102_LED_DemoDevice();

    void daqStart() override;
    void daqStop() override;

    void powerUp() override;
    void powerDown() override;
    void setUsrclkFreq(const uint64_t freq);

  };


} // namespace caribou

#endif /* DEVICE_ZCU102_LED_DEMO_H */
