#ifndef DEVICE_C1004_DEFAULTS_H
#define DEVICE_C1004_DEFAULTS_H

#include "utils/dictionary.hpp"

namespace caribou {

/** Default device path for this device: SPI interface
 */
#define DEFAULT_SCIPATH "/dev/spidev1.0"
#define DEFAULT_DRIPATH "/dev/media0"
#define DEFAULT_DRI_MEDIA_OUTPUT_ENTITY "\"falcon_video output 1\" [fmt:Y16/32x60]"

  const std::string MEDIA_BALANCE_LINKS("\"a0010000.axis_switch\":2 -> \"a0020000.axis_switch\":0 [1] , "
                                        "\"a0020000.axis_switch\":2 -> \"a0060000.singleBalance_proc\":0 [1] , "
                                        "\"a0060000.singleBalance_proc\":1 -> \"a0030000.axis_switch\":2 [1] , "
                                        "\"a0030000.axis_switch\":4 -> \"falcon_video output 1\":0 [1]");

  const std::string MEDIA_BALANCE_ROUTES("\"a0010000.axis_switch\" 0 -> 2 , "
                                         "\"a0020000.axis_switch\" 0 -> 2 , "
                                         "\"a0030000.axis_switch\" 2 -> 4");

  const std::string MEDIA_INTENSITY_LINKS("\"a0010000.axis_switch\":2 -> \"a0020000.axis_switch\":0 [1] , "
                                          "\"a0020000.axis_switch\":1 -> \"a0050000.intensity_proc\":0 [1] , "
                                          "\"a0050000.intensity_proc\":1 -> \"a0030000.axis_switch\":1 [1] , "
                                          "\"a0030000.axis_switch\":4 -> \"falcon_video output 1\":0 [1]");

  const std::string MEDIA_INTENSITY_ROUTES("\"a0010000.axis_switch\" 0 -> 2 , "
                                           "\"a0020000.axis_switch\" 0 -> 1 , "
                                           "\"a0030000.axis_switch\" 1 -> 4");

  const std::string MEDIA_RAWTIMESTAMPS_LINKS("\"a0010000.axis_switch\":1 -> \"a0040000.rawData_proc\":0 [1] , "
                                              "\"a0040000.rawData_proc\":1 -> \"a0030000.axis_switch\":0 [1] , "
                                              "\"a0030000.axis_switch\":4 -> \"falcon_video output 1\":0 [1]");

  const std::string MEDIA_RAWTIMESTAMPS_ROUTES("\"a0010000.axis_switch\" 0 -> 1 , "
                                               "\"a0030000.axis_switch\" 0 -> 4");

  const std::string MEDIA_USER_LINKS("\"a0010000.axis_switch\":2 -> \"a0020000.axis_switch\":0 [1] , "
                                     "\"a0020000.axis_switch\":3 -> \"a0070000.user_proc\":0 [1] , "
                                     "\"a0070000.user_proc\":1 -> \"a0030000.axis_switch\":3 [1] , "
                                     "\"a0030000.axis_switch\":4 -> \"falcon_video output 1\":0 [1]");

  const std::string MEDIA_USER_ROUTES("\"a0010000.axis_switch\" 0 -> 2 , "
                                      "\"a0020000.axis_switch\" 0 -> 3 , "
                                      "\"a0030000.axis_switch\" 3 -> 4");

  const std::string MEDIA_FORMATS("\"a0010000.axis_switch\":0 [fmt:FIXED/32x60] , "
                                  "\"a0020000.axis_switch\":0 [fmt:FIXED/32x60] , "
                                  "\"a0030000.axis_switch\":0 [fmt:Y16/32x60] , "
                                  "\"a0030000.axis_switch\":1 [fmt:Y16/32x60] , "
                                  "\"a0030000.axis_switch\":2 [fmt:Y16/32x60] , "
                                  "\"a0030000.axis_switch\":3 [fmt:Y16/32x60] , "
                                  "\"a0040000.rawData_proc\":1 [fmt:Y16/32x60] , "
                                  "\"a0040000.rawData_proc\":0 [fmt:FIXED/32x60] , "
                                  "\"a0050000.intensity_proc\":1 [fmt:Y16/32x60] , "
                                  "\"a0050000.intensity_proc\":0 [fmt:FIXED/32x60] , "
                                  "\"a0060000.singleBalance_proc\":1 [fmt:Y16/32x60] , "
                                  "\"a0060000.singleBalance_proc\":0 [fmt:FIXED/32x60] , "
                                  "\"a0070000.user_proc\":1 [fmt:Y16/32x60] , "
                                  "\"a0070000.user_proc\":0 [fmt:FIXED/32x60]");

