/**
 * Caribou SPI interface class implementation
 */

#include <cstring>
#include <utility>

// OS SPI support
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "utils/log.hpp"
#include "utils/utils.hpp"

namespace caribou {

  template <typename REG_T>
  iface_spi<REG_T>::iface_spi(const caribou::InterfaceConfiguration& config) : Interface<REG_T, spi_t>(config), spiDesc() {
    std::lock_guard<std::mutex> lock(mutex);

    // Open device
    if((spiDesc = open(this->devicePath().c_str(), O_RDWR)) < 0) {
      throw DeviceException("Open " + this->devicePath() + " device failed. " + std::strerror(spiDesc));
    }

    // The interface requires 8 bits_per_word transactions
    if(ioctl(spiDesc, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) == -1) {
      throw DeviceException("Set bits_per_per word on" + this->devicePath() + " device failed. " + std::strerror(spiDesc));
    }
  }

  template <typename REG_T> iface_spi<REG_T>::~iface_spi() {
    LOG(TRACE) << "Closing SPI device " << this->devicePath();
    close(spiDesc);
  }

  template <typename REG_T> std::pair<REG_T, spi_t> iface_spi<REG_T>::write(const std::pair<REG_T, spi_t>& data) {

    std::lock_guard<std::mutex> lock(mutex);
    std::array<uint8_t, sizeof(spi_reg_t) + sizeof(spi_t)> _data;

    std::memcpy(_data.data(), &data.first, sizeof(spi_reg_t));
    std::memcpy(_data.data() + sizeof(spi_reg_t), &data.second, sizeof(spi_t));

    spi_ioc_transfer tr = spi_ioc_transfer();
    tr.tx_buf = reinterpret_cast<uintptr_t>(_data.data());
    tr.rx_buf = reinterpret_cast<uintptr_t>(_data.data());
    tr.len = sizeof(spi_reg_t) + sizeof(spi_t);

    if(ioctl(spiDesc, SPI_IOC_MESSAGE(1), &tr) < 1) {
      throw CommunicationError("Failed to access device " + this->devicePath() + ": " + std::strerror(errno));
    }

    std::pair<spi_reg_t, spi_t> rx(*_data.data(), *_data.data() + sizeof(spi_reg_t));

    LOG(TRACE) << "SPI device " << this->devicePath() << ": Register " << to_hex_string(data.first) << " Wrote data \""
               << to_hex_string(data.second) << "\" Read data \"" << to_hex_string(rx.second) << "\"";

    return rx;
  }

  template <typename REG_T>
  Interface<>::dataVector_type iface_spi<REG_T>::write(const REG_T& reg, const Interface<>::dataVector_type& data) {

    std::vector<std::pair<spi_reg_t, spi_t>> _data;
    Interface<>::dataVector_type rx;

    _data.reserve(data.size());
    for(auto i : data) {
      _data.emplace_back(reg, i);
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
    std::unique_ptr<spi_ioc_transfer[]> tr(new spi_ioc_transfer[data.size()]());
    std::vector<std::pair<spi_reg_t, spi_t>> rx;

    // pack
    for(struct {
          unsigned int i = 0;
          unsigned int pos = 0;
        } loop;
        loop.i < data.size();
        ++loop.i) {
      std::memcpy(_data.data() + loop.pos, &data[loop.i].first, sizeof(spi_reg_t));
      loop.pos += static_cast<unsigned int>(sizeof(spi_reg_t));
      std::memcpy(_data.data() + loop.pos, &data[loop.i].second, sizeof(spi_t));
      loop.pos += static_cast<unsigned int>(sizeof(spi_t));

      tr[loop.i].tx_buf = reinterpret_cast<uintptr_t>(_data.data()) + (sizeof(spi_reg_t) + sizeof(spi_t)) * loop.i;
      tr[loop.i].rx_buf = reinterpret_cast<uintptr_t>(_data.data()) + (sizeof(spi_reg_t) + sizeof(spi_t)) * loop.i;
      tr[loop.i].len = sizeof(spi_reg_t) + sizeof(spi_t);
    }

    // In order to avoid variable length arrays of SPI_IOC_MESSAGE macro
    // use the C++ vector
    std::vector<char> argp(SPI_MSGSIZE(data.size()));
    if(ioctl(spiDesc, _IOW(SPI_IOC_MAGIC, 0, argp.data()), tr.get()) < static_cast<int>(data.size())) {
      throw CommunicationError("Failed to access device " + this->devicePath() + ": " + std::strerror(errno));
    }

    // unpack
    rx.reserve(data.size());
    for(struct {
          unsigned int i = 0;
          unsigned int pos = 0;
        } loop;
        loop.i < data.size();
        ++loop.i) {
      rx.emplace_back(*_data.data() + loop.pos, *_data.data() + loop.pos + sizeof(spi_reg_t));
      loop.pos += static_cast<unsigned int>(sizeof(spi_reg_t) + sizeof(spi_t));
    }

    LOG(TRACE) << "SPI device " << this->devicePath() << ": \n\t Wrote block data (Reg: data): \""
               << listVector(data, ", ", true) << "\"\n\t Read  block data (Reg: data): \"" << listVector(rx, ", ", true)
               << "\"";

    return rx;
  }

  template <typename REG_T>
  Interface<>::dataVector_type iface_spi<REG_T>::read(const REG_T& reg, const unsigned int length) {
    Interface<>::dataVector_type rx(length);
    // Workaround for SPI interface: when reading a register, "0" is written to the address.
    // Thus, we simply write back the retrieved value.

    // Read register value:
    Interface<>::dataVector_type data = write(reg, rx);
    // Re-write register value:
    write(reg, data);

    return data;
  }
} // namespace caribou
