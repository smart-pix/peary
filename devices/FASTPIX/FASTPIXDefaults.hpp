#ifndef DEVICE_FASTPIX_DEFAULTS_H
#define DEVICE_FASTPIX_DEFAULTS_H

#include "utils/dictionary.hpp"

namespace caribou {

  // Supply voltages

#define FASTPIX_AVDD 1.8
#define FASTPIX_AVDD_CURRENT 0.3
#define FASTPIX_DVDD 1.8
#define FASTPIX_DVDD_CURRENT 0.3
#define FASTPIX_PVDD 1.8
#define FASTPIX_PVDD_CURRENT 0.3

  // Bias voltages

#define FASTPIX_NGATE 0.7
#define FASTPIX_DRESET 0.9
#define FASTPIX_VRESET 0.8
#define FASTPIX_VPULSE 1.8
#define FASTPIX_BUFF_VCASP 0.6
#define FASTPIX_BUFF_VCASN 0.8
#define FASTPIX_BUFF_VCAS 1.2
#define FASTPIX_VCAS2 1.1
#define FASTPIX_VCASP 0.6

  // Bias currents

#define FASTPIX_ITHR 0.82
#define FASTPIX_IDB 0.65
#define FASTPIX_IBIASP_OFF 0.98
#define FASTPIX_IRESET 1.13
#define FASTPIX_BUFF_IBIASP 0.72

#define FASTPIX_BUFF_IBIASN 1.55
#define FASTPIX_ITHRN 1.25

#define FASTPIX_IBIASP 1000
#define FASTPIX_IBIASN 0 // Not used

  // Digital outputs

#define FASTPIX_PB 0
#define FASTPIX_EN_CMFB 0
#define FASTPIX_PRE 0
#define FASTPIX_HBRIDGE 0

  // FASTPIX readout
  const std::intptr_t FASTPIX_BASE_ADDRESS = 0x43C40000;
  const std::intptr_t FASTPIX_LSB = 1;
  const std::size_t FASTPIX_MAP_SIZE = 4096;

#define FASTPIX_MEMORY                                                                                                      \
  {                                                                                                                         \
    {"gpio",                                                                                                                \
     {memory_map(FASTPIX_BASE_ADDRESS, FASTPIX_MAP_SIZE, PROT_READ | PROT_WRITE),                                           \
      register_t<std::uintptr_t, std::uintptr_t>((0 << FASTPIX_LSB))}},                                                     \
  }

} // namespace caribou

#endif /* DEVICE_FASTPIX_DEFAULTS_H */
