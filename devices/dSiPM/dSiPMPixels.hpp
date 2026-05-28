// Defines CLICTD pixels types

#ifndef DSIPM_PIXELS_HPP
#define DSIPM_PIXELS_HPP

#include <ostream>
#include "utils/datatypes.hpp"
#include "utils/exceptions.hpp"

namespace caribou {

  /* Basic pixel class
   * Binary data, so signal is simplt a boolean
   */
  // TODO add timestamps?
  // TODO For CLICTD this class is virtual, data and constructors are  protected.
  //      Is this fine for our purpose, as we do not need derived classes for configuration?

  class dsipm_pixel : public pixel {
  public:
    dsipm_pixel() = default;
    dsipm_pixel(uint8_t s, bool vb, uint64_t bc, uint8_t ft, uint8_t ct)
        : m_bit(s), m_validBit(vb), m_bunchCounter(bc), m_fineTime(ft), m_coarseTime(ct){};
    virtual ~dsipm_pixel(){};

    /* get functions for private data members
     */
    uint8_t getBit() { return m_bit; }
    uint64_t getBunchCounter() { return m_bunchCounter; }
    uint8_t getFineTime() { return m_fineTime; }
    uint8_t getCoarseTime() { return m_coarseTime; }
    bool getValidBit() { return m_validBit; }

  private:
    /* hit data
     */
    uint8_t m_bit;
    /* time data
     */
    bool m_validBit;
    uint64_t m_bunchCounter;
    uint8_t m_fineTime;
    uint8_t m_coarseTime;
  };

} // namespace caribou

#endif
