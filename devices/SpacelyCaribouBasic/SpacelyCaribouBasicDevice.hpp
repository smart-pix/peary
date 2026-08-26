/**
 * Header file for the Spacely-Caribou Basic C++ Caribou Device
 */

#ifndef DEVICE_SPACELYCARIBOUBASIC_H
#define DEVICE_SPACELYCARIBOUBASIC_H

#include "device/CaribouDevice.hpp"
#include "hardware_abstraction/carboard/Carboard.hpp"

#include "Si5345-RevD-SP3A-Registers.h"
#include "Si5345-RevD-SP3A-Registers-1.h"
#include "Si5345-RevD-SP3A-Registers-2.h"

#include "SpacelyCaribouBasicDefaults.hpp"

#include <fstream>

//using data_type = uintptr_t;
//using dataVector_type = std::vector<data_type>;


namespace caribou {

  /** Spacely Caribou Basic Device class definition
   */
  class SpacelyCaribouBasicDevice : public CaribouDevice<carboard::Carboard, iface_mem> {

  public:
    SpacelyCaribouBasicDevice(const caribou::Configuration config);
    ~SpacelyCaribouBasicDevice();

    void daqStart() override;
    void daqStop() override;

    void powerUp() override;
    void powerDown() override;
    void setUsrclkFreq(const uint64_t freq);

    void car_i2c_write(const uint32_t bus, const uint32_t component_addr, const uint32_t mem_addr, const uint32_t data);

    uint16_t car_i2c_read(const uint32_t bus, const uint32_t component_addr, const uint32_t mem_addr, const uint32_t len);

    void checkSI5345Locked();
    void configureSI5345(int config_num);
    void streamMemoryToFile(const std::string& name, const unsigned int N);
    std::string burstReadDataArray1(const unsigned int opcode, const unsigned int base_addr, const unsigned int N);
    void burstWriteSw0(const std::string& values_csv);
    std::string burstPollStatusDone(const unsigned int bit_index, const unsigned int timeout_us);
    void disableSI5345();

    void setInputCMOSLevel(double voltage);
    void setOutputCMOSLevel(double voltage);

  };


} // namespace caribou

#endif /* DEVICE_ZCU102_LED_DEMO_H */
