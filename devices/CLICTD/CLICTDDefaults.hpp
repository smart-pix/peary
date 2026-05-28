#ifndef DEVICE_CLICTD_DEFAULTS_H
#define DEVICE_CLICTD_DEFAULTS_H

#include "utils/dictionary.hpp"

namespace caribou {

/** Default device path for this device: SEAF-connector I2C bus on BUS_I2C2
 */
#define DEFAULT_DEVICEPATH caribou::carboard::BUS_I2C2

/** Default I2C address for CLICTD chip-board with unconnected I2C address lines
 */
#define CLICTD_DEFAULT_I2C 0x50

#define CLICTD_VDDD 1.8
#define CLICTD_VDDD_CURRENT 3
#define CLICTD_VDDA 1.8
#define CLICTD_VDDA_CURRENT 3
#define CLICTD_PWELL 2
#define CLICTD_PWELL_CURRENT 3
#define CLICTD_SUB 2
#define CLICTD_SUB_CURRENT 3
#define CLICTD_VCTRL_M3V6 3.2
#define CLICTD_VCTRL_M3V6_CURRENT 3
#define CLICTD_P3V6 3.2
#define CLICTD_P3V6_CURRENT 3

#define CLICTD_MAX_CONF_RETRY 15

  // CLICTD readout
  const std::intptr_t CLICTD_READOUT_BASE_ADDRESS = 0x43C70000;
  const std::intptr_t CLICTD_READOUT_LSB = 2;
  const std::size_t CLICTD_READOUT_MAP_SIZE = 4096;

// CLICTD output signals
#define CLICTD_READOUT_START 0x1
#define CLICTD_POWER_ENABLE 0x2
#define CLICTD_TESTPULSE 0x4
#define CLICTD_SHUTTER 0x8
#define CLICTD_RESET 0x10
#define CLICTD_PULSER 0x20
#define CLICTD_DUMMY_READOUT_START 0x40

  const memory_map CLICTD_READOUT_RW{CLICTD_READOUT_BASE_ADDRESS, CLICTD_READOUT_MAP_SIZE, PROT_READ | PROT_WRITE};
  const memory_map CLICTD_READOUT_RO{CLICTD_READOUT_BASE_ADDRESS, CLICTD_READOUT_MAP_SIZE, PROT_READ};

  // clang-format off

#define CLICTD_MEMORY                                                                                                       \
  {                                                                                                                         \
    {"wgtriggertimeout", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(0 << CLICTD_READOUT_LSB)}},         \
      {"rdstatus", {CLICTD_READOUT_RO, register_t<std::uintptr_t, std::uintptr_t>(1 << CLICTD_READOUT_LSB)}},               \
      {"rdcontrol", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(2 << CLICTD_READOUT_LSB)}},              \
      {"chipcontrol", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(3 << CLICTD_READOUT_LSB)}},            \
      {"shuttertimeout", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(4 << CLICTD_READOUT_LSB)}},         \
      {"wgcontrol", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(5 << CLICTD_READOUT_LSB)}},              \
      {"wgstatus", {CLICTD_READOUT_RO, register_t<std::uintptr_t, std::uintptr_t>(6 << CLICTD_READOUT_LSB)}},               \
      {"wgcapacity", {CLICTD_READOUT_RO, register_t<std::uintptr_t, std::uintptr_t>(7 << CLICTD_READOUT_LSB)}},             \
      {"wgconfruns", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(8 << CLICTD_READOUT_LSB)}},             \
      {"wgpatterntime", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(9 << CLICTD_READOUT_LSB)}},          \
      {"wgpatternoutput", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(10 << CLICTD_READOUT_LSB)}},       \
      {"wgpatterntriggers", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(11 << CLICTD_READOUT_LSB)}},     \
      {"fifodata_lsb", {CLICTD_READOUT_RO, register_t<std::uintptr_t, std::uintptr_t>(12 << CLICTD_READOUT_LSB)}},          \
      {"fifodata_msb", {CLICTD_READOUT_RO, register_t<std::uintptr_t, std::uintptr_t>(13 << CLICTD_READOUT_LSB)}},          \
      {"fifostatus", {CLICTD_READOUT_RO, register_t<std::uintptr_t, std::uintptr_t>(14 << CLICTD_READOUT_LSB)}},            \
      {"tscontrol", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(15 << CLICTD_READOUT_LSB)}},             \
      {"tsedgeconf", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(16 << CLICTD_READOUT_LSB)}},            \
      {"tsinittime_lsb", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(17 << CLICTD_READOUT_LSB)}},        \
      {"tsinittime_msb", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(18 << CLICTD_READOUT_LSB)}},        \
      {"chipsignal_enable", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(19 << CLICTD_READOUT_LSB)}},     \
      {"pulser_periods", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(20 << CLICTD_READOUT_LSB)}},        \
      {"pulser_time_high", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(21 << CLICTD_READOUT_LSB)}},      \
      {"pulser_time_low", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(22 << CLICTD_READOUT_LSB)}},       \
      {"pulser_control", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(23 << CLICTD_READOUT_LSB)}},        \
      {"ps_num", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(24 << CLICTD_READOUT_LSB)}},                \
      {"ps_shift", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(25 << CLICTD_READOUT_LSB)}},              \
      {"ps_status", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(26 << CLICTD_READOUT_LSB)}},             \
      {"rdconfig", {CLICTD_READOUT_RW, register_t<std::uintptr_t, std::uintptr_t>(27 << CLICTD_READOUT_LSB)}}               \
  }

