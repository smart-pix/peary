/**
 * Caribou SPI interface class implementation
 */

#include <climits>
#include <cstring>
#include <utility>

#include "spi_CLICpix2.hpp"
#include "utils/log.hpp"
#include "utils/utils.hpp"

using namespace caribou;

std::pair<iface_spi<>::spi_reg_t, spi_t> iface_spi_CLICpix2::write(const std::pair<iface_spi<>::spi_reg_t, spi_t>& data) {

  std::lock_guard<std::mutex> lock(mutex);

  std::pair<spi_reg_t, spi_t> rx = std::make_pair(spi_reg_t(), spi_t());

  LOG(TRACE) << "SPI/CP2/emu (" << devicePath() << "): Register " << to_hex_string(data.first) << " Wrote data \""
             << to_hex_string(data.second) << "\" Read data \"" << to_hex_string(rx.second) << "\"";

  return rx;
}

std::vector<std::pair<iface_spi<>::spi_reg_t, spi_t>>
iface_spi_CLICpix2::write(const std::vector<std::pair<iface_spi<>::spi_reg_t, spi_t>>& data) {

  std::lock_guard<std::mutex> lock(mutex);

  std::vector<uint8_t> _data((sizeof(spi_reg_t) + sizeof(spi_t)) * data.size());
  std::vector<std::pair<spi_reg_t, spi_t>> rx;

  // pack
  for(struct {
        unsigned int i = 0;
        unsigned int pos = 0;
      } loop;
      loop.i < data.size();
      ++loop.i) {
    std::memcpy(_data.data() + loop.pos, &data[loop.i].second, sizeof(spi_t));
    loop.pos += static_cast<unsigned int>(sizeof(spi_t));
    std::memcpy(_data.data() + loop.pos + sizeof(spi_t), &data[loop.i].first, sizeof(spi_reg_t));
    loop.pos += static_cast<unsigned int>(sizeof(spi_reg_t));
  }

  // unpack
  rx.reserve(data.size());
  for(struct {
        unsigned int i = 0;
        unsigned int pos = 0;
      } loop;
      loop.i < data.size();
      ++loop.i) {
    rx.push_back(std::make_pair(*_data.data() + loop.pos + sizeof(spi_t), *_data.data() + loop.pos));
  }

  LOG(TRACE) << "SPI/CP2/emu (" << devicePath() << ")\n\t Wrote block data (Reg: data): \"" << listVector(data, ", ", true)
             << "\"\n\t Read  block data (Reg: data): \"" << listVector(rx, ", ", true) << "\"";

  return rx;
}
