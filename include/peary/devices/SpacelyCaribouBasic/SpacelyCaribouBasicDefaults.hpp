#ifndef DEVICE_SPACELYCARIBOUBASIC_DEFAULTS_H
#define DEVICE_SPACELYCARIBOUBASIC_DEFAULTS_H

#include "utils/dictionary.hpp"

namespace caribou {


  // FPGA address space
  const intptr_t DEMO_BASE_ADDRESS = 0x400000000;

  const size_t DEMO_MEM_SIZE = 0x30000;

  // These are offsets *relative* to the base address
  const intptr_t FPGA_REG_GPIO_0_DATA = 0x0;
  const intptr_t FPGA_REG_GPIO_0_TRI  = 0x4;
  const intptr_t FPGA_REG_GPIO_1_DATA = 0x10000;
  const intptr_t FPGA_REG_GPIO_1_TRI  = 0x10004;

  
  const memory_map FPGA_MEM{DEMO_BASE_ADDRESS, DEMO_MEM_SIZE, PROT_READ | PROT_WRITE};

  //Add generated mem map here...



#define FPGA_REGS \
 { \
  {"sw_write32_0", {FPGA_MEM, register_t<size_t>(0x000000, 0xffffffff, true, true, false)}}, \
  {"sw_read32_0", {FPGA_MEM, register_t<size_t>(0x000004, 0xffffffff, true, true, false)}}, \
  {"sw_read32_1", {FPGA_MEM, register_t<size_t>(0x000008, 0xffffffff, true, true, false)}}, \
  {"uplinkRst", {FPGA_MEM, register_t<size_t>(0x000000, 0x000001, true, true, false)}}, \
  {"mgt_rxpolarity", {FPGA_MEM, register_t<size_t>(0x000000, 0x000002, true, true, false)}}, \
  {"lpgbtfpga_status", {FPGA_MEM, register_t<size_t>(0x000004, 0xffffffff, true, false, false)}}, \
  {"spi_apg_run", {FPGA_MEM, register_t<size_t>(0x001000, 0x000001, false, true, false)}}, \
  {"spi_apg_write_channel", {FPGA_MEM, register_t<size_t>(0x001004, 0xffffffff, true, true, false)}}, \
  {"spi_apg_read_channel", {FPGA_MEM, register_t<size_t>(0x001008, 0xffffffff, true, false, false)}}, \
  {"spi_apg_sample_count", {FPGA_MEM, register_t<size_t>(0x00100c, 0xffffffff, true, false, false)}}, \
  {"spi_apg_n_samples", {FPGA_MEM, register_t<size_t>(0x001010, 0xffffffff, true, true, false)}}, \
  {"spi_apg_control", {FPGA_MEM, register_t<size_t>(0x001014, 0x0000ff, true, true, false)}}, \
  {"spi_apg_write_buffer_len", {FPGA_MEM, register_t<size_t>(0x001018, 0xffffffff, true, false, false)}}, \
  {"spi_apg_next_read_sample", {FPGA_MEM, register_t<size_t>(0x00101c, 0xffffffff, true, false, false)}}, \
  {"spi_apg_wave_ptr", {FPGA_MEM, register_t<size_t>(0x001020, 0xffffffff, true, false, false)}}, \
  {"spi_apg_status", {FPGA_MEM, register_t<size_t>(0x001024, 0x000007, true, false, false)}}, \
  {"spi_apg_clear", {FPGA_MEM, register_t<size_t>(0x001028, 0x000001, false, true, false)}}, \
  {"spi_apg_dbg_error", {FPGA_MEM, register_t<size_t>(0x00102c, 0xffffffff, true, false, false)}}, \
  {"lpgbt_rd_en", {FPGA_MEM, register_t<size_t>(0x002000, 0x000001, true, true, false)}}, \
  {"empty", {FPGA_MEM, register_t<size_t>(0x002004, 0x000001, true, false, false)}}, \
  {"full", {FPGA_MEM, register_t<size_t>(0x002004, 0x000002, true, false, false)}}, \
  {"data_frame[0]", {FPGA_MEM, register_t<size_t>(0x002008, 0xffffffff, false, false, false)}}, \
  {"data_frame[1]", {FPGA_MEM, register_t<size_t>(0x00200c, 0xffffffff, false, false, false)}}, \
  {"data_frame[2]", {FPGA_MEM, register_t<size_t>(0x002010, 0xffffffff, false, false, false)}}, \
  {"data_frame[3]", {FPGA_MEM, register_t<size_t>(0x002014, 0xffffffff, false, false, false)}}, \
  {"data_frame[4]", {FPGA_MEM, register_t<size_t>(0x002018, 0xffffffff, false, false, false)}}, \
  {"data_frame[5]", {FPGA_MEM, register_t<size_t>(0x00201c, 0xffffffff, false, false, false)}}, \
  {"data_frame[6]", {FPGA_MEM, register_t<size_t>(0x002020, 0xffffffff, false, false, false)}}, \
  {"data_frame[7]", {FPGA_MEM, register_t<size_t>(0x002024, 0x0003ff, false, false, false)}}, \
  {"err_counter", {FPGA_MEM, register_t<size_t>(0x002028, 0xffffffff, false, false, false)}}, \
  {"gpio_data", {FPGA_MEM, register_t<size_t>(0x003000, 0xffffffff, true, true, false)}}, \
  {"gpio_direction", {FPGA_MEM, register_t<size_t>(0x003004, 0xffffffff, true, true, false)}}, \
  {"apg_run", {FPGA_MEM, register_t<size_t>(0x004000, 0x000001, false, true, false)}}, \
  {"apg_write_channel", {FPGA_MEM, register_t<size_t>(0x004004, 0xfffffff, true, true, false)}}, \
  {"apg_read_channel", {FPGA_MEM, register_t<size_t>(0x004008, 0xffffffff, true, false, false)}}, \
  {"apg_sample_count", {FPGA_MEM, register_t<size_t>(0x00400c, 0xffffffff, true, false, false)}}, \
  {"apg_n_samples", {FPGA_MEM, register_t<size_t>(0x004010, 0xffffffff, true, true, false)}}, \
  {"apg_control", {FPGA_MEM, register_t<size_t>(0x004014, 0x0000ff, true, true, false)}}, \
  {"apg_write_buffer_len", {FPGA_MEM, register_t<size_t>(0x004018, 0xffffffff, true, false, false)}}, \
  {"apg_next_read_sample", {FPGA_MEM, register_t<size_t>(0x00401c, 0xffffffff, true, false, false)}}, \
  {"apg_wave_ptr", {FPGA_MEM, register_t<size_t>(0x004020, 0xffffffff, true, false, false)}}, \
  {"apg_status", {FPGA_MEM, register_t<size_t>(0x004024, 0x000007, true, false, false)}}, \
  {"apg_clear", {FPGA_MEM, register_t<size_t>(0x004028, 0x000001, false, true, false)}}, \
  {"apg_dbg_error", {FPGA_MEM, register_t<size_t>(0x00402c, 0xffffffff, true, false, false)}}, \
  {"count_0", {FPGA_MEM, register_t<size_t>(0x005000, 0xffffffff, true, false, false)}}, \
  {"count_1", {FPGA_MEM, register_t<size_t>(0x005004, 0xffffffff, true, false, false)}}, \
  {"count_2", {FPGA_MEM, register_t<size_t>(0x005008, 0xffffffff, true, false, false)}}, \
  {"count_3", {FPGA_MEM, register_t<size_t>(0x00500c, 0xffffffff, true, false, false)}}, \
  {"count_4", {FPGA_MEM, register_t<size_t>(0x005010, 0xffffffff, true, false, false)}}, \
  {"count_5", {FPGA_MEM, register_t<size_t>(0x005014, 0xffffffff, true, false, false)}}, \
  {"count_6", {FPGA_MEM, register_t<size_t>(0x005018, 0xffffffff, true, false, false)}}, \
  {"count_7", {FPGA_MEM, register_t<size_t>(0x00501c, 0xffffffff, true, false, false)}}, \
  {"divider_cycles", {FPGA_MEM, register_t<size_t>(0x006000, 0xffffffff, true, true, false)}}, \
  {"divider_rstn", {FPGA_MEM, register_t<size_t>(0x006004, 0x000001, true, true, false)}}, \
}




} // namespace caribou

#endif /* DEVICE_SPACELYCARIBOUBASIC_DEFAULTS_H */
