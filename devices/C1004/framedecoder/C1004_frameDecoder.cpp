// Implementation of the C1004 frame decoder

#include "C1004_frameDecoder.hpp"
#include "C1004_pixels.hpp"

using namespace caribou;

void C1004_frameDecoder::decode(const iface_media::data_type& frame) {
  if(frame.size() != C1004_frameDecoder::C1004_ROW * C1004_frameDecoder::C1004_COL * sizeof(C1004_balancePixel::value_type) /
                       sizeof(iface_media::data_type::value_type))
    throw caribou::DataException(
      "Wrong frame size (received: " +
      std::to_string(frame.size() * sizeof(iface_media::data_type::value_type) / sizeof(C1004_balancePixel::value_type)) +
      " pixels, expected: " + std::to_string(C1004_frameDecoder::C1004_ROW * C1004_frameDecoder::C1004_COL) + " pixels)");

  this->_frame = iface_media::data_type(frame);
}

pearydata C1004_frameDecoder::getFrame() {
  pearydata decFrame;
  auto pointer = reinterpret_cast<const uint16_t*>(this->_frame.data());
  for(auto r = 0u; r < C1004_frameDecoder::C1004_ROW; ++r)
    for(auto c = 0u; c < C1004_frameDecoder::C1004_COL; ++c)
      decFrame[std::make_pair(c, r)] = std::make_unique<C1004_balancePixel>(*pointer++);
  return decFrame;
}

namespace caribou {
  std::ostream& operator<<(std::ostream& out, const C1004_frameDecoder& decoder) {
    auto pointer = reinterpret_cast<const uint16_t*>(decoder._frame.data());
    for(auto r = 0u; r < C1004_frameDecoder::C1004_ROW; ++r)
      for(auto c = 0u; c < C1004_frameDecoder::C1004_COL; ++c)
        out << "[" << r << "][" << c << "] [" << C1004_balancePixel(*pointer++);
    return out;
  }
} // namespace caribou
