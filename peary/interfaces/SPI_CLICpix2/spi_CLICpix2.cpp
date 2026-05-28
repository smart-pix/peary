/**
 * Caribou SPI interface class implementation
 */

#include <array>
#include <cstring>
#include <utility>

// OS SPI support
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "spi_CLICpix2.hpp"
#include "utils/log.hpp"
#include "utils/utils.hpp"

using namespace caribou;

std::pair<iface_spi<>::spi_reg_t, spi_t> iface_spi_CLICpix2::write(const std::pair<iface_spi<>::spi_reg_t, spi_t>& data) {

  std::lock_guard<std::mutex> lock(mutex);
  std::array<uint8_t, 2 * (sizeof(spi_reg_t) + sizeof(spi_t))> _data;

  std::memcpy(_data.data(), &data.first, sizeof(spi_reg_t));
  std::memcpy(_data.data() + sizeof(spi_reg_t), &data.second, sizeof(spi_t));

  spi_ioc_transfer tr;
  tr = spi_ioc_transfer();
  tr.tx_buf = reinterpret_cast<uintptr_t>(_data.data());
  tr.rx_buf = reinterpret_cast<uintptr_t>(_data.data());
  tr.len = 2 * (sizeof(spi_reg_t) + sizeof(spi_t));

  if(ioctl(spiDesc, SPI_IOC_MESSAGE(1), &tr) < 3) {
    throw CommunicationError("Failed to access device " + devicePath() + ": " + std::strerror(errno));
  }

  uint8_t* rx_raw = _data.data();
  std::pair<spi_reg_t, spi_t> rx(static_cast<spi_reg_t>((rx_raw[0] << 3) | ((rx_raw[1] >> 5) & 0x1F)),
                                 static_cast<spi_t>(((rx_raw[1] & (0x1F)) << 3) | ((rx_raw[2] & 0xE0) >> 5)));

  LOG(TRACE) << "SPI/CP2 device " << devicePath() << ": Register " << to_hex_string(data.first) << " Wrote data \""
             << to_hex_string(data.second) << "\" Read data \"" << to_hex_string(rx.second) << "\"";

  return rx;
}

std::vector<std::pair<iface_spi<>::spi_reg_t, spi_t>>
iface_spi_CLICpix2::write(const std::vector<std::pair<iface_spi<>::spi_reg_t, spi_t>>& data) {

  std::lock_guard<std::mutex> lock(mutex);

  std::vector<uint8_t> _data(2 * (sizeof(spi_reg_t) + sizeof(spi_t)) * data.size(), 0);
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

    tr[loop.i].tx_buf = reinterpret_cast<uintptr_t>(_data.data()) + (sizeof(spi_reg_t) + sizeof(spi_t)) * 2 * loop.i;
    tr[loop.i].rx_buf = reinterpret_cast<uintptr_t>(_data.data()) + (sizeof(spi_reg_t) + sizeof(spi_t)) * 2 * loop.i;
    tr[loop.i].len = 2 * (sizeof(spi_reg_t) + sizeof(spi_t));

    loop.pos += static_cast<unsigned int>(sizeof(spi_reg_t) + sizeof(spi_t));

    // SPIDEV has limit of 2^7 words per transfer
    //(_IOC_SIZE has 14 bits indicates number of bytes taken by spi_ioc_transfer times number of messages)
    if(loop.i % (2 << 7) == ((2 << 7) - 1)) { // i % 2^7 == 2^7 - 1
      if(ioctl(spiDesc, SPI_IOC_MESSAGE((2 << 7)), &tr[loop.i - ((2 << 7) - 1)]) < (2 << 7)) {
        throw CommunicationError("Failed to access device " + devicePath() + ": " + std::strerror(errno));
      }
    }
  }
  // SPIDEV has limit of 2^7 words
  //(_IOC_SIZE has 14 bits indicates number of bytes taken by spi_ioc_transfer times number of messages)
  if((data.size() % (2 << 7)) != 0u) {
    // In order to avoid variable length arrays of SPI_IOC_MESSAGE macro
    // use the C++ vector
    std::vector<char> argp(SPI_MSGSIZE(data.size() % (2 << 7)));

    if(ioctl(spiDesc, _IOW(SPI_IOC_MAGIC, 0, argp.data()), &tr[data.size() / (2 << 7) * (2 << 7)]) <
       static_cast<int>(data.size()) % (2 << 7)) {
      throw CommunicationError("Failed to access device " + devicePath() + ": " + std::strerror(errno));
    }
  }
  // unpack
  rx.reserve(data.size());
  for(struct {
        unsigned int i = 0;
        unsigned int pos = 0;
      } loop;
      loop.i < data.size();
      ++loop.i) {

    uint8_t* rx_raw = _data.data() + loop.pos;
    rx.emplace_back(static_cast<spi_reg_t>((rx_raw[0] << 3) | ((rx_raw[1] >> 5) & 0x1F)),
                    static_cast<spi_t>(((rx_raw[1] & (0x1F)) << 3) | ((rx_raw[2] & 0xE0) >> 5)));
    loop.pos += 2 * static_cast<unsigned int>(sizeof(spi_t) + sizeof(spi_reg_t));
  }

  LOG(TRACE) << "SPI/CP2 device " << devicePath() << ": \n\t Wrote block data (Reg: data): \""
             << listVector(data, ", ", true) << "\"\n\t Read  block data (Reg: data): \"" << listVector(rx, ", ", true)
             << "\"";

  return rx;
}
