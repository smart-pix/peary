#ifndef DEVICE_APTS_DEFAULTS_H
#define DEVICE_APTS_DEFAULTS_H

#include "utils/dictionary.hpp"

namespace caribou {

  // Supply voltages

#define APTS_AVDD 1.2
#define APTS_AVDD_CURRENT 0.2
#define APTS_VD 1.2
#define APTS_VD_CURRENT 0.2

  // Bias voltages

#define APTS_IBIAS4_P 0.00495 // for source follower:  0.198V = 6mA*100Ohm/(100k/33k) or 0.00495V = 150uA*100Ohm/(100k/33k)
#define APTS_IBIAS4_N 0.0     // for opamp: 0.0825V = 2.5mA*100Ohm/(100k/33k)
#define APTS_VH 0.0
#define APTS_SEL_0 0.0
#define APTS_SEL_1 0.0
#define APTS_TRG 0.0
#define APTS_VRESET 0.5
#define APTS_IRESET_P 0.0
#define APTS_IRESET_N 0.1 // 0.1V = 1µA*10kOhm/(10k/100k)

  // OPAMP bias
#define APTS_VCASN 0.75
#define APTS_VCASP 0.3

  // Bias currents (uA)

#define APTS_IBIASN 20
#define APTS_IBIASP 2
#define APTS_IBIAS3 200
#define APTS_CUR1 0

  // DPTS readout
  const std::intptr_t APTS_BASE_ADDRESS = 0x43C40000;
  const std::intptr_t APTS_LSB = 1;
  const std::size_t APTS_MAP_SIZE = 4096;

#define APTS_MEMORY                                                                                                         \
  {                                                                                                                         \
    {"gpio",                                                                                                                \
     {memory_map(APTS_BASE_ADDRESS, APTS_MAP_SIZE, PROT_READ | PROT_WRITE),                                                 \
      register_t<std::uintptr_t, std::uintptr_t>((0 << APTS_LSB))}},                                                        \
  }

} // namespace caribou

#endif /* DEVICE_APTS_DEFAULTS_H */
