#ifndef DEVICE_CORDIA_DEFAULTS_H
#define DEVICE_CORDIA_DEFAULTS_H

#include "utils/dictionary.hpp"

namespace caribou {

  // define voltages

#define CoRDIA_VDDA 1.2
#define CoRDIA_VDDA_CURRENT 1
#define CoRDIA_VDDPA 1.2
#define CoRDIA_VDDPA_CURRENT 1
#define CoRDIA_VDDD 1.2
#define CoRDIA_VDDD_CURRENT 1
#define CoRDIA_VDD33A 3.3
#define CoRDIA_VDD33A_CURRENT 1
#define CoRDIA_VDD33B 3.3
#define CoRDIA_VDD33B_CURRENT 1
#define CoRDIA_COMP_VREF 0.9
#define CoRDIA_BPLATE1 0.4
#define CoRDIA_BPLATE2 0.6
#define CoRDIA_VREF_P 0.9
#define CoRDIA_VREF_M 0.3
#define CoRDIA_ISOURCE 44
#define CoRDIA_ISOURCE_DIR 1
#define CoRDIA_COMP_ITAIL 150
#define CoRDIA_COMP_ITAIL_DIR 1
#define CoRDIA_CDS_IBP 250
#define CoRDIA_CDS_IBP_DIR 0
#define CoRDIA_PULSEC 1.5
#define CoRDIA_CMOS_OUT_1_TO_4 1.2
#define CoRDIA_CMOS_OUT_5_TO_8 1.2
#define CoRDIA_CMOS_IN_1_TO_4 1.2
#define CoRDIA_CMOS_IN_5_TO_8 1.2
#define CoRDIA_CMOS_IN_9_TO_12 1.2
#define CoRDIA_CMOS_IN_13_TO_14 1.2

// outfile names
#define dSiPM_frameFilename "frameData.txt"
#define dSiPM_LEDframeFilename "LEDframeData.txt"
#define dSiPM_BINframeFilename "BINframeData.bin"
#define dSiPM_temperatureFilename "temperatureData.txt"

// CoRDIA pattern generator parameters
#define CORDIA_PG_PATTERN_WIDTH 23
#define CORDIA_PG_PATTERN_MAX_LENGTH 2048
#define CORDIA_PG_PATTERN_MAX_RUNS 65535

  // DSIPM FPGA address space
  const intptr_t CORDIA_CORE_BASE_ADDRESS = 0x43C70000;
  const intptr_t CORDIA_CORE_LSB = 2;
  const size_t CORDIA_WORD_SIZE = 32;

  const size_t CORDIA_FPGA_MEM_SIZE = 8 << CORDIA_CORE_LSB;
  const size_t CORDIA_PG_MEM_SIZE = CORDIA_PG_PATTERN_MAX_LENGTH << CORDIA_CORE_LSB;

  // These are offsets *relative* to the base address
  const intptr_t CORDIA_FPGA_REG_PG_CONTROL = 0x0 << CORDIA_CORE_LSB;
  const intptr_t CORDIA_FPGA_REG_SDI_CONTROL = 0x1 << CORDIA_CORE_LSB;
  const intptr_t CORDIA_FPGA_REG_SDI_DATA_IN = 0x2 << CORDIA_CORE_LSB;
  const intptr_t CORDIA_FPGA_REG_SDI_DATA_OUT = 0x3 << CORDIA_CORE_LSB;
  const intptr_t CORDIA_FPGA_REG_SDI_DATA_IN_AUT = 0x4 << CORDIA_CORE_LSB;
  const intptr_t CORDIA_FPGA_REG_READOUT_CONTROL = 0x5 << CORDIA_CORE_LSB;
  const intptr_t CORDIA_FPGA_REG_READOUT_DATA = 0x6 << CORDIA_CORE_LSB;

