#pragma once

#include "utils/dictionary.hpp"

namespace caribou {

/** Supply currents and biases
 * voltages in V,
 * currents in A
 */
#define H2M_VDDD 1.2
#define H2M_VDDA 1.2
#define H2M_LVDS 3.3
#define H2M_VCIRCUIT 3.3
#define H2M_VIO 1.2
#define H2M_VSUB -1.2   // this is VSUB
#define H2M_VPWELL -1.2 // this is VPWELL

#define H2M_VDDD_CURRENT 0.3     // to be tested
#define H2M_VDDA_CURRENT 0.3     // to be tested
#define H2M_LVDS_CURRENT 0.5     // to be tested
#define H2M_VCIRCUIT_CURRENT 0.1 // to be tested
#define H2M_VIO_CURRENT 0.0      // to be decided!

/** define pattern generator signals
 */
#define H2M_PG_SH 0x1
#define H2M_PG_AQ 0x2
#define H2M_PG_RO 0x4
#define H2M_PG_BS 0x8

/** outfile names
 */
#define h2m_frameFilename "frameData.txt"
#define h2m_testpulseFilename "testpulseData.txt"
#define h2m_occupancyFilename "occupancyData"

  /** occupancy scan parameter
   */
  const std::map<std::string, int> h2m_dac_index{{"dac_vtpulse", 0}, // will default to test-pulse
                                                 {"dac_ibias", 1},
                                                 {"dac_itrim", 2},
                                                 {"dac_ikrum", 3},
                                                 {"dac_vthr", 4},
                                                 {"dac_vref", 5}};
#define h2m_scan_start 0
#define h2m_scan_step 1
#define h2m_scan_end 255
#define h2m_scan_n 100
#define h2m_scan_dac "vtpulse"

  /** defining acquisition modes
   */
  const uint8_t ACQ_MODE_TOA = 0b00;
  const uint8_t ACQ_MODE_TOT = 0b01;
  const uint8_t ACQ_MODE_CNT = 0b10;
  const uint8_t ACQ_MODE_TRG = 0b11;

  /** define matrix dimensions
   */
  const uint16_t H2M_NCOL = 64;
  const uint16_t H2M_NROW = 16;
  const uint16_t H2M_NPXGROUP = 4;

  /** Frame length delivered by FW FIFO **/
  const unsigned int H2M_FW_EVENTSIZE = 262;

  /** Define address space.
   * Each address corresponds to a 32-bit memory block.
   * We have 64 address for each: CHIP, ROUT, and CTRL.
   * The READOUT_MAP_SIZE gives the size of the address space associated with the memory.
   */
  const intptr_t CORE_BASE_ADDRESS = 0x43C70000;
  const intptr_t CORE_LSB = 2;
  const size_t READOUT_MAP_SIZE = 4096;

  const intptr_t H2M_FPGA_REG_SC_IFACE_ADDR_WRITE = 0 << CORE_LSB;
  const intptr_t H2M_FPGA_REG_SC_IFACE_DATA_WRITE = 1 << CORE_LSB;
  const intptr_t H2M_FPGA_REG_SC_IFACE_ADDR_READ = 2 << CORE_LSB;
  const intptr_t H2M_FPGA_REG_SC_IFACE_DATA_RECEIVED = 3 << CORE_LSB;
  const intptr_t H2M_FPGA_REG_SC_IFACE_STATUS = 4 << CORE_LSB;
  const intptr_t H2M_FPGA_REG_SC_IFACE_ADDR_RECEIVED = 5 << CORE_LSB;
  const intptr_t H2M_FPGA_REG_CTRL = 6 << CORE_LSB;
  const intptr_t H2M_FPGA_PG_CTRL = 7 << CORE_LSB;
  const intptr_t H2M_FPGA_PG_TIMER_PATTERN = 8 << CORE_LSB;
  const intptr_t H2M_FPGA_PG_TIMEOUT_PATTERN = 9 << CORE_LSB;
  const intptr_t H2M_FPGA_PG_OUT_PATTERN = 10 << CORE_LSB;
  const intptr_t H2M_FPGA_PG_TRIG_PATTERN = 11 << CORE_LSB;
  const intptr_t H2M_FPGA_PG_RUNS_N = 12 << CORE_LSB;
  const intptr_t H2M_FPGA_REG_SHUTTER = 13 << CORE_LSB;
  const intptr_t H2M_FPGA_REG_READOUT_DATA = 14 << CORE_LSB;
  const intptr_t H2M_FPGA_REG_READOUT_INFO = 15 << CORE_LSB;
  const intptr_t H2M_FPGA_REG_DEBUG = 16 << CORE_LSB;

