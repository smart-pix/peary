// C1004 frame decoder

#ifndef C1004_FRAMEDECODER_HPP
#define C1004_FRAMEDECODER_HPP

#include <ostream>
#include "interfaces/Media/media.hpp"
#include "utils/datatypes.hpp"

namespace caribou {

  class C1004_frameDecoder {
  public:
    static const unsigned int C1004_ROW = 60;
    static const unsigned int C1004_COL = 32;

    void decode(const iface_media::data_type& frame);
    pearydata getFrame();

    /** Overloaded ostream operator for simple printing of pixel data
     */
    friend std::ostream& operator<<(std::ostream& out, const C1004_frameDecoder& decoder);

  private:
    iface_media::data_type _frame;
  };
} // namespace caribou

#endif