  const intptr_t CORDIA_FPGA_MEM_BASE = CORDIA_CORE_BASE_ADDRESS + 0;
  const intptr_t CORDIA_PG_MEM_BASE = CORDIA_CORE_BASE_ADDRESS + (0x800 << CORDIA_CORE_LSB);

  const memory_map CORDIA_FPGA_MEM{CORDIA_FPGA_MEM_BASE, CORDIA_FPGA_MEM_SIZE, PROT_READ | PROT_WRITE};
  const memory_map CORDIA_PG_MEM{CORDIA_PG_MEM_BASE, CORDIA_PG_MEM_SIZE, PROT_WRITE};

  // clang-format off

#define CORDIA_FPGA_REGS                                                                                                                       \
  {                                                                                                                                            \
    {"pg_cmd_rst",               {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_PG_CONTROL,   0x00000001,        false, true,  false)}}, \
    {"pg_cmd_start",             {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_PG_CONTROL,   0x00000002,        false, true,  false)}}, \
    {"pg_cmd_stop",              {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_PG_CONTROL,   0x00000004,        false, true,  false)}}, \
    {"pg_stat_running",          {CORDIA_FPGA_MEM, register_t<size_t>(CORDIA_FPGA_REG_PG_CONTROL,   0x00000010,        true,  false, false)}}, \
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
  }

#define CORDIA_PG_REGS                                                                                   \
  {                                                                                                      \
    {"pg_mem_base",              {CORDIA_PG_MEM, register_t<size_t>(0, 0xFFFFFFFF, false, true, false)}} \
  }

// clang-format on

/** Dictionary for register address/name lookup for CoRDIA
 */

// clang-format off

// {"<name>",       register_t<size_t>(CORDIA_CHIP_CONF_ADDR, <mask>, <default>, <read>,  <write>,  <special>)}
#define CORDIA_CHIP_REGS                                                                   \
  {                                                                                        \
    {"inject_pix0",       register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00000001, true,  true,  false)}, \
    {"inject_pix1",       register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00000002, true,  true,  false)}, \
    {"inject_pix2",       register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00000004, true,  true,  false)}, \
    {"inject_pix3",       register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00000008, true,  true,  false)}, \
    {"inject_pix4",       register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00000010, true,  true,  false)}, \
    {"inject_pix5",       register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00000020, true,  true,  false)}, \
    {"inject_pix6",       register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00000040, true,  true,  false)}, \
    {"inject_pix7",       register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00000080, true,  true,  false)}, \
    {"inject_pix8",       register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00000100, true,  true,  false)}, \
    {"inject_pix9",       register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00000200, true,  true,  false)}, \
    {"inject_pix10",      register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00000400, true,  true,  false)}, \
    {"inject_pix11",      register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00000800, true,  true,  false)}, \
    {"inject_pix12",      register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00001000, true,  true,  false)}, \
    {"inject_pix13",      register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00002000, true,  true,  false)}, \
    {"inject_pix14",      register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00004000, true,  true,  false)}, \
    {"inject_pix15",      register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00008000, true,  true,  false)}, \
    {"use_isource",       register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00010000, true,  true,  false)}, \
    {"use_pulsec",        register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00020000, true,  true,  false)}, \
    {"set_gn1_external",  register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00040000, true,  true,  false)}, \
    {"notgn2_cds",        register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00080000, true,  true,  false)}, \
    {"notsc_cds",         register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00100000, true,  true,  false)}, \
    {"select_biases2adc", register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00200000, true,  true,  false)}, \
    {"mon_spix_addr22",   register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00400000, true,  true,  false)}, \
    {"mon_spix_addr23",   register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00800000, true,  true,  false)}, \
    {"full_config",       register_t<size_t>(CORDIA_FPGA_REG_SDI_DATA_IN_AUT, 0x00FFFFFF, true,  true,  false)}, \
    {"pg_freq",           register_t<size_t>(0,                               0x00000000, false, true,  true)}   \
  }

  // clang-format on

} // namespace caribou

#endif /* DEVICE_CORDIA_DEFAULTS_H */
