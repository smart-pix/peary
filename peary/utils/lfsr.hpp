/* This file contains helper classes which are used by the Caribou
 * library implementations
 */

#ifndef CARIBOU_LFSR_H
#define CARIBOU_LFSR_H

#include <array>
#include <cstdint>

namespace caribou {

  class LFSR {
  public:
    /**
     * Lookup Table for 5-bit XNOR LFSR counters
     */
    static uint16_t LUT13(uint16_t);

    /**
     * Lookup Table for 8-bit XNOR LFSR counters
     */
    static uint8_t LUT8(uint8_t);

    /**
     * Lookup Table for 13-bit XNOR LFSR counters
     */
    static uint8_t LUT5(uint8_t);

  private:
    static const std::array<uint16_t, 8191> lfsr13_lut;
    static const std::array<uint8_t, 255> lfsr8_lut;
    static const std::array<uint8_t, 31> lfsr5_lut;
  };

} // namespace caribou

#endif /* CARIBOU_LFSR_H */