  const std::string dataProcessorSubDeviceNaming[] = {
    "a0060000.singleBalance_proc", "a0070000.user_proc", "a0050000.intensity_proc", "a0040000.rawData_proc"};

#define C1004_VBPEXT_TDC 1.8
#define C1004_VBPEXT_VCO 1.05
#define C1004_TDC_REF 1.144
#define C1004_VEB 1.8
#define C1004_VQ 0.0
#define C1004_VPU 0.8
#define C1004_VWIN 1.0
#define C1004_VOP 26.030
#define C1004_VCO_BIAS 0.0

  // C1004 control
  const std::uintptr_t C1004_CONTROL_BASE_ADDRESS = 0xA0001000;
  const std::uintptr_t C1004_RESET_OFFSET = 0x00 << 3;
  const std::uintptr_t C1004_CLOCK_OFFSET = 0x01 << 3;
  const std::uintptr_t C1004_DEBUG_MODE_OFFSET = 0x02 << 3;
  const std::uintptr_t C1004_PIXEL_ARRAY_EN_OFFSET = 0x03 << 3;
  const std::uintptr_t C1004_SEQUENCER_EN_OFFSET = 0x04 << 3;
  const std::uintptr_t C1004_SEQUENCER_DIRECT_CONTROL_OFFSET = 0x05 << 3;
  const std::uintptr_t C1004_SEQUENCER_REPETITION_RATE_OFFSET = 0x06 << 3;
  const std::uintptr_t C1004_SEQUENCER_DURATION_OFFSET = 0x07 << 3;
  const std::uintptr_t C1004_SEQUENCER_LATCH_DURATION_OFFSET = 0x08 << 3;
  const std::uintptr_t C1004_SEQUENCER_GATING_OFFSET = 0x09 << 3;
  const std::uintptr_t C1004_SEQUENCER_FLISS_EN_OFFSET = 0x0a << 3;
  const std::uintptr_t C1004_SEQUENCER_FLISS_POLYNOMIAL_OFFSET = 0x0b << 3;
  const std::uintptr_t C1004_SEQUENCER_FLISS_MASK_OFFSET = 0x0c << 3;
  const std::uintptr_t C1004_SEQUENCER_CLEAR_VCO_EN_GAP_OFFSET = 0x0d << 3;
  const std::uintptr_t C1004_LASER_HV_ENABLE_OFFSET = 0x0e << 3;
  const std::uintptr_t C1004_LASER_HV_STATUS_OFFSET = 0x0f << 3;

  const uint64_t C1004_CONTROL_RST_MASK = 0x1;
  const uint64_t C1004_CONTROL_RST_BYPASS_MASK = 0x2;
  const uint64_t C1004_CLOCK_SOURCE_MASK = 0x1;
  const uint64_t C1004_CLOCK_CLOCK_NOT_PRESENT_MASK = 0x40;
  const uint64_t C1004_CLOCK_CLOCK_LOCKED_MASK = 0x80;
  const uint64_t C1004_CONTROL_DEBUG_MODE_MASK = 0x1;
  const uint64_t C1004_CONTROL_DEBUG_REF_TDC_TRIG_WINDOW_MODE_MASK = 0x2;
  const uint64_t C1004_CONTROL_DEBUG_REF_TDC_TRIG_EN_MASK = 0x4;
  const uint64_t C1004_CONTROL_PIXEL_ARRAY_EN_MASK = 0x1;
  const uint64_t C1004_SEQUENCER_EN_MASK = 0x1;
  const uint64_t C1004_SEQUENCER_DIRECT_CONTROL_MASK = 0x1;
  const uint64_t C1004_SEQUENCER_REPETITION_RATE_MASK = 0xFFFFFFFF;
  const uint64_t C1004_SEQUENCER_DURATION_MASK = 0xFFFFFFFF;
  const uint64_t C1004_SEQUENCER_LATCH_DURATION_MASK = 0x00000007;
  const uint64_t C1004_SEQUENCER_GATING_MASK = 0xFFFFFFFF;
  const uint64_t C1004_SEQUENCER_FLISS_EN_MASK = 0x1;
  const uint64_t C1004_SEQUENCER_FLISS_POLYNOMIAL_MASK = 0xFFFFFFFF;
  const uint64_t C1004_SEQUENCER_FLISS_MAKS_MASK = 0xFFFFFFFF;
  const uint64_t C1004_SEQUENCER_CLEAR_VCO_EN_GAP_MASK = 0x1;
  const uint64_t C1004_LASER_HV_ENABLE_MASK = 0x1;
  const uint64_t C1004_LASER_HV_STATUS_MASK = 0x1;

  const std::size_t C1004_CONTROL_MAP_SIZE = 4096;

