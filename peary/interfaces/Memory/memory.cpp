/**
 * Caribou Memory interface class implementation
 */

#include "utils/log.hpp"
#include "utils/utils.hpp"

#include "memory.hpp"

using namespace caribou;

iface_mem::iface_mem(const configuration_type& config)
    : Interface(config), _memfd(-1), _mappedMemory(nullptr), _mem(config._mem) {

  // Get access to FPGA memory mapped registers
  _memfd = open(devicePath().c_str(), O_RDWR | O_SYNC);

  // If we still don't have one, something went wrong:
  if(_memfd == -1) {
    throw DeviceException("Can't open /dev/mem.\n");
  }
  LOG(TRACE) << "Opened memory device at " << devicePath();

  mapMemory(mem());
}

iface_mem::~iface_mem() {
  // Unmap memory page:
  LOG(TRACE) << "Unmapping memory at " << std::hex << mem().getBaseAddress() << std::dec;
  if(munmap(_mappedMemory, mem().getSize()) == -1) {
    LOG(FATAL) << "Can't unmap memory from user space.";
  }

  close(_memfd);
}

std::pair<size_t, uintptr_t> iface_mem::write(const std::pair<size_t, uintptr_t>& dest) {
  if(!mem().writable()) {
    throw ConfigInvalid("Requested memory page not writable");
  }

  LOG(TRACE) << "Writing to mapped memory at 0x" << std::hex << mem().getBaseAddress() << ", offset " << dest.first
             << std::dec << ": " << dest.second;
  volatile auto* reg = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::uintptr_t>(_mappedMemory) + dest.first);
  *reg = (uint32_t)dest.second;
  return std::pair<size_t, uintptr_t>();
}

uintptr_t iface_mem::readWord(const size_t offset) const {
  //[AQ 5/9/2024] Treat reads as 32bits always for Spacely-Caribou (platform-independent)
  
  volatile auto* reg = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::uintptr_t>(_mappedMemory) + offset);
  uint32_t value = *reg;
  uintptr_t value_1 = (uintptr_t)value;
  LOG(TRACE) << "Reading from mapped memory at 0x" << std::hex << mem().getBaseAddress() << ", offset " << offset << std::dec
             << ": " << value;
  return value_1;
}

iface_mem::dataVector_type iface_mem::read(const size_t& offset, const unsigned int n) {
  dataVector_type values;
  for(unsigned int i = 0; i < n; i++) {
    values.push_back(this->readWord(offset));
  }
  return values;
}

//mem.read() slightly optimized for fast reads of the same register many times.
iface_mem::dataVector_type iface_mem::read_opt(const size_t& offset, const unsigned int n) {
  dataVector_type values;

  //Calculate the pointer once.
  volatile auto* reg = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<std::uintptr_t>(_mappedMemory) + offset);

  //Pre-emptively reserve all the space we may need to hold N integers.
  values.reserve(n);

  //In the critical loop, do only critical steps (dereference reg and store to values)
  for(unsigned int i = 0; i < n; i++) {
    values.push_back(*reg);
  }
  return values;
}

void iface_mem::mapMemory(const memory_map& page) {

  // Check if this memory page is already mapped and return the pointer:
  LOG(TRACE) << "Returning mapped memory at 0x" << std::hex << page.getBaseAddress() << std::dec;

  LOG(TRACE) << "Memory was not yet mapped, mapping...";
  // Map one page of memory into user space such that the device is in that page, but it may not
  // be at the start of the page.
  void* map_base = mmap(nullptr,
                        page.getSize(),
                        page.getFlags(),
                        MAP_SHARED,
                        _memfd,
                        static_cast<long int>(page.getBaseAddress() & ~(page.getSize() - 1)));
  if(map_base == reinterpret_cast<void*>(-1)) {
    throw DeviceException("Can't map the memory to user space.\n");
  }

  // get the address of the device in user space which will be an offset from the base
  // that was mapped as memory is mapped at the start of a page
  void* base_pointer =
    reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(map_base) + (page.getBaseAddress() & (page.getSize() - 1)));

  // Store the mapped memory, so we can unmap it later:
  _mappedMemory = base_pointer;
}
