/**
 * Caribou SPI interface class implementation
 */

#include <array>
#include <cstring>
#include <utility>

// OS SPI support
#include <fcntl.h>

#ifndef SPI_BUS_EMULATED
#include <linux/spi/spidev.h>
#endif

#include <sys/ioctl.h>
#include <unistd.h>

#include "utils/log.hpp"
#include "utils/utils.hpp"

#include "spi_bus.hpp"

#ifndef SPI_BUS_EMULATED
using namespace caribou;
#endif

iface_spi_bus::iface_spi_bus(const configuration_type& config)
    : iface_spi(config), _addressBits(config._addressBits), _dataBits(config._dataBits),
      _addressMask(static_cast<spi_reg_t>((1 << _addressBits) - 1)), _dataMask(static_cast<spi_t>((1 << _dataBits) - 1)),
      _length(static_cast<spi_t>((1 + _addressBits + _dataBits) / 8 + ((1 + _addressBits + _dataBits) % 8 != 0))),
      _ws(config._ws), _alignMSB(config._alignMSB) {}

std::pair<iface_spi<>::spi_reg_t, spi_t> iface_spi_bus::access(const bool rw,
                                                               const std::pair<iface_spi<>::spi_reg_t, spi_t>& data) {

  std::lock_guard<std::mutex> lock(mutex);
  std::pair<spi_reg_t, spi_t> rx;

#ifndef SPI_BUS_EMULATED
  uintmax_t shift = 0;
  std::array<uint8_t, 1 + sizeof(spi_reg_t) + sizeof(spi_t)> _data;

  if(_alignMSB) {
    shift = (static_cast<uintmax_t>(rw ? _ws : !_ws) << (sizeof(shift) * 8 - 1)) |
            (static_cast<uintmax_t>(data.first & _addressMask) << (sizeof(shift) * 8 - 1 - _addressBits)) |
            (static_cast<uintmax_t>(data.second & _dataMask) << (sizeof(shift) * 8 - 1 - _addressBits - _dataBits));

    // copy data to the char array
    // shifting assure endianess on different machines
    for(unsigned int i = 0; i < _length; i++) {
      _data[i] = (shift >> (sizeof(shift) - 1 - i) * 8) & 0xFF;
    }
  } else {
    shift = (static_cast<uintmax_t>(rw ? _ws : !_ws) << (_addressBits + _dataBits)) |
            static_cast<uintmax_t>((data.first & _addressMask) << _dataBits) |
            static_cast<uintmax_t>(data.second & _dataMask);

    // copy data to the char array
    // shifting assure endianess on different machines
    for(unsigned int i = 0; i < _length; i++) {
      _data[i] = (shift >> (_length - 1 - i) * 8) & 0xFF;
    }
  }

  spi_ioc_transfer tr = spi_ioc_transfer();
  tr.tx_buf = reinterpret_cast<uintptr_t>(_data.data());
  tr.rx_buf = reinterpret_cast<uintptr_t>(_data.data());
  tr.len = static_cast<__u32>(_length);

  if(ioctl(spiDesc, SPI_IOC_MESSAGE(1), &tr) < 1) {
    throw CommunicationError("Failed to access device " + devicePath() + ": " + std::strerror(errno));
  }

  shift = 0;

  if(_alignMSB) {
    for(unsigned int i = 0; i < _length; i++) {
      shift |= (static_cast<uintmax_t>(_data[i]) << (sizeof(shift) - 1 - i) * 8);
    }
    rx = std::make_pair(static_cast<spi_reg_t>((shift >> (sizeof(shift) * 8 - 1 - _addressBits)) & _addressMask),
                        static_cast<spi_t>((shift >> (sizeof(shift) * 8 - 1 - _addressBits - _dataBits)) & _dataMask));
  } else {
    for(unsigned int i = 0; i < _length; i++) {
      shift |= (static_cast<uintmax_t>(_data[i]) << (_length - 1 - i) * 8);
    }

    rx = std::make_pair(static_cast<spi_reg_t>((shift >> _dataBits) & _addressMask), static_cast<spi_t>(shift & _dataMask));
  }

#endif

  LOG(TRACE) << "SPI device " << devicePath() << ": Register " << to_hex_string(data.first) << " Wrote data \""
             << to_hex_string(data.second) << (rw ? "(write)" : "(read)") << "\" Read data \"" << to_hex_string(rx.second)
             << "\"";
  return rx;
}

std::pair<iface_spi<>::spi_reg_t, spi_t> iface_spi_bus::write(const std::pair<iface_spi<>::spi_reg_t, spi_t>& data) {
  return access(true, data);
}

std::vector<std::pair<iface_spi<>::spi_reg_t, spi_t>>
iface_spi_bus::write(const std::vector<std::pair<iface_spi<>::spi_reg_t, spi_t>>& data) {
  std::vector<std::pair<spi_reg_t, spi_t>> rx;
  rx.reserve(data.size());
  for(const auto& _data : data) {
    rx.push_back(write(_data));
  }

  return rx;
}

iface_spi_bus::dataVector_type iface_spi_bus::read(const iface_spi<>::spi_reg_t& reg, const unsigned int length) {

  dataVector_type rx;

  for(unsigned int i = 0; i < length; i++) {
    rx.push_back(access(false, std::make_pair(reg, 0)).second);
  }

  return rx;
}