  const double C1004_SEQUENCER_CLOCK_PERIOD_NS = 12.5;

  const memory_map C1004_MEM_CONTROL_RW =
    memory_map(C1004_CONTROL_BASE_ADDRESS, C1004_CONTROL_MAP_SIZE, PROT_READ | PROT_WRITE);
  const memory_map C1004_MEM_CONTROL_RO = memory_map(C1004_CONTROL_BASE_ADDRESS, C1004_CONTROL_MAP_SIZE, PROT_READ);

// clang-format off
#define C1004_MEMORY                                                                                                        \
  {                                                                                                                         \
      {"reset",                                                                                                             \
       {C1004_MEM_CONTROL_RW,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_RESET_OFFSET, C1004_CONTROL_RST_MASK)}},                           \
      {"reset_bypass",                                                                                                      \
       {C1004_MEM_CONTROL_RW,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_RESET_OFFSET, C1004_CONTROL_RST_BYPASS_MASK, 0)}},                 \
      {"clock_source",                                                                                                      \
       {C1004_MEM_CONTROL_RW,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_CLOCK_OFFSET, C1004_CLOCK_SOURCE_MASK)}},                          \
      {"clock_not_present",                                                                                                 \
       {C1004_MEM_CONTROL_RO,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_CLOCK_OFFSET, C1004_CLOCK_CLOCK_NOT_PRESENT_MASK)}},               \
      {"clock_locked",                                                                                                      \
       {C1004_MEM_CONTROL_RO,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_CLOCK_OFFSET, C1004_CLOCK_CLOCK_LOCKED_MASK)}},                    \
      {"debug_mode",                                                                                                        \
       {C1004_MEM_CONTROL_RW,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_DEBUG_MODE_OFFSET, C1004_CONTROL_DEBUG_MODE_MASK)}},               \
      {"ref_tdc_trig_window_mode",                                                                                          \
       {C1004_MEM_CONTROL_RW,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_DEBUG_MODE_OFFSET,                                                 \
                                                   C1004_CONTROL_DEBUG_REF_TDC_TRIG_WINDOW_MODE_MASK)}},                    \
      {"ref_tdc_trig_en",                                                                                                   \
       {C1004_MEM_CONTROL_RW,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_DEBUG_MODE_OFFSET, C1004_CONTROL_DEBUG_REF_TDC_TRIG_EN_MASK)}},    \
      {"pixel_array_en",                                                                                                    \
       {C1004_MEM_CONTROL_RW,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_PIXEL_ARRAY_EN_OFFSET, C1004_CONTROL_PIXEL_ARRAY_EN_MASK)}},       \
      {"sequencer_en",                                                                                                      \
       {C1004_MEM_CONTROL_RO,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_SEQUENCER_EN_OFFSET, C1004_SEQUENCER_EN_MASK)}},                   \
      {"sequencer_direct_control",                                                                                          \
       {C1004_MEM_CONTROL_RO,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_SEQUENCER_DIRECT_CONTROL_OFFSET,                                   \
                                                   C1004_SEQUENCER_DIRECT_CONTROL_MASK)}},                                  \
      {"sequencer_repetition_rate",                                                                                         \
       {C1004_MEM_CONTROL_RO,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_SEQUENCER_REPETITION_RATE_OFFSET,                                   \
                                                   C1004_SEQUENCER_REPETITION_RATE_MASK)}},                                  \
      {"sequencer_duration",                                                                                                \
       {C1004_MEM_CONTROL_RO,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_SEQUENCER_DURATION_OFFSET, C1004_SEQUENCER_DURATION_MASK)}},         \
      {"sequencer_latch_duration",                                                                                          \
       {C1004_MEM_CONTROL_RO,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_SEQUENCER_LATCH_DURATION_OFFSET,                                    \
                                                   C1004_SEQUENCER_LATCH_DURATION_MASK)}},                                   \
      {"gating",                                                                                          \
       {C1004_MEM_CONTROL_RW,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_SEQUENCER_GATING_OFFSET,                                    \
                                                   C1004_SEQUENCER_GATING_MASK)}},                                   \
      {"fliss_en",                                                                                                          \
       {C1004_MEM_CONTROL_RW,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_SEQUENCER_FLISS_EN_OFFSET, C1004_SEQUENCER_FLISS_EN_MASK)}},         \
      {"fliss_polynomial",                                                                                                  \
       {C1004_MEM_CONTROL_RW,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_SEQUENCER_FLISS_POLYNOMIAL_OFFSET,                                  \
						   C1004_SEQUENCER_FLISS_POLYNOMIAL_MASK)}},	                            \
      {"fliss_mask",                                                                                                        \
       {C1004_MEM_CONTROL_RW,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_SEQUENCER_FLISS_MASK_OFFSET, C1004_SEQUENCER_FLISS_MAKS_MASK)}},     \
      {"sequencer_clear_vco_en_gap",                                                                                        \
       {C1004_MEM_CONTROL_RW,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_SEQUENCER_CLEAR_VCO_EN_GAP_OFFSET,                                  \
						   C1004_SEQUENCER_CLEAR_VCO_EN_GAP_MASK)}},                                 \
      {"laser_en",                                                                                                          \
       {C1004_MEM_CONTROL_RO,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_LASER_HV_ENABLE_OFFSET, C1004_LASER_HV_ENABLE_MASK)}},             \
      {"laser_status",                                                                                                      \
       {C1004_MEM_CONTROL_RO,                                                                                               \
        register_t<std::uintptr_t, std::uintptr_t>(C1004_LASER_HV_STATUS_OFFSET, C1004_LASER_HV_STATUS_MASK)}},             \
  }
  // clang-format on

  // The map contains the safe repetition rate in MHz (value) for the given voltage (key)
  const std::map<unsigned int, double> SAFE_REPETITION_RATE{
    {10, 10.909}, {11, 9.016}, {12, 7.576}, {13, 6.455}, {14, 5.566}, {15, 4.848}, {16, 4.261}, {17, 3.775}, {18, 3.367},
    {19, 3.022},  {20, 2.727}, {21, 2.474}, {22, 2.254}, {23, 2.062}, {24, 1.894}, {25, 1.745}, {26, 1.614}, {27, 1.496},
    {28, 1.391},  {29, 1.297}, {30, 1.212}, {31, 1.135}, {32, 1.065}, {33, 1.002}, {34, 0.944}, {35, 0.891}, {36, 0.842},
    {37, 0.797},  {38, 0.755}, {39, 0.717}, {40, 0.682}, {41, 0.649}, {42, 0.618}, {43, 0.590}, {44, 0.563}, {45, 0.539},
    {46, 0.516},  {47, 0.494}, {48, 0.473}, {49, 0.454}, {50, 0.436}, {51, 0.419}, {52, 0.403}, {53, 0.388}, {54, 0.374},
    {55, 0.361},  {56, 0.348}, {57, 0.336}, {58, 0.324}, {59, 0.313}, {60, 0.303}, {61, 0.293}, {62, 0.284}, {63, 0.275},
    {64, 0.266},  {65, 0.258}, {66, 0.250}, {67, 0.243}, {68, 0.236}, {69, 0.229}, {70, 0.223}, {71, 0.216}, {72, 0.210},
    {73, 0.205},  {74, 0.199}, {75, 0.194}, {76, 0.189}, {77, 0.184}, {78, 0.179}, {79, 0.175}, {80, 0.170}};