/** Dictionary for register address/name lookup for CLICTD
 */

#define CLICTD_REGISTERS				\
  {						\
    {"globalconfig", register_t<>(0x00, 0x07)},		\
    {"tpen", register_t<>(0x00, 0x01)},		\
    {"tpexten", register_t<>(0x00, 0x02)},		\
    {"pwrexten", register_t<>(0x00, 0x04)},		\
    {"internalstrobes", register_t<>(0x01, 0x03)},		\
    {"tpint", register_t<>(0x01, 0x01)},		\
    {"pwrint", register_t<>(0x01, 0x02)},		\
    {"externaldacsel", register_t<>(0x02,0x1F)},		\
    {"oir", register_t<>(0x02, 0x80)},		\
    {"monitordacsel", register_t<>(0x03, 0x1F)},		\
    {"matrixconfig", register_t<>(0x04, 0x1F)},		\
    {"photoncnt", register_t<>(0x04, 0x10)},		\
    {"nocompress", register_t<>(0x04, 0x08, true, true, true)},		\
    {"longcnt", register_t<>(0x04, 0x04, true, true, true)},		\
    {"totdiv", register_t<>(0x04, 0x03)},		\
    {"configctrl", register_t<>(0x05, 0x13, false)},		\
    {"configdata_lsb", register_t<>(0x06)},		\
    {"configdata_msb", register_t<>(0x07)},		\
    {"configdata", register_t<>(0x06, 0xFF, true, true, true)},		\
    {"readoutctrl", register_t<>(0x08, 0x03)},		\
    {"roint", register_t<>(0x08, 0x01)},		\
    {"roexten", register_t<>(0x08, 0x02)},		\
    {"tpulsectrl_lsb", register_t<>(0x0A)},		\
    {"tpulsectrl_msb", register_t<>(0x0B)},		\
    {"tpulsectrl", register_t<>(0x0A, 0xFF, false, true, true)},		\
    {"vbiasresettransistor", register_t<>(0x10)},		\
    {"vreset", register_t<>(0x11)},		\
    {"vbiaslevelshift", register_t<>(0x12, 0x3F)},		\
    {"vanalog1_lsb", register_t<>(0x13)},		\
    {"vanalog1_msb", register_t<>(0x14, 0x01)},		\
    {"vanalog1", register_t<>(0x13, 0xFF, true, true, true)},		\
    {"vanalog2", register_t<>(0x15)},		\
    {"vbiaspreampn", register_t<>(0x16, 0x3F)},		\
    {"vncasc", register_t<>(0x17)},		\
    {"vpcasc", register_t<>(0x18)},		\
    {"vfbk", register_t<>(0x19)},		\
    {"vbiasikrum", register_t<>(0x1A, 0x3F)},		\
    {"vbiasdiscn", register_t<>(0x1B, 0x3F)},		\
    {"vbiasdiscp", register_t<>(0x1C, 0x3F)},		\
    {"vbiasdac", register_t<>(0x1D, 0x3F)},		\
    {"vthreshold_lsb", register_t<>(0x1E)},		\
    {"vthreshold_msb", register_t<>(0x1F, 0x01)},		\
    {"vthreshold", register_t<>(0x1E, 0xFF, true, true, true)},		\
    {"vncasccomp", register_t<>(0x20)},		\
    {"vbiaslevelshiftstby", register_t<>(0x21, 0x3F)},		\
    {"vbiaspreampnstby", register_t<>(0x22, 0x3F)},		\
    {"vbiasdiscnstby", register_t<>(0x23, 0x3F)},		\
    {"vbiasdiscpstby", register_t<>(0x24, 0x3F)},		\
    {"vbiasdacstby", register_t<>(0x25, 0x3F)},		\
    {"vbiasslowbuffer", register_t<>(0x26)},		\
    {"adjustdacrange", register_t<>(0x27)},		\
    {"vlvdsd", register_t<>(0x28, 0x0F)},		\
    {"bias_analog_in", register_t<>(0x00, 0x00, true, true, true)},		\
    {"tp_phase",     register_t<>(0x00, 0x00, false, true, true)},		\
    {"tp_phase_add", register_t<>(0x00, 0x00, false, true, true)},		\
    {"tp_phase_sub", register_t<>(0x00, 0x00, false, true, true)},		\
    {"tp_rand_clk",  register_t<>(0x00, 0x00, true, true, true)},		\
  }
  // clang-format on

} // namespace caribou

#endif /* DEVICE_CLICTD_DEFAULTS_H */
