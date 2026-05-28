#ifndef DEVICE_DSIPM_DEFAULTS_H
#define DEVICE_DSIPM_DEFAULTS_H

#include "utils/dictionary.hpp"

namespace caribou {

// define voltages
#define dSiPM_VDDIO3V3 3.3
#define dSiPM_VDDIO3V3_CURRENT 3
#define dSiPM_VDDIO1V8 1.8
#define dSiPM_VDDIO1V8_CURRENT 3
#define dSiPM_VDDCORE 1.8
#define dSiPM_VDDCORE_CURRENT 3
#define dSiPM_LED_PWR 3.5
#define dSiPM_LED_PWR_CURRENT 3
#define dSiPM_LED_ON true
#define dSiPM_T_ON true
#define dSiPM_DS_PWR 3.3
#define dSiPM_DS_PWR_CURRENT 3

#define dSiPM_VBQ 0.72      // should range 0.5 to 2.5 V
#define dSiPM_VCLAMP 2.0    // should range 1.8 to 2.5 V
#define dSiPM_VBQ_TC 0.72   // should range 1.8 to 2.5 V
#define dSiPM_VCLAMP_TC 2.0 // should range 0.5 to 2.5 V
#define dSiPM_TD_IN 100     // in uA
#define CMOS_OUT_1_TO_4_START_VOLTAGE 1.3
#define CMOS_OUT_1_TO_4_VOLTAGE 1.8
#define CMOS_OUT_5_TO_8_VOLTAGE 3.3

// define temperature offset
#define dSiPM_TEMP_OFFSET 0.0

// define temperature slope
#define dSiPM_TEMP_SLOPE 1.0

// define daq mode
#define dSiPM_TLU_DAQ 0     // so this defaults to standalone
#define dSiPM_mode_2bit 0x0 // this defaults to off
#define dSiPM_set2Bit 0x1
// outfile names
#define dSiPM_frameFilename "frameData.txt"
#define dSiPM_LEDframeFilename "LEDframeData.txt"
#define dSiPM_BINframeFilename "BINframeData.bin"
#define dSiPM_temperatureFilename "temperatureData.txt"

  // DSIPM FPGA address space
  const intptr_t DSIPM_CORE_BASE_ADDRESS = 0x43C70000;
  const intptr_t DSIPM_CORE_LSB = 2;
  const size_t DSIPM_READOUT_MAP_SIZE = 4096;
  const size_t DSIPM_WORD_SIZE = 32;

  // Assuming we start with the 32 bit for initialization settings,
  // followed by 32 x 32 bit for pixel masking (see)
  // These are offsets *relative* to the base address
  const intptr_t DSIPM_CHIP_BASE_ADDR = 0x000;
  const intptr_t DSIPM_CTRL_BASE_ADDR = 0x100;
  const intptr_t DSIPM_ROUT_BASE_ADDR = 0x200;

  // DSIPM pattern generator output signals
  // TODO, put correct values
#define DSIPM_READOUT_START 0x01
#define DSIPM_SHUTTER 0x00
#define DSIPM_LED 0x00

  // TODO, check if this is what we want
  const memory_map DSIPM_READOUT{DSIPM_CORE_BASE_ADDRESS, DSIPM_READOUT_MAP_SIZE, PROT_READ | PROT_WRITE};

