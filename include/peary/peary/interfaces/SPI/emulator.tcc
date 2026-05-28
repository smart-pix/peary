/**
 * Caribou SPI interface class implementation
 */

#include <climits>
#include <cstring>
#include <utility>

#include "utils/log.hpp"
#include "utils/utils.hpp"

namespace caribou {

  template <typename REG_T>
  iface_spi<REG_T>::iface_spi(const caribou::InterfaceConfiguration& config) : Interface<REG_T, spi_t>(config) {
    std::lock_guard<std::mutex> lock(mutex);

    // Open device
  }
  template <typename REG_T> iface_spi<REG_T>::~iface_spi() {
    LOG(TRACE) << "Closing SPI device " << this->devicePath();
  }
  template <typename REG_T> std::pair<REG_T, spi_t> iface_spi<REG_T>::write(const std::pair<REG_T, spi_t>& data) {

    std::lock_guard<std::mutex> lock(mutex);
    std::pair<REG_T, spi_t> rx = std::make_pair(REG_T(), spi_t());

    LOG(TRACE) << "SPI/emu (" << this->devicePath() << ") : Register " << to_hex_string(data.first) << " Wrote data \""
               << to_hex_string(data.second) << "\" Read data \"" << to_hex_string(rx.second) << "\"";

    return rx;
  }

  template <typename REG_T>
  Interface<>::dataVector_type iface_spi<REG_T>::write(const REG_T& reg, const Interface<>::dataVector_type& data) {

    std::vector<std::pair<spi_reg_t, spi_t>> _data;
    Interface<>::dataVector_type rx;

    _data.reserve(data.size());
    for(auto i : data) {
      _data.push_back(std::make_pair(reg, i));
    }

    _data = write(_data);

    rx.reserve(data.size());
    for(auto i : _data) {
      rx.push_back(i.second);
    }

    return rx;
  }
  template <typename REG_T>
  std::vector<std::pair<REG_T, spi_t>> iface_spi<REG_T>::write(const std::vector<std::pair<REG_T, spi_t>>& data) {

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

    LOG(TRACE) << "SPI/emu (" << this->devicePath() << ")\n\t Wrote block data (Reg: data): \""
               << listVector(data, ", ", true) << "\"\n\t Read  block data (Reg: data): \"" << listVector(rx, ", ", true)
               << "\"";

    return rx;
  }
  template <typename REG_T>
  Interface<>::dataVector_type iface_spi<REG_T>::read(const REG_T& reg, const unsigned int length) {
    Interface<>::dataVector_type rx(length);
    return write(reg, rx);
  }
} // namespace caribou
