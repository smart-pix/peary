#ifndef CARIBOU_FALCONBOARD_MAPS_H
#define CARIBOU_FALCONBOARD_MAPS_H

#include "utils/datatypes.hpp"

namespace caribou {
  namespace falconboard {

    // Caribou control addresses
    const std::uintptr_t CARIBOU_CONTROL_BASE_ADDRESS = 0xA0002000;
    const std::uintptr_t CARIBOU_FIRMWARE_VERSION_OFFSET = 0;
    const std::size_t CARIBOU_CONTROL_MAP_SIZE = 4096;

    // Falcon control address
    const std::uintptr_t FALCON_CONTROL_BASE_ADDRESS = 0xA0001000;
    const std::uintptr_t FALCON_LASER_HV_ENABLE_OFFSET = 0x0e << 3;
    const std::size_t FALCON_CONTROL_MAP_SIZE = 4096;

    const uint64_t FALCON_LASER_HV_ENABLE_MASK = 0x1;

    /** IIO Devices
     */
    // DAC LTC2634
    const uint8_t ADDR_U7_U_ASIC_DAC0 = 3;
    const uint8_t ADDR_U7_U_ASIC_DAC1 = 2;
    const uint8_t ADDR_U7_U_ASIC_DAC2 = 1;
    const uint8_t ADDR_U_DC_DC_DAC = 4;

    /** LTC2634 channels
     */
    const uint8_t REG_DAC_CHANNEL_VOUTA = 0x00;
    const uint8_t REG_DAC_CHANNEL_VOUTB = 0x01;
    const uint8_t REG_DAC_CHANNEL_VOUTC = 0x02;
    const uint8_t REG_DAC_CHANNEL_VOUTD = 0x03;

    const caribou::BIAS_REGULATOR_T BIAS_1("V1_0-2V5", ADDR_U7_U_ASIC_DAC0, REG_DAC_CHANNEL_VOUTA);
    const caribou::BIAS_REGULATOR_T BIAS_2("V2_0-2V5", ADDR_U7_U_ASIC_DAC0, REG_DAC_CHANNEL_VOUTB);
    const caribou::BIAS_REGULATOR_T BIAS_3("V3_0-2V5", ADDR_U7_U_ASIC_DAC0, REG_DAC_CHANNEL_VOUTC);
    const caribou::BIAS_REGULATOR_T BIAS_4("V4_0-2V5", ADDR_U7_U_ASIC_DAC0, REG_DAC_CHANNEL_VOUTD);

    const caribou::BIAS_REGULATOR_T BIAS_5("V5_0-2V5", ADDR_U7_U_ASIC_DAC1, REG_DAC_CHANNEL_VOUTA);
    const caribou::BIAS_REGULATOR_T BIAS_6("V6_0-2V5", ADDR_U7_U_ASIC_DAC1, REG_DAC_CHANNEL_VOUTB);
    const caribou::BIAS_REGULATOR_T BIAS_7("V7_0-2V5", ADDR_U7_U_ASIC_DAC1, REG_DAC_CHANNEL_VOUTC);
    const caribou::BIAS_REGULATOR_T BIAS_8("V8_0-30V", ADDR_U7_U_ASIC_DAC1, REG_DAC_CHANNEL_VOUTD);

    const caribou::BIAS_REGULATOR_T BIAS_9("V9_0-2V5", ADDR_U7_U_ASIC_DAC2, REG_DAC_CHANNEL_VOUTA);
    const caribou::BIAS_REGULATOR_T BIAS_10("V10_0-2V5", ADDR_U7_U_ASIC_DAC2, REG_DAC_CHANNEL_VOUTB);
    const caribou::BIAS_REGULATOR_T BIAS_11("V11_0-2V5", ADDR_U7_U_ASIC_DAC2, REG_DAC_CHANNEL_VOUTC);
    const caribou::BIAS_REGULATOR_T BIAS_12("V12_0-30V", ADDR_U7_U_ASIC_DAC2, REG_DAC_CHANNEL_VOUTD);

    const uint8_t DAC_BOOST_AMPLIFICATION_RATIO = 13;

    const caribou::DCDC_CONVERTER_T DCDC_VCSEL("VCSEL", ADDR_U_DC_DC_DAC, REG_DAC_CHANNEL_VOUTA);

    // ADC TI 122S051
    const uint8_t ADDR_U10_U_ADC = 5;

    // ADC TI 122S051 channels
    const uint8_t REG_ADC_CHANNEL_VIN1 = 0x00;
    const uint8_t REG_ADC_CHANNEL_VIN2 = 0x01;

    const caribou::SLOW_ADC_CHANNEL_T ADC_VIN_VCSEL("VIN_VCSEL", ADDR_U10_U_ADC, REG_ADC_CHANNEL_VIN1);
  } // namespace falconboard
} // namespace caribou

#endif