  /** Defining memory space --- i.e. everything we want to write to or read from the FPGA.
   *
   *
   * NOTE: We will have to discuss the structure of the memory for pixel data (reading and configuration).
   *       We have 8 bit per pixel and 1024 pixels (64 columns x 16 rows).
   *       One pixel group can contain data for 4 pixels, so we need 256 groups if we use 32 bit as for dSiPM.
   *       Does READOUT_MAP_SIZE allow that?
   *
   * NOTE: I am assigning the same memeroy block for reading of data and writing of configuration.
   *       Is that sane to do?
   *
   * TODO: Assign all the pixel groups once we dicided on the structure.
   */
  const memory_map H2M_MEM{CORE_BASE_ADDRESS, READOUT_MAP_SIZE, PROT_READ | PROT_WRITE};

  // clang-format off
#define H2M_MEMORY \
{ \
    {"sc_addr_write",         {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_SC_IFACE_ADDR_WRITE   , 0xFFFFFFFF, true,  true,  false)}}, \
    {"sc_data_write",         {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_SC_IFACE_DATA_WRITE   , 0xFFFFFFFF, true,  true,  false)}}, \
    {"sc_addr_read",          {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_SC_IFACE_ADDR_READ    , 0xFFFFFFFF, true,  true,  false)}}, \
    {"sc_data_received",      {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_SC_IFACE_DATA_RECEIVED, 0xFFFFFFFF, true,  false, false)}}, \
    {"sc_stat_idle",          {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_SC_IFACE_STATUS       , 0x00000001, true,  false, false)}}, \
    {"sc_err_cmd",            {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_SC_IFACE_STATUS       , 0x00000002, true,  false, false)}}, \
    {"sc_err_addr",           {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_SC_IFACE_STATUS       , 0x00000004, true,  false, false)}}, \
    {"sc_timeout",            {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_SC_IFACE_STATUS       , 0x00000008, true,  false, false)}}, \
    {"sc_testpule",           {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_SC_IFACE_STATUS       , 0x00000010, true,  false, false)}}, \
    {"sc_autoreadout",        {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_SC_IFACE_STATUS       , 0x00000020, true,  false, false)}}, \
    {"sc_status",             {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_SC_IFACE_STATUS       , 0x000000FF, true,  false, false)}}, \
    {"sc_addr_received",      {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_SC_IFACE_ADDR_RECEIVED, 0x0000FFFF, true,  false, false)}}, \
    {"sc_read_flag_received", {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_SC_IFACE_ADDR_RECEIVED, 0x00010000, true,  false, false)}}, \
    \
    {"chip_RESETn",           {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_CTRL                  , 0x00000001, true,  true,  false)}}, \
    {"fpga_reset",            {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_CTRL                  , 0x00000002, false, true,  false)}}, \
    {"sc_reset",              {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_CTRL                  , 0x00000008, false, true,  false)}}, \
    {"timer_start",           {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_CTRL                  , 0x00000010, false, true,  false)}}, \
    {"sc_ctrl_mode",          {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_CTRL                  , 0x00000020, true,  true,  false)}}, \
    {"buffer_LVDS1_pwr",      {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_CTRL                  , 0x00000100, true,  true,  false)}}, \
    {"buffer_LVDS1_pem",      {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_CTRL                  , 0x00000200, true,  true,  false)}}, \
    {"buffer_LVDS2_pwr",      {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_CTRL                  , 0x00000400, true,  true,  false)}}, \
    {"buffer_LVDS2_pem",      {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_CTRL                  , 0x00000800, true,  true,  false)}}, \
    {"readout_idle",          {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_CTRL                  , 0x00010000, true, false,  false)}}, \
    {"readout_acq_err",       {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_CTRL                  , 0x00020000, true, false,  false)}}, \
    \
    {"pg_start",              {H2M_MEM, register_t<size_t>(H2M_FPGA_PG_CTRL                   , 0x00000001, false, true,  false)}}, \
    {"pg_stop",               {H2M_MEM, register_t<size_t>(H2M_FPGA_PG_CTRL                   , 0x00000002, false, true,  false)}}, \
    {"pg_rst",                {H2M_MEM, register_t<size_t>(H2M_FPGA_PG_CTRL                   , 0x00000004, false, true,  false)}}, \
    {"pg_write_pattern",      {H2M_MEM, register_t<size_t>(H2M_FPGA_PG_CTRL                   , 0x00000008, false, true,  false)}}, \
    {"pg_running",            {H2M_MEM, register_t<size_t>(H2M_FPGA_PG_CTRL                   , 0x00000100, true,  false, false)}}, \
    {"pg_mem_remaining",      {H2M_MEM, register_t<size_t>(H2M_FPGA_PG_CTRL                   , 0xFFFF0000, true,  false, false)}}, \
    {"pg_timer_pattern",      {H2M_MEM, register_t<size_t>(H2M_FPGA_PG_TIMER_PATTERN          , 0xFFFFFFFF, true,  true,  false)}}, \
    {"pg_timeout_pattern",    {H2M_MEM, register_t<size_t>(H2M_FPGA_PG_TIMEOUT_PATTERN        , 0xFFFFFFFF, true,  true,  false)}}, \
    {"pg_output_pattern",     {H2M_MEM, register_t<size_t>(H2M_FPGA_PG_OUT_PATTERN            , 0xFFFFFFFF, true,  true,  false)}}, \
    {"pg_triggers_pattern",   {H2M_MEM, register_t<size_t>(H2M_FPGA_PG_TRIG_PATTERN           , 0xFFFFFFFF, true,  true,  false)}}, \
    {"pg_runs",               {H2M_MEM, register_t<size_t>(H2M_FPGA_PG_RUNS_N                 , 0xFFFFFFFF, true,  true,  false)}}, \
    \
    {"shutter",               {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_SHUTTER               , 0xFFFFFFFF, true,  true,  false)}}, \
    \
    {"data",                  {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_READOUT_DATA          , 0xFFFFFFFF, true, false,  false)}}, \
    {"fifoStatus",            {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_READOUT_INFO          , 0x7FFFFFFF, true, false,  false)}}, \
    {"debug",                 {H2M_MEM, register_t<size_t>(H2M_FPGA_REG_DEBUG                 , 0x7FFFFFFF, true, true,   false)}} \
}
// clang-format on

