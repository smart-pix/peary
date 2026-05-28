#pragma once

#include <fstream>
#include <vector>

#include "utils/datatypes.hpp"
#include "utils/log.hpp"

#include "H2MDefaults.hpp"
#include "h2m_pixels.hpp"
#include "utils/lfsr.hpp"

namespace caribou {

  class H2MFrameDecoder {
  public:
    template <typename T> pearydata decodeFrame(const std::vector<T>& rawFrame, uint8_t mode, bool decode_lfsr = true) {

      // check size of raw data
      if(rawFrame.size() != (H2M_NCOL * H2M_NROW) / H2M_NPXGROUP) {
        LOG(ERROR) << "Frame has wrong size: " << rawFrame.size() << " != " << (H2M_NCOL * H2M_NROW) / H2M_NPXGROUP;
        throw DataCorrupt("Frame has wrong size!");
      }

      if(mode == ACQ_MODE_TRG) {
        return decode_triggered_mode(rawFrame);
      } else {
        return decode_frame_mode(rawFrame, mode, decode_lfsr);
      }
    }

    /**
     * \brief Helper function to decode data recorded in triggered mode
     * \details This takes a frame recorded in triggered mode and turns it into a decoded pixel matrix. Every pixel in the
     * readout only contributes a single bit and the matrix readout is padded by trailing zeros. With 16 pixels per column
     * only the first two bytes can contain pixel flags, the rest has to be zeros.
     *
     * This also treats (regular patterns of) not-top-pixel configurations since higher row numbers will then be zero and
     * never show up in the decoded data. Irregular pattern would require knowing the full matrix configuration but are also
     * problematic in reconstruction.
     *
     * \param rawFrame Input frame data
     * \return map of decoded pixels
     */
    template <typename T> pearydata decode_triggered_mode(const std::vector<T>& rawFrame) {

      pearydata decodedData;
      auto dataWord = rawFrame.begin();

      // Pixel data returned in order Pixel 0 -> Pixel 15
      for(uint col = 0; col < H2M_NCOL; ++col) {
        for(uint row = 0; row < H2M_NROW; ++row) {
          // Shifting to the right pixel-by-pixel, read a single bit starting from the MSB:
          if(((*dataWord) >> (31 - row)) & 0x1) {
            // Only store if the hit bit is set, zero value is an empty pixel:
            decodedData[std::make_pair(col, row)] = std::make_unique<h2m_pixel_readout>(0x1, ACQ_MODE_TRG);
          }
        }
        // Drop the next blocks since they are zero - next data expected on next column:
        dataWord += 4;
      }
      return decodedData;
    }

    /**
     * \brief Helper function to decode data recorded in frame-based mode
     * \details Here each pixel provides 8 bits of data. The lowest pixel (0) is the one arriving first in the data, i.e.
     * located at the MSB of the word. Therefore we start reading the MSB for every block of 32b.
     *
     * The interpretation of the data depends on whether the LFSR was active or not and needs to be decoded.
     *
     * \param rawFrame Input frame data
     * \param mode Data taking mode the pixels were configured to
     * \param decode_lfsr Bool to select whether LFSR-encoding has to be reversed
     * \return map of decoded pixels
     */
    template <typename T>
    pearydata decode_frame_mode(const std::vector<T>& rawFrame, uint8_t mode, bool decode_lfsr = true) {

      pearydata decodedData;
      auto dataWord = rawFrame.begin();
      // Pixel data returned in order Pixel 0 -> Pixel 15
      for(uint col = 0; col < H2M_NCOL; ++col) {
        for(uint row = 0; row < H2M_NROW; ++row) {
          // Shifting to the right and applying a mask to extract 8-bits
          uint8_t data = ((*dataWord) >> (8 * (3 - (row % H2M_NPXGROUP)))) & 0xFF;

          if(decode_lfsr) {
            data = LFSR::LUT8(data);
          }

          // Only decode if we have data, zero value is an empty pixel:
          if(data > 0) {
            decodedData[std::make_pair(col, row)] = std::make_unique<h2m_pixel_readout>(data, mode);
          }

          // Move to next 32b data word if we have decoded the last pixel of this word
          if(row % H2M_NPXGROUP == (H2M_NPXGROUP - 1)) {
            dataWord++;
          }
        }
      }

      return decodedData;
    };

    /**
     * Header layout:
     *
     * 1st 32b: {ts_trigger[47:32] , t0_seen[0:0], reserved[5:0], frame_length[8:0]}
     * 2nd 32b: {ts_trigger[31:0]}
     * 3rd 32b: {ts_shutter_open[31:0]}
     * 4th 32b: {ts_shutter_close[47:32], ts_shutter_open[47:32]}
     * 5th 32b: {ts_shutter_close[31:0]}
     * 6th 32b: {frame_number[31:0]}
     */
    template <typename T>
    std::tuple<uint64_t, uint64_t, uint64_t, size_t, size_t, bool> decodeHeader(const std::vector<T>& data) {

      if(data.size() < 6) {
        LOG(WARNING) << "Not enough data to decode header";
        return {};
      }

      // Check for T0:
      bool t0_seen = data.at(0) & 0x8000;

      // Check for frame data length:
      size_t frame_length = data.at(0) & 0x1FF;

      // Read timestamps:
      uint64_t ts_trigger = (static_cast<uint64_t>(data.at(0) & 0xFFFF0000) << 16) | data.at(1);
      uint64_t ts_shutter_open = (static_cast<uint64_t>(data.at(3) & 0x0000FFFF) << 32) | data.at(2);
      uint64_t ts_shutter_close = (static_cast<uint64_t>(data.at(3) & 0xFFFF0000) << 16) | data.at(4);

      // Frame ID:
      size_t frame_id = data.at(5);

      return {ts_trigger, ts_shutter_open, ts_shutter_close, frame_id, frame_length, t0_seen};
    };

    /* Print frame to ASCII for easy inspection
     */
    void printFrame(const pearydata& frame, const char* filename, bool PrintEndFrame = false);

    typedef unsigned int index_type;

  private:
    /* Print some frames for easy assessment
     */
    std::ofstream m_outfileFrames;
  };

} // namespace caribou