// clang-format off
#define C1004_REGISTERS		           		                   \
  {							                   \
    {"chip_id", register_t<>(0x00, 0xFF, true, false)},	                   \
    {"mask_prg", register_t<>(0x01, 0xFF, false, true)},                   \
    {"rmask_len", register_t<>(0x02)},	                                   \
    {"swbias_vco", register_t<>(0x03, 0x0F)},	                           \
    {"swbias_vcobuff", register_t<>(0x04, 0x03)},	                   \
    {"swbias_tdc", register_t<>(0x05, 0x0F)},	                           \
    {"seq_seq_en", register_t<>(0x06, 0x80)},	                           \
    {"seq_latch_duration", register_t<>(0x06, 0x07)},	                   \
    {"csel", register_t<>(0x07, 0x03)},	                                   \
    {"sw_rst", register_t<>(0x08, 0xFF, false, true)},	                   \
    {"ren_lsb", register_t<>(0x09, 0xFF)},	                           \
    {"ren_msb", register_t<>(0x0A, 0x03)},	                           \
    {"ren", register_t<>(0x09, 0xFF, true, true, true)},                   \
    {"mask_n_0", register_t<>(11, 0xFF)},                                \
    {"mask_n_1", register_t<>(12, 0xFF)},                                \
    {"mask_n_2", register_t<>(13, 0xFF)},                                \
    {"mask_n_3", register_t<>(14, 0xFF)},                                \
    {"mask_n_4", register_t<>(15, 0xFF)},                                \
    {"mask_n_5", register_t<>(16, 0xFF)},                                \
    {"mask_n_6", register_t<>(17, 0xFF)},                                \
    {"mask_n_7", register_t<>(18, 0xFF)},                                \
    {"mask_n_8", register_t<>(19, 0xFF)},                                \
    {"mask_n_9", register_t<>(20, 0xFF)},                                \
    {"mask_n_10", register_t<>(21, 0xFF)},                               \
    {"mask_n_11", register_t<>(22, 0xFF)},                               \
    {"mask_n_12", register_t<>(23, 0xFF)},                               \
    {"mask_n_13", register_t<>(24, 0xFF)},                               \
    {"mask_n_14", register_t<>(25, 0xFF)},                               \
    {"mask_n_15", register_t<>(26, 0xFF)},                               \
    {"mask_n_16", register_t<>(27, 0xFF)},                               \
    {"mask_n_17", register_t<>(28, 0xFF)},                               \
    {"mask_n_18", register_t<>(29, 0xFF)},                               \
    {"mask_n_19", register_t<>(30, 0xFF)},                               \
    {"mask_n_20", register_t<>(31, 0xFF)},                               \
    {"mask_n_21", register_t<>(32, 0xFF)},                               \
    {"mask_n_22", register_t<>(33, 0xFF)},                               \
    {"mask_n_23", register_t<>(34, 0xFF)},                               \
    {"mask_n_24", register_t<>(35, 0xFF)},                               \
    {"mask_n_25", register_t<>(36, 0xFF)},                               \
    {"mask_n_26", register_t<>(37, 0xFF)},                               \
    {"mask_n_27", register_t<>(38, 0xFF)},                               \
    {"mask_n_28", register_t<>(39, 0xFF)},                               \
    {"mask_n_29", register_t<>(40, 0xFF)},                               \
    {"mask_n_30", register_t<>(41, 0xFF)},                               \
    {"mask_n_31", register_t<>(42, 0xFF)},                               \
    {"mask_n_32", register_t<>(43, 0xFF)},                               \
    {"mask_n_33", register_t<>(44, 0xFF)},                               \
    {"mask_n_34", register_t<>(45, 0xFF)},                               \
    {"mask_n_35", register_t<>(46, 0xFF)},                               \
    {"mask_n_36", register_t<>(47, 0xFF)},                               \
    {"mask_n_37", register_t<>(48, 0xFF)},                               \
    {"mask_n_38", register_t<>(49, 0xFF)},                               \
    {"mask_n_39", register_t<>(50, 0xFF)},                               \
    {"mask_n_40", register_t<>(51, 0xFF)},                               \
    {"mask_n_41", register_t<>(52, 0xFF)},                               \
    {"mask_n_42", register_t<>(53, 0xFF)},                               \
    {"mask_n_43", register_t<>(54, 0xFF)},                               \
    {"mask_n_44", register_t<>(55, 0xFF)},                               \
    {"mask_n_45", register_t<>(56, 0xFF)},                               \
    {"mask_n_46", register_t<>(57, 0xFF)},                               \
    {"mask_n_47", register_t<>(58, 0xFF)},                               \
    {"mask_n_48", register_t<>(59, 0xFF)},                               \
    {"mask_n_49", register_t<>(60, 0xFF)},                               \
    {"mask_n_50", register_t<>(61, 0xFF)},                               \
    {"mask_n_51", register_t<>(62, 0xFF)},                               \
    {"mask_n_52", register_t<>(63, 0xFF)},                               \
    {"mask_n_53", register_t<>(64, 0xFF)},                               \
    {"mask_n_54", register_t<>(65, 0xFF)},                               \
    {"mask_n_55", register_t<>(66, 0xFF)},                               \
    {"mask_n_56", register_t<>(67, 0xFF)},                               \
    {"mask_n_57", register_t<>(68, 0xFF)},                               \
    {"mask_n_58", register_t<>(69, 0xFF)},                               \
    {"mask_n_59", register_t<>(70, 0xFF)},                               \
    {"mask_n_60", register_t<>(71, 0xFF)},                               \
    {"mask_n_61", register_t<>(72, 0xFF)},                               \
    {"mask_n_62", register_t<>(73, 0xFF)},                               \
    {"mask_n_63", register_t<>(74, 0xFF)},                               \
    {"mask_n_64", register_t<>(75, 0xFF)},                               \
    {"mask_n_65", register_t<>(76, 0xFF)},                               \
    {"mask_n_66", register_t<>(77, 0xFF)},                               \
    {"mask_n_67", register_t<>(78, 0xFF)},                               \
    {"mask_n_68", register_t<>(79, 0xFF)},                               \
    {"mask_n_69", register_t<>(80, 0xFF)},                               \
    {"mask_n_70", register_t<>(81, 0xFF)},                               \
    {"mask_n_71", register_t<>(82, 0xFF)},                               \
    {"mask_n_72", register_t<>(83, 0xFF)},                               \
    {"mask_n_73", register_t<>(84, 0xFF)},                               \
    {"mask_n_74", register_t<>(85, 0xFF)},                               \
    {"mask_n_75", register_t<>(86, 0xFF)},                               \
    {"mask_n_76", register_t<>(87, 0xFF)},                               \
    {"mask_n_77", register_t<>(88, 0xFF)},                               \
    {"mask_n_78", register_t<>(89, 0xFF)},                               \
    {"mask_n_79", register_t<>(90, 0xFF)},                               \
    {"mask_n_80", register_t<>(91, 0xFF)},                               \
    {"mask_n_81", register_t<>(92, 0xFF)},                               \
    {"mask_n_82", register_t<>(93, 0xFF)},                               \
    {"mask_n_83", register_t<>(94, 0xFF)},                               \
    {"mask_n_84", register_t<>(95, 0xFF)},                               \
    {"mask_n_85", register_t<>(96, 0xFF)},                               \
    {"mask_n_86", register_t<>(97, 0xFF)},                               \
    {"mask_n_87", register_t<>(98, 0xFF)},                               \
    {"mask_n_88", register_t<>(99, 0xFF)},                               \
    {"mask_n_89", register_t<>(100, 0xFF)},                               \
    {"mask_n_90", register_t<>(101, 0xFF)},                               \
    {"mask_n_91", register_t<>(102, 0xFF)},                               \
    {"mask_n_92", register_t<>(103, 0xFF)},                               \
    {"mask_n_93", register_t<>(104, 0xFF)},                               \
    {"mask_n_94", register_t<>(105, 0xFF)},                               \
    {"mask_n_95", register_t<>(106, 0xFF)},                               \
    {"mask_n_96", register_t<>(107, 0xFF)},                               \
    {"mask_n_97", register_t<>(108, 0xFF)},                               \
    {"mask_n_98", register_t<>(109, 0xFF)},                               \
    {"mask_n_99", register_t<>(110, 0xFF)},                               \
    {"mask_n_100", register_t<>(111, 0xFF)},                              \
    {"mask_n_101", register_t<>(112, 0xFF)},                              \
    {"mask_n_102", register_t<>(113, 0xFF)},                              \
    {"mask_n_103", register_t<>(114, 0xFF)},                              \
    {"mask_n_104", register_t<>(115, 0xFF)},                              \
    {"mask_n_105", register_t<>(116, 0xFF)},                              \
    {"mask_n_106", register_t<>(117, 0xFF)},                              \
    {"mask_n_107", register_t<>(118, 0xFF)},                              \
    {"mask_n_108", register_t<>(119, 0xFF)},                              \
    {"mask_n_109", register_t<>(120, 0xFF)},                              \
    {"mask_n_110", register_t<>(121, 0xFF)},                              \
    {"mask_n_111", register_t<>(122, 0xFF)},                              \
    {"mask_n_112", register_t<>(123, 0xFF)},                              \
    {"mask_n_113", register_t<>(124, 0xFF)},                              \
    {"mask_n_114", register_t<>(125, 0xFF)},                              \
    {"mask_n_115", register_t<>(126, 0xFF)},                              \
    {"mask_n_116", register_t<>(127, 0xFF)},                              \
    {"mask_n_117", register_t<>(128, 0xFF)},                              \
    {"mask_n_118", register_t<>(129, 0xFF)},                              \
    {"mask_n_119", register_t<>(130, 0xFF)},                              \
    {"mask_n_120", register_t<>(131, 0xFF)},                              \
    {"mask_n_121", register_t<>(132, 0xFF)},                              \
    {"mask_n_122", register_t<>(133, 0xFF)},                              \
    {"mask_n_123", register_t<>(134, 0xFF)},                              \
    {"mask_n_124", register_t<>(135, 0xFF)},                              \
    {"mask_n_125", register_t<>(136, 0xFF)},                              \
    {"mask_n_126", register_t<>(137, 0xFF)},                              \
    {"mask_n_127", register_t<>(138, 0xFF)},                              \
    {"mask_n_128", register_t<>(139, 0xFF)},                              \
    {"mask_n_129", register_t<>(140, 0xFF)},                              \
    {"mask_n_130", register_t<>(141, 0xFF)},                              \
    {"mask_n_131", register_t<>(142, 0xFF)},                              \
    {"mask_n_132", register_t<>(143, 0xFF)},                              \
    {"mask_n_133", register_t<>(144, 0xFF)},                              \
    {"mask_n_134", register_t<>(145, 0xFF)},                              \
    {"mask_n_135", register_t<>(146, 0xFF)},                              \
    {"mask_n_136", register_t<>(147, 0xFF)},                              \
    {"mask_n_137", register_t<>(148, 0xFF)},                              \
    {"mask_n_138", register_t<>(149, 0xFF)},                              \
    {"mask_n_139", register_t<>(150, 0xFF)},                              \
    {"mask_n_140", register_t<>(151, 0xFF)},                              \
    {"mask_n_141", register_t<>(152, 0xFF)},                              \
    {"mask_n_142", register_t<>(153, 0xFF)},                              \
    {"mask_n_143", register_t<>(154, 0xFF)},                              \
    {"mask_n_144", register_t<>(155, 0xFF)},                              \
    {"mask_n_145", register_t<>(156, 0xFF)},                              \
    {"mask_n_146", register_t<>(157, 0xFF)},                              \
    {"mask_n_147", register_t<>(158, 0xFF)},                              \
    {"mask_n_148", register_t<>(159, 0xFF)},                              \
    {"mask_n_149", register_t<>(160, 0xFF)},                              \
    {"mask_n_150", register_t<>(161, 0xFF)},                              \
    {"mask_n_151", register_t<>(162, 0xFF)},                              \
    {"mask_n_152", register_t<>(163, 0xFF)},                              \
    {"mask_n_153", register_t<>(164, 0xFF)},                              \
    {"mask_n_154", register_t<>(165, 0xFF)},                              \
    {"mask_n_155", register_t<>(166, 0xFF)},                              \
    {"mask_n_156", register_t<>(167, 0xFF)},                              \
    {"mask_n_157", register_t<>(168, 0xFF)},                              \
    {"mask_n_158", register_t<>(169, 0xFF)},                              \
    {"mask_n_159", register_t<>(170, 0xFF)},                              \
    {"mask_n_160", register_t<>(171, 0xFF)},                              \
    {"mask_n_161", register_t<>(172, 0xFF)},                              \
    {"mask_n_162", register_t<>(173, 0xFF)},                              \
    {"mask_n_163", register_t<>(174, 0xFF)},                              \
    {"mask_n_164", register_t<>(175, 0xFF)},                              \
    {"mask_n_165", register_t<>(176, 0xFF)},                              \
    {"mask_n_166", register_t<>(177, 0xFF)},                              \
    {"mask_n_167", register_t<>(178, 0xFF)},                              \
    {"mask_n_168", register_t<>(179, 0xFF)},                              \
    {"mask_n_169", register_t<>(180, 0xFF)},                              \
    {"mask_n_170", register_t<>(181, 0xFF)},                              \
    {"mask_n_171", register_t<>(182, 0xFF)},                              \
    {"mask_n_172", register_t<>(183, 0xFF)},                              \
    {"mask_n_173", register_t<>(184, 0xFF)},                              \
    {"mask_n_174", register_t<>(185, 0xFF)},                              \
    {"mask_n_175", register_t<>(186, 0xFF)},                              \
    {"mask_n_176", register_t<>(187, 0xFF)},                              \
    {"mask_n_177", register_t<>(188, 0xFF)},                              \
    {"mask_n_178", register_t<>(189, 0xFF)},                              \
    {"mask_n_179", register_t<>(190, 0xFF)},                              \
    {"mask_n_180", register_t<>(191, 0xFF)},                              \
    {"mask_n_181", register_t<>(192, 0xFF)},                              \
    {"mask_n_182", register_t<>(193, 0xFF)},                              \
    {"mask_n_183", register_t<>(194, 0xFF)},                              \
    {"mask_n_184", register_t<>(195, 0xFF)},                              \
    {"mask_n_185", register_t<>(196, 0xFF)},                              \
    {"mask_n_186", register_t<>(197, 0xFF)},                              \
    {"mask_n_187", register_t<>(198, 0xFF)},                              \
    {"mask_n_188", register_t<>(199, 0xFF)},                              \
    {"mask_n_189", register_t<>(200, 0xFF)},                              \
    {"mask_n_190", register_t<>(201, 0xFF)},                              \
    {"mask_n_191", register_t<>(202, 0xFF)},                              \
    {"mask_n_192", register_t<>(203, 0xFF)},                              \
    {"mask_n_193", register_t<>(204, 0xFF)},                              \
    {"mask_n_194", register_t<>(205, 0xFF)},                              \
    {"mask_n_195", register_t<>(206, 0xFF)},                              \
    {"mask_n_196", register_t<>(207, 0xFF)},                              \
    {"mask_n_197", register_t<>(208, 0xFF)},                              \
    {"mask_n_198", register_t<>(209, 0xFF)},                              \
    {"mask_n_199", register_t<>(210, 0xFF)},                              \
    {"mask_n_200", register_t<>(211, 0xFF)},                              \
    {"mask_n_201", register_t<>(212, 0xFF)},                              \
    {"mask_n_202", register_t<>(213, 0xFF)},                              \
    {"mask_n_203", register_t<>(214, 0xFF)},                              \
    {"mask_n_204", register_t<>(215, 0xFF)},                              \
    {"mask_n_205", register_t<>(216, 0xFF)},                              \
    {"mask_n_206", register_t<>(217, 0xFF)},                              \
    {"mask_n_207", register_t<>(218, 0xFF)},                              \
    {"mask_n_208", register_t<>(219, 0xFF)},                              \
    {"mask_n_209", register_t<>(220, 0xFF)},                              \
    {"mask_n_210", register_t<>(221, 0xFF)},                              \
    {"mask_n_211", register_t<>(222, 0xFF)},                              \
    {"mask_n_212", register_t<>(223, 0xFF)},                              \
    {"mask_n_213", register_t<>(224, 0xFF)},                              \
    {"mask_n_214", register_t<>(225, 0xFF)},                              \
    {"mask_n_215", register_t<>(226, 0xFF)},                              \
    {"mask_n_216", register_t<>(227, 0xFF)},                              \
    {"mask_n_217", register_t<>(228, 0xFF)},                              \
    {"mask_n_218", register_t<>(229, 0xFF)},                              \
    {"mask_n_219", register_t<>(230, 0xFF)},                              \
    {"mask_n_220", register_t<>(231, 0xFF)},                              \
    {"mask_n_221", register_t<>(232, 0xFF)},                              \
    {"mask_n_222", register_t<>(233, 0xFF)},                              \
    {"mask_n_223", register_t<>(234, 0xFF)},                              \
    {"mask_n_224", register_t<>(235, 0xFF)},                              \
    {"mask_n_225", register_t<>(236, 0xFF)},                              \
    {"mask_n_226", register_t<>(237, 0xFF)},                              \
    {"mask_n_227", register_t<>(238, 0xFF)},                              \
    {"mask_n_228", register_t<>(239, 0xFF)},                              \
    {"mask_n_229", register_t<>(240, 0xFF)},                              \
    {"mask_n_230", register_t<>(241, 0xFF)},                              \
    {"mask_n_231", register_t<>(242, 0xFF)},                              \
    {"mask_n_232", register_t<>(243, 0xFF)},                              \
    {"mask_n_233", register_t<>(244, 0xFF)},                              \
    {"mask_n_234", register_t<>(245, 0xFF)},                              \
    {"mask_n_235", register_t<>(246, 0xFF)},                              \
    {"mask_n_236", register_t<>(247, 0xFF)},                              \
    {"mask_n_237", register_t<>(248, 0xFF)},                              \
    {"mask_n_238", register_t<>(249, 0xFF)},                              \
    {"mask_n_239", register_t<>(250, 0xFF)},                              \
    {"readout_select_column_0", register_t<>(0xFB, 0xFF)},               \
    {"readout_select_column_1", register_t<>(0xFC, 0xFF)},               \
    {"readout_select_column_2", register_t<>(0xFD, 0xFF)},               \
    {"readout_select_column_3", register_t<>(0xFE, 0xFF)},               \
    {"readout_select_column", register_t<>(0xFB, 0xFF, true, true, true)} \
  }
  // clang-format on

