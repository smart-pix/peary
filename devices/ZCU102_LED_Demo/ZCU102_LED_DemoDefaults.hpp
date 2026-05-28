#ifndef DEVICE_ZCU102_LED_DEMO_DEFAULTS_H
#define DEVICE_ZCU102_LED_DEMO_DEFAULTS_H

#include "utils/dictionary.hpp"

namespace caribou {

  // define voltages


  // DSIPM FPGA address space
  const intptr_t DEMO_BASE_ADDRESS = 0x400000000;

  const size_t DEMO_MEM_SIZE = 0x30000;

  // These are offsets *relative* to the base address
  const intptr_t FPGA_REG_GPIO_0_DATA = 0x0;
  const intptr_t FPGA_REG_GPIO_0_TRI  = 0x4;
  const intptr_t FPGA_REG_GPIO_1_DATA = 0x10000;
  const intptr_t FPGA_REG_GPIO_1_TRI  = 0x10004;

  
  const memory_map FPGA_MEM{DEMO_BASE_ADDRESS, DEMO_MEM_SIZE, PROT_READ | PROT_WRITE};



#define FPGA_REGS                                                                                                           \
  {                                                                                                                         \
    {"gpio_0_data",              {FPGA_MEM, register_t<size_t>(FPGA_REG_GPIO_0_DATA,   0xFF,         true, true,  false)}}, \
      {"gpio_0_tri",             {FPGA_MEM, register_t<size_t>(FPGA_REG_GPIO_0_TRI,    0xFF,        true, true,  false)}}, \
      {"gpio_1_data",            {FPGA_MEM, register_t<size_t>(FPGA_REG_GPIO_1_DATA,   0xFF,        true, true,  false)}}, \
      {"gpio_1_tri",             {FPGA_MEM, register_t<size_t>(FPGA_REG_GPIO_1_TRI,    0xFF,        true,  true, false)}} \
  }
  /*
    {"pg_conf_pattern_length",   {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_PG_CONTROL,   0x0000FFE0, 0x7FF, true,  true,  false)}}, \
    {"pg_conf_number_of_runs",   {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_PG_CONTROL,   0xFFFF0000, 0x0,   true,  true,  false)}}, \
    {"sdi_cmd_rst",              {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_SDI_CONTROL,  0x00000001,        false, true,  false)}}, \
    {"sdi_cmd_write_config",     {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_SDI_CONTROL,  0x00000002,        false, true,  false)}}, \
    {"sdi_conf_sanity_check_en", {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_SDI_CONTROL,  0x00000004, 0x1,   true,  true,  false)}}, \
    {"sdi_stat_write_done",      {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_SDI_CONTROL,  0x00000010,        true,  false, false)}}, \
    {"sdi_err_sanity_check",     {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_SDI_CONTROL,  0x00000020,        true,  false, false)}}, \
    {"sdi_conf_bits",            {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN,  0x00FFFFFF,        true,  true,  false)}}, \
    {"sdi_readout_bits",         {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_OUT, 0x00FFFFFF,        true,  false, false)}}, \
    {"readout_en",               {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_READOUT_CONTROL, 0x00000001, 0x0, true,  true, false)}}, \
    {"rx_strobe_delay",          {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_READOUT_CONTROL, 0x0000000E, 0x3, true,  true, false)}}, \
    {"rx_strobe_sel",            {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_READOUT_CONTROL, 0x00000010, 0x0, true,  true, false)}}, \
    {"rx_strobe_neg",            {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_READOUT_CONTROL, 0x00000020, 0x0, true,  true, false)}}, \
    {"pg_clk_ce",                {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_READOUT_CONTROL, 0x00000040, 0x1, true,  true, false)}}, \
    {"data_out",                 {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_READOUT_DATA, 0xFFFFFFFF,        true,  false, false)}}  \
    }*/



} // namespace caribou

#endif /* DEVICE_SPARKDREAM_DEFAULTS_H */
