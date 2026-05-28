#ifndef DEVICE_AD9249_DEFAULTS_H
#define DEVICE_AD9249_DEFAULTS_H

#include "utils/dictionary.hpp"

namespace caribou {

/** Default device path for this device: SPI interface
 */
#define DEFAULT_DEVICEPATH "/dev/spidev1.0"

  // AD9249 IP Control
  const std::intptr_t AD9249_RECEIVER_BASE_ADDRESS = 0x43C00000;
  const std::size_t AD9249_RECEIVER_MAP_SIZE = 4096;
  const std::intptr_t AD9249_RECEIVER_LSB = 2;

  // ADC0 readout
  const std::intptr_t ADC0_READOUT_BASE_ADDRESS = 0x30000000;
  const std::size_t ADC0_READOUT_MAP_SIZE = 134217728;

  // ADC1 readout
  const std::intptr_t ADC1_READOUT_BASE_ADDRESS = 0x38000000;
  const std::size_t ADC1_READOUT_MAP_SIZE = 134217728;

  const memory_map AD9249_RECEIVER{AD9249_RECEIVER_BASE_ADDRESS, AD9249_RECEIVER_MAP_SIZE, PROT_READ | PROT_WRITE};
  // const memory_map ADC0_READOUT{ADC0_READOUT_BASE_ADDRESS, ADC0_READOUT_MAP_SIZE, PROT_READ | PROT_WRITE};
  // const memory_map ADC1_READOUT{ADC1_READOUT_BASE_ADDRESS, ADC1_READOUT_MAP_SIZE, PROT_READ | PROT_WRITE};

#define AD9249_MEMORY                                                                                                       \
  {                                                                                                                         \
    {"data_gen_reset", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(0 << AD9249_RECEIVER_LSB)}},            \
      {"trigger", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(1 << AD9249_RECEIVER_LSB)}},                 \
      {"fifo_reset", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(2 << AD9249_RECEIVER_LSB)}},              \
      {"address_reset", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(3 << AD9249_RECEIVER_LSB)}},           \
      {"burst_enable", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(4 << AD9249_RECEIVER_LSB)}},            \
      {"test_data_enable", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(5 << AD9249_RECEIVER_LSB)}},        \
      {"adc0_burst_length", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(6 << AD9249_RECEIVER_LSB)}},       \
      {"adc0_current_address", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(7 << AD9249_RECEIVER_LSB)}},    \
      {"adc1_burst_length", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(8 << AD9249_RECEIVER_LSB)}},       \
      {"adc1_current_address", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(9 << AD9249_RECEIVER_LSB)}},    \
      {"adc_data_clk_freq", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(10 << AD9249_RECEIVER_LSB)}},      \
      {"adc_frame_clk_freq", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(11 << AD9249_RECEIVER_LSB)}},     \
      {"adc_framedata_alignment",                                                                                           \
       {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(12 << AD9249_RECEIVER_LSB)}},                           \
      {"adc_finedelay", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(13 << AD9249_RECEIVER_LSB)}},          \
      {"adc_fine_delay_rb", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(14 << AD9249_RECEIVER_LSB)}},      \
      {"fine_delay_stable", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(15 << AD9249_RECEIVER_LSB)}},      \
      {"trigger_flags", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(16 << AD9249_RECEIVER_LSB)}},          \
      {"trigger_threshold", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(17 << AD9249_RECEIVER_LSB)}},      \
      {"data_delay", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(18 << AD9249_RECEIVER_LSB)}},             \
      {"adc0_trigger_count", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(19 << AD9249_RECEIVER_LSB)}},     \
      {"adc1_trigger_count", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(20 << AD9249_RECEIVER_LSB)}},     \
      {"counter_reset", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(21 << AD9249_RECEIVER_LSB)}},          \
      {"deadtime_cycles", {AD9249_RECEIVER, register_t<std::uintptr_t, std::uintptr_t>(22 << AD9249_RECEIVER_LSB)}},        \
  }

  //{"adc0_readout", {ADC0_READOUT}},
  //{"adc1_readout", {ADC1_READOUT}},

// clang-format off
#define AD9249_REGISTERS						\
  {									\
    {"spi_port_config", register_t<uint16_t, uint8_t>(0x0000, 0xFF,0x18, false, true,false)},			\
    {"chip_id", register_t<uint16_t, uint8_t>(0x0100, 0xFF,0x92, false, false,false)},	\
    {"chip_grade", register_t<uint16_t, uint8_t>(0x0200,0xFF,0b00110000,false,true,false)},				\
    {"device_index2", register_t<uint16_t, uint8_t>(0x0400,0xFF,0x0F,false,true,false)},				\
    {"device_index1", register_t<uint16_t, uint8_t>(0x0500,0xFF,0x0F,false,true,false)},					\
    {"transfer", register_t<uint16_t, uint8_t>(0xFF00,0xFF,0x00,false,true,false)},				\
    {"power_mode", register_t<uint16_t, uint8_t>(0x800,0xFF,0x0,false,true,false)},				\
    {"clock", register_t<uint16_t, uint8_t>(0x0900,0xFF, 0x01,false,true,false)},			\
    {"clock_divide", register_t<uint16_t, uint8_t>(0x0B00, 0xFF,0x0,false,true,false)},			\
    {"enhancement_control", register_t<uint16_t, uint8_t>(0x0C00,0xFF,0x0,false,true,false)},				\
    {"test_mode", register_t<uint16_t, uint8_t>(0x0D00,0xFF,0x0,false,true,false)},				\
    {"offset_adjust", register_t<uint16_t, uint8_t>(0x1000,0xFF,0x00,false,true,false)},				\
    {"output_mode", register_t<uint16_t, uint8_t>(0x1400,0xFF,0x01,false,true,false)},					\
    {"output_adjust", register_t<uint16_t, uint8_t>(0x1500,0xFF,0x0,false,true,false)},				\
    {"output_phase", register_t<uint16_t, uint8_t>(0x1600,0xFF,0x03,false,true,false)},				\
    {"vref", register_t<uint16_t, uint8_t>(0x1600,0xFF,0x03,false,true,false)},				\
    {"user_pattern1_lsb", register_t<uint16_t, uint8_t>(0x1900,0xFF,0x0,false,true,false)},				\
    {"user_pattern1_msb", register_t<uint16_t, uint8_t>(0x1A00,0xFF,0x0,false,true,false)},				\
    {"user_pattern2_lsb", register_t<uint16_t, uint8_t>(0x1B00, 0xFF,0x0, false, true, true)},  	\
    {"user_pattern2_msb", register_t<uint16_t, uint8_t>(0x1C00,0xFF,0x0,false,true,false)},				\
    {"serial_output_data_ctrl", register_t<uint16_t, uint8_t>(0x2100,0xFF,0x41,false,true,false)},				\
    {"serial_channel_status", register_t<uint16_t, uint8_t>(0x2200, 0xFF,0x0, false, true, false)},          \
    {"resolution_sample_override", register_t<uint16_t, uint8_t>(0x0001,0xFF,0x00,false,true,false)},  				\
    {"user_io_ctrl2", register_t<uint16_t, uint8_t>(0x0101,0xFF,0x0,false,true,false)},				    									\
    {"user_io_ctrl3", register_t<uint16_t, uint8_t>(0x0201, 0xFF,0x0,false,true,false)},			\
    {"sync", register_t<uint16_t, uint8_t>(0x0901,0xFF,0x0,false,true,false)},				\
  }
  // clang-format on

} // namespace caribou

#endif /* DEVICE_AD9249_DEFAULTS_H */