/** Defining registers --- i.e. everything we want to write to or read from the CHIP.
 */

// clang-format off
#define H2M_REGISTERS \
{ \
    {"acq_start",        register_t<size_t, size_t>(0x0001, 0x0001, true,  true,  false)}, \
    {"conf",             register_t<size_t, size_t>(0x0001, 0x0002, true,  true,  false)}, \
    \
    {"acq_mode",         register_t<size_t, size_t>(0x0001, 0x000C, true,  true,  false)}, \
    {"shutter_select",   register_t<size_t, size_t>(0x0001, 0x0010, true,  true,  false)}, \
    {"latency",          register_t<size_t, size_t>(0x0001, 0xFF00, true,  true,  false)}, \
    \
    {"analog_out_ctrl",  register_t<size_t, size_t>(0x0003, 0x0007, true,  true,  false)}, \
    {"digital_out_ctrl", register_t<size_t, size_t>(0x0004, 0x000F, true,  true,  false)}, \
    {"matrix_ctrl",      register_t<size_t, size_t>(0x0005, 0x000F, true,  true,  true )}, \
    \
    {"shutter_tp_lat",   register_t<size_t, size_t>(0x0006, 0x007F, true,  true,  false)}, \
    {"link_shutter_tp",  register_t<size_t, size_t>(0x0006, 0x0080, true,  true,  false)}, \
    {"tp_polarity",      register_t<size_t, size_t>(0x0006, 0x0100, true,  true,  false)}, \
    {"tp_sel_digital",   register_t<size_t, size_t>(0x0006, 0x0200, true,  true,  false)}, \
    \
    {"tp_num",           register_t<size_t, size_t>(0x0007, 0xFFFF, true,  true,  false)}, \
    {"tp_on",            register_t<size_t, size_t>(0x0008, 0xFFFF, true,  true,  false)}, \
    {"tp_off",           register_t<size_t, size_t>(0x0009, 0xFFFF, true,  true,  false)}, \
    \
    {"dac_ibias",        register_t<size_t, size_t>(0x0010, 0x00FF, 0x20, true,  true,  false)}, \
    {"dac_itrim",        register_t<size_t, size_t>(0x0011, 0x00FF, 0x20, true,  true,  false)}, \
    {"dac_ikrum",        register_t<size_t, size_t>(0x0012, 0x00FF, 0x20, true,  true,  false)}, \
    {"dac_vthr",         register_t<size_t, size_t>(0x0013, 0x00FF, 0x46, true,  true,  false)}, \
    {"dac_vref",         register_t<size_t, size_t>(0x0014, 0x00FF, 0x46, true,  true,  false)}, \
    {"dac_vtpulse",      register_t<size_t, size_t>(0x0015, 0x00FF, 0x00, true,  true,  false)}, \
    \
    {"bgr_res_0",        register_t<size_t, size_t>(0x0016, 0x000F, true,  true,  false)}, \
    {"bgr_res_1",        register_t<size_t, size_t>(0x0016, 0x0030, true,  true,  false)}, \
    {"bgr_res_2",        register_t<size_t, size_t>(0x0016, 0x00C0, true,  true,  false)}, \
    {"bgr_iref_trim",    register_t<size_t, size_t>(0x0016, 0x0F00, true,  true,  false)}, \
    \
    {"enable_bridge",    register_t<size_t, size_t>(0x0017, 0x00FF, true,  true,  false)}, \
    \
    {"px_cnt_w_0",         register_t<size_t, size_t>(0xA000, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_1",         register_t<size_t, size_t>(0xA001, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_2",         register_t<size_t, size_t>(0xA002, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_3",         register_t<size_t, size_t>(0xA003, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_4",         register_t<size_t, size_t>(0xA004, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_5",         register_t<size_t, size_t>(0xA005, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_6",         register_t<size_t, size_t>(0xA006, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_7",         register_t<size_t, size_t>(0xA007, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_8",         register_t<size_t, size_t>(0xA008, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_9",         register_t<size_t, size_t>(0xA009, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_10",        register_t<size_t, size_t>(0xA00A, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_11",        register_t<size_t, size_t>(0xA00B, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_12",        register_t<size_t, size_t>(0xA00C, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_13",        register_t<size_t, size_t>(0xA00D, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_14",        register_t<size_t, size_t>(0xA00E, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_15",        register_t<size_t, size_t>(0xA00F, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_16",        register_t<size_t, size_t>(0xA010, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_17",        register_t<size_t, size_t>(0xA011, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_18",        register_t<size_t, size_t>(0xA012, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_19",        register_t<size_t, size_t>(0xA013, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_20",        register_t<size_t, size_t>(0xA014, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_21",        register_t<size_t, size_t>(0xA015, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_22",        register_t<size_t, size_t>(0xA016, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_23",        register_t<size_t, size_t>(0xA017, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_24",        register_t<size_t, size_t>(0xA018, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_25",        register_t<size_t, size_t>(0xA019, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_26",        register_t<size_t, size_t>(0xA01A, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_27",        register_t<size_t, size_t>(0xA01B, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_28",        register_t<size_t, size_t>(0xA01C, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_29",        register_t<size_t, size_t>(0xA01D, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_30",        register_t<size_t, size_t>(0xA01E, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_31",        register_t<size_t, size_t>(0xA01F, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_32",        register_t<size_t, size_t>(0xA020, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_33",        register_t<size_t, size_t>(0xA021, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_34",        register_t<size_t, size_t>(0xA022, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_35",        register_t<size_t, size_t>(0xA023, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_36",        register_t<size_t, size_t>(0xA024, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_37",        register_t<size_t, size_t>(0xA025, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_38",        register_t<size_t, size_t>(0xA026, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_39",        register_t<size_t, size_t>(0xA027, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_40",        register_t<size_t, size_t>(0xA028, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_41",        register_t<size_t, size_t>(0xA029, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_42",        register_t<size_t, size_t>(0xA02A, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_43",        register_t<size_t, size_t>(0xA02B, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_44",        register_t<size_t, size_t>(0xA02C, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_45",        register_t<size_t, size_t>(0xA02D, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_46",        register_t<size_t, size_t>(0xA02E, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_47",        register_t<size_t, size_t>(0xA02F, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_48",        register_t<size_t, size_t>(0xA030, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_49",        register_t<size_t, size_t>(0xA031, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_50",        register_t<size_t, size_t>(0xA032, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_51",        register_t<size_t, size_t>(0xA033, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_52",        register_t<size_t, size_t>(0xA034, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_53",        register_t<size_t, size_t>(0xA035, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_54",        register_t<size_t, size_t>(0xA036, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_55",        register_t<size_t, size_t>(0xA037, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_56",        register_t<size_t, size_t>(0xA038, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_57",        register_t<size_t, size_t>(0xA039, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_58",        register_t<size_t, size_t>(0xA03A, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_59",        register_t<size_t, size_t>(0xA03B, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_60",        register_t<size_t, size_t>(0xA03C, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_61",        register_t<size_t, size_t>(0xA03D, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_62",        register_t<size_t, size_t>(0xA03E, 0xFFFFFFFF, false,  true,  false)}, \
    {"px_cnt_w_63",        register_t<size_t, size_t>(0xA03F, 0xFFFFFFFF, false,  true,  false)}, \
    \
    {"px_cnt_r_0",         register_t<size_t, size_t>(0xA000, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_1",         register_t<size_t, size_t>(0xA001, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_2",         register_t<size_t, size_t>(0xA002, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_3",         register_t<size_t, size_t>(0xA003, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_4",         register_t<size_t, size_t>(0xA004, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_5",         register_t<size_t, size_t>(0xA005, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_6",         register_t<size_t, size_t>(0xA006, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_7",         register_t<size_t, size_t>(0xA007, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_8",         register_t<size_t, size_t>(0xA008, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_9",         register_t<size_t, size_t>(0xA009, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_10",        register_t<size_t, size_t>(0xA00A, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_11",        register_t<size_t, size_t>(0xA00B, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_12",        register_t<size_t, size_t>(0xA00C, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_13",        register_t<size_t, size_t>(0xA00D, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_14",        register_t<size_t, size_t>(0xA00E, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_15",        register_t<size_t, size_t>(0xA00F, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_16",        register_t<size_t, size_t>(0xA010, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_17",        register_t<size_t, size_t>(0xA011, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_18",        register_t<size_t, size_t>(0xA012, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_19",        register_t<size_t, size_t>(0xA013, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_20",        register_t<size_t, size_t>(0xA014, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_21",        register_t<size_t, size_t>(0xA015, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_22",        register_t<size_t, size_t>(0xA016, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_23",        register_t<size_t, size_t>(0xA017, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_24",        register_t<size_t, size_t>(0xA018, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_25",        register_t<size_t, size_t>(0xA019, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_26",        register_t<size_t, size_t>(0xA01A, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_27",        register_t<size_t, size_t>(0xA01B, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_28",        register_t<size_t, size_t>(0xA01C, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_29",        register_t<size_t, size_t>(0xA01D, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_30",        register_t<size_t, size_t>(0xA01E, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_31",        register_t<size_t, size_t>(0xA01F, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_32",        register_t<size_t, size_t>(0xA020, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_33",        register_t<size_t, size_t>(0xA021, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_34",        register_t<size_t, size_t>(0xA022, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_35",        register_t<size_t, size_t>(0xA023, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_36",        register_t<size_t, size_t>(0xA024, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_37",        register_t<size_t, size_t>(0xA025, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_38",        register_t<size_t, size_t>(0xA026, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_39",        register_t<size_t, size_t>(0xA027, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_40",        register_t<size_t, size_t>(0xA028, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_41",        register_t<size_t, size_t>(0xA029, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_42",        register_t<size_t, size_t>(0xA02A, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_43",        register_t<size_t, size_t>(0xA02B, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_44",        register_t<size_t, size_t>(0xA02C, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_45",        register_t<size_t, size_t>(0xA02D, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_46",        register_t<size_t, size_t>(0xA02E, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_47",        register_t<size_t, size_t>(0xA02F, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_48",        register_t<size_t, size_t>(0xA030, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_49",        register_t<size_t, size_t>(0xA031, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_50",        register_t<size_t, size_t>(0xA032, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_51",        register_t<size_t, size_t>(0xA033, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_52",        register_t<size_t, size_t>(0xA034, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_53",        register_t<size_t, size_t>(0xA035, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_54",        register_t<size_t, size_t>(0xA036, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_55",        register_t<size_t, size_t>(0xA037, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_56",        register_t<size_t, size_t>(0xA038, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_57",        register_t<size_t, size_t>(0xA039, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_58",        register_t<size_t, size_t>(0xA03A, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_59",        register_t<size_t, size_t>(0xA03B, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_60",        register_t<size_t, size_t>(0xA03C, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_61",        register_t<size_t, size_t>(0xA03D, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_62",        register_t<size_t, size_t>(0xA03E, 0xFFFFFFFF, true,  false,  false)}, \
    {"px_cnt_r_63",        register_t<size_t, size_t>(0xA03F, 0xFFFFFFFF, true,  false,  false)}  \
}
  // clang-format on

} // namespace caribou