// clang-format off
#define C1004_SINGLEBALANCE_REGISTERS		             \
  {						             \
    {"num_acq_per_frame", register_t<std::uintptr_t, std::uintptr_t>(0x00, 0xFFFFFFFF)},   \
    {"qual_step", register_t<std::uintptr_t, std::uintptr_t>(0x08, 0x3FF)}, \
    {"dist_step_qual_0", register_t<std::uintptr_t, std::uintptr_t>(0x10, 0x7FF)}, \
    {"dist_step_qual_1", register_t<std::uintptr_t, std::uintptr_t>(0x18, 0x7FF)}, \
    {"dist_step_qual_2", register_t<std::uintptr_t, std::uintptr_t>(0x20, 0x7FF)}, \
    {"dist_step_qual_3", register_t<std::uintptr_t, std::uintptr_t>(0x28, 0x7FF)}, \
    {"active_range", register_t<std::uintptr_t, std::uintptr_t>(0x30, 0xF)},       \
    {"qual_half_range", register_t<std::uintptr_t, std::uintptr_t>(0x38, 0x3FF)},  \
  }

#define C1004_INTENSITY_REGISTERS		                                         \
  {						                                         \
    {"num_acq_per_frame", register_t<std::uintptr_t, std::uintptr_t>(0x00, 0xFFFFFFFF)}, \
  }

#define C1004_RAWTIMESTAMPS_REGISTERS					       \
    {									       \
    {"msb_bits", register_t<std::uintptr_t, std::uintptr_t>(0x00, 0x00000001)}, \
  }

  // clang-format on

} // namespace caribou

#endif