  // clang-format off

#define DSIPM_MEMORY                                                                             \
  {                                                                                              \
    {"writeChipConfig",    {DSIPM_READOUT, register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 0 << DSIPM_CORE_LSB), 0x00000001,             false, true,  false)}}, \
    {"addConfClock",       {DSIPM_READOUT, register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 0 << DSIPM_CORE_LSB), 0x0000007E, 0x1       , true,  true,  false)}}, \
    {"matrixConfDirty",    {DSIPM_READOUT, register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 0 << DSIPM_CORE_LSB), 0x00000100,             true,  false, false)}}, \
    {"testConfDirty",      {DSIPM_READOUT, register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 0 << DSIPM_CORE_LSB), 0x00000200,             true,  false, false)}}, \
    {"chipConfRunning",    {DSIPM_READOUT, register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 0 << DSIPM_CORE_LSB), 0x00000400,             true,  false, false)}}, \
    {"testConfOffset",     {DSIPM_READOUT, register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 0 << DSIPM_CORE_LSB), 0x07FF0000, 0x00000420, true,  true,  false)}}, \
    {"testConfEnable",     {DSIPM_READOUT, register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 0 << DSIPM_CORE_LSB), 0x08000000, 0x1       , true,  true,  false)}}, \
    \
    {"daq_start_tlu",      {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 0 << DSIPM_CORE_LSB), 0x00000001,                 false,  true,  false)}}, \
    {"daq_start_stdalone", {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 0 << DSIPM_CORE_LSB), 0x00000002,                 false,  true,  false)}}, \
    {"daq_stop",           {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 0 << DSIPM_CORE_LSB), 0x00000004,                 false,  true,  false)}}, \
    {"mode_2bit",          {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 0 << DSIPM_CORE_LSB), 0x00000008, dSiPM_mode_2bit, true,  true,  false)}}, \
    {"daq_running",        {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 0 << DSIPM_CORE_LSB), 0x00000100, 0x0,             true,  false, false)}}, \
    {"hv_enable",          {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 1 << DSIPM_CORE_LSB), 0x00000001, 0x0,             true,  true,  false)}}, \
    {"ds_pd",              {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 1 << DSIPM_CORE_LSB), 0x00000004, 0x0,             true,  true,  false)}}, \
    {"ds_eq",              {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 1 << DSIPM_CORE_LSB), 0x00000008, 0x0,             true,  true,  false)}}, \
    {"ds_pe",              {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 1 << DSIPM_CORE_LSB), 0x00000010, 0x0,             true,  true,  false)}}, \
    {"fpga_reset",         {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 1 << DSIPM_CORE_LSB), 0x00000100,                  true,  true,  false)}}, \
    {"ext_trg_enable",     {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 2 << DSIPM_CORE_LSB), 0x00000001, 0x0,             true,  true,  false)}}, \
    {"tlu_trg_enable",     {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 2 << DSIPM_CORE_LSB), 0x00000002, 0x0,             true,  true,  false)}}, \
    {"led_trg_enable",     {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 2 << DSIPM_CORE_LSB), 0x00000004, 0x0,             true,  true,  false)}}, \
    {"auto_trg_enable",    {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 2 << DSIPM_CORE_LSB), 0x00000008, 0x0,             true,  true,  false)}}, \
    {"wait_for_fifo_empty",{DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 2 << DSIPM_CORE_LSB), 0x00000010, 0x0,             true,  true,  false)}}, \
    {"t0_enable",          {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 2 << DSIPM_CORE_LSB), 0x00000020, 0x0,             true,  true,  false)}}, \
    {"fifo_pop",           {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 2 << DSIPM_CORE_LSB), 0x00000040,                 false,  true,  false)}}, \
    {"frames_before_trg",  {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 2 << DSIPM_CORE_LSB), 0x00000700, 0x0,             true,  true,  false)}}, \
    {"frames_after_trg",   {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 2 << DSIPM_CORE_LSB), 0x00007000, 0x0,             true,  true,  false)}}, \
    {"auto_trg_spacing",   {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 2 << DSIPM_CORE_LSB), 0xFFFF0000, 0x0,             true,  true,  false)}}, \
    {"busy_time",          {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 3 << DSIPM_CORE_LSB), 0x0000FFFF, 0x0,             true,  true,  false)}}, \
    {"led_trigger",        {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 4 << DSIPM_CORE_LSB), 0x00000001, 0x0,             true,  true,  false)}}, \
    {"led_start_offset",   {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 4 << DSIPM_CORE_LSB), 0x00001FF0, 0x0,             true,  true,  false)}}, \
    {"led_num_pulses",     {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 4 << DSIPM_CORE_LSB), 0x0000C000, 0x0,             true,  true,  false)}}, \
    {"led_pulse_width",    {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 4 << DSIPM_CORE_LSB), 0x00FF0000, 0x0,             true,  true,  false)}}, \
    {"led_pulse_spacing",  {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 4 << DSIPM_CORE_LSB), 0xFF000000, 0x0,             true,  true,  false)}}, \
    {"led_frames",         {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 5 << DSIPM_CORE_LSB), 0x0000FFFF, 0x0,             true,  true,  false)}}, \
    {"led_skip_frames",    {DSIPM_READOUT, register_t<size_t>(DSIPM_CTRL_BASE_ADDR+( 5 << DSIPM_CORE_LSB), 0xFFFF0000, 0x0,             true,  true,  false)}}, \
    \
    {"rd_data_available",  {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+( 0 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_0",          {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+( 1 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_1",          {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+( 2 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_2",          {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+( 3 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_3",          {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+( 4 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_4",          {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+( 5 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_5",          {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+( 6 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_6",          {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+( 7 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_7",          {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+( 8 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_8",          {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+( 9 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_9",          {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(10 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_10",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(11 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_11",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(12 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_12",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(13 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_13",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(14 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_14",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(15 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_15",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(16 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_16",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(17 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_17",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(18 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_18",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(19 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_19",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(20 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_20",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(21 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_21",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(22 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_22",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(23 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_23",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(24 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_24",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(25 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_25",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(26 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_26",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(27 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_27",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(28 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_28",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(29 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_29",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(30 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_30",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(31 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_31",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(32 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_32",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(33 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_33",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(34 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_34",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(35 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_35",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(36 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_36",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(37 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_37",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(38 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_38",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(39 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_39",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(40 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_40",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(41 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_41",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(42 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}}, \
    {"rd_data_42",         {DSIPM_READOUT, register_t<size_t>(DSIPM_ROUT_BASE_ADDR+(43 << DSIPM_CORE_LSB), 0xFFFFFFFF, true, false, false)}} \
  }

  // clang-format on

  /** Dictionary for register address/name lookup for DSIPM
   */

  // clang-format off

#define DSIPM_REGISTERS                                                                                                     \
  {                                                                                                                         \
    {"OpenCntrBits_p1",    register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 1 << DSIPM_CORE_LSB), 0x0000000F, 0x0           , true,  true,  false)}, \
    {"validCntr_Q1",       register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 1 << DSIPM_CORE_LSB), 0x000000F0, 0x0           , true,  true,  false)}, \
    {"TDC_Q1",             register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 1 << DSIPM_CORE_LSB), 0x00000300, 0x2           , true,  true,  false)}, \
    {"validCntr_Q3",       register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 1 << DSIPM_CORE_LSB), 0x00003C00, 0x0           , true,  true,  false)}, \
    {"TDC_Q3",             register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 1 << DSIPM_CORE_LSB), 0x0000C000, 0x2           , true,  true,  false)}, \
    {"set2Bit",            register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 1 << DSIPM_CORE_LSB), 0x00010000, dSiPM_set2Bit , true,  true,  false)}, \
    {"validCntr_Q2",       register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 1 << DSIPM_CORE_LSB), 0x001E0000, 0x0           , true,  true,  false)}, \
    {"TDC_Q2",             register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 1 << DSIPM_CORE_LSB), 0x00600000, 0x2           , true,  true,  false)}, \
    {"validCntr_Q4",       register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 1 << DSIPM_CORE_LSB), 0x07800000, 0x0           , true,  true,  false)}, \
    {"TDC_Q4",             register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 1 << DSIPM_CORE_LSB), 0x18000000, 0x2           , true,  true,  false)}, \
    {"OpenCntrBits_p2",    register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 1 << DSIPM_CORE_LSB), 0xE0000000, 0x0           , true,  true,  false)}, \
    {"pixelEnableRow0",    register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 2 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow1",    register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 3 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow2",    register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 4 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow3",    register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 5 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow4",    register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 6 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow5",    register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 7 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow6",    register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 8 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow7",    register_t<size_t>(DSIPM_CHIP_BASE_ADDR+( 9 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow8",    register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(10 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow9",    register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(11 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow10",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(12 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow11",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(13 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow12",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(14 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow13",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(15 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow14",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(16 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow15",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(17 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow16",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(18 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow17",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(19 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow18",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(20 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow19",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(21 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow20",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(22 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow21",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(23 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow22",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(24 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow23",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(25 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow24",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(26 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow25",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(27 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow26",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(28 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow27",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(29 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow28",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(30 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow29",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(31 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow30",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(32 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"pixelEnableRow31",   register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(33 << DSIPM_CORE_LSB), 0xFFFFFFFF, 0xFFFFFFFF, true,  true,  false)}, \
    {"quadrantsEnable",    register_t<size_t>(0,                                           0x00000000, 0x0000000F, true,  true,   true)}, \
    {"pixelEnable_TC",     register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(34 << DSIPM_CORE_LSB), 0x000001FF, 0x00000000, true,  true,  false)}, \
    {"pixelEnableWOFE_TC", register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(34 << DSIPM_CORE_LSB), 0x0001FE00, 0x00000000, true,  true,  false)}, \
    {"TDC_TC",             register_t<size_t>(DSIPM_CHIP_BASE_ADDR+(34 << DSIPM_CORE_LSB), 0x00060000, 0x2       , true,  true,  false)}, \
}

  // clang-format on

} // namespace caribou

#endif /* DEVICE_DSIPM_DEFAULTS_H */
