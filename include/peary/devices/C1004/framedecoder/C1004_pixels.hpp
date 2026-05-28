// Defines C1004 pixel types

#ifndef C1004_PIXELS_HPP
#define C1004_PIXELS_HPP

#include <ostream>
#include "utils/datatypes.hpp"

namespace caribou {

  // Basic C1004 pixel class
  class C1004_pixel : public pixel {
  public:
    virtual ~C1004_pixel(){};
    typedef uint16_t value_type;

  protected:
    C1004_pixel(){};
    C1004_pixel(uint16_t val) : value(val){};
    value_type value;
  };

  // Pixel holding information from the balance algorithm
  class C1004_balancePixel : public C1004_pixel {
  public:
    C1004_balancePixel(){};
    C1004_balancePixel(uint16_t val) : C1004_pixel(val){};
    C1004_balancePixel(uint8_t quality, uint16_t timestamp) {
      SetQuality(quality);
      SetTimestamp(timestamp);
    };

    void SetQuality(uint8_t quality) { value = static_cast<uint16_t>((value & 0x9FFF) | ((quality & 0x3) << 13)); }

    uint8_t GetQuality() const { return (value >> 13) & 0x3; }

    void SetTimestamp(uint16_t timestamp) { value = static_cast<uint16_t>((value & 0xE000) | (timestamp & 0x1FFF)); }

    uint16_t GetTimestamp() const { return (value & 0x1FFF); }

    /** Overloaded print function for ostream operator
     */
    void print(std::ostream& out) const {
      out << static_cast<unsigned int>(this->GetQuality()) << "," << static_cast<unsigned int>(this->GetTimestamp());
    }
  };

} // namespace caribou

#endif
