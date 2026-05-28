#ifndef DEVICE_DPTS_DEFAULTS_H
#define DEVICE_DPTS_DEFAULTS_H

#include "utils/dictionary.hpp"

namespace caribou {

  // Supply voltages

#define DPTS_AVDD 1.2
#define DPTS_AVDD_CURRENT 0.05
#define DPTS_DVDD 1.2
#define DPTS_DVDD_CURRENT 0.05
#define DPTS_BVDD 1.2
#define DPTS_BVDD_CURRENT 0.05

  // Bias voltages

#define DPTS_VCASB 0.3
#define DPTS_VCASN 0.3
#define DPTS_VH 0
#define DPTS_IBIASF 0.132 // 0.132V = 4mA*100Ohm/(100k/33k)

  // Bias currents

#define DPTS_IBIASN 1
#define DPTS_IBIAS 1
#define DPTS_IRESET 10
#define DPTS_IDB 10

  // DPTS readout
  const std::intptr_t DPTS_BASE_ADDRESS = 0x43C40000;
  const std::intptr_t DPTS_LSB = 1;
  const std::size_t DPTS_MAP_SIZE = 4096;

#define DPTS_MEMORY                                                                                                         \
  {                                                                                                                         \
    {"gpio",                                                                                                                \
     {memory_map(DPTS_BASE_ADDRESS, DPTS_MAP_SIZE, PROT_READ | PROT_WRITE),                                                 \
      register_t<std::uintptr_t, std::uintptr_t>((0 << DPTS_LSB))}},                                                        \
  }

} // namespace caribou

#endif /* DEVICE_DPTS_DEFAULTS_H */
