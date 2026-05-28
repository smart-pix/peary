/**
 * Caribou HAL class implementation
 */

#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>  // for std::hex formatting

#include <functional>
#include <unordered_map>

#include "HAL.hpp"

#include "carboard/Carboard.hpp"
#include "falconboard/Falconboard.hpp"

using namespace caribou;

bool caribouHAL::generalResetDone = false;

caribouHAL::~caribouHAL() {
  this->Finalize();
}

void caribouHAL::Initialize() {
  if(!caribou::caribouHAL::generalResetDone) { // board needs to be reset
    generalReset();
  }
}

void caribouHAL::Finalize() {}

void caribouHAL::generalReset() {
  caribou::caribouHAL::generalResetDone = true;
}

void caribouHAL::writeMemory(memory_map mem, uintptr_t value) {
  writeMemory(mem, 0, value);
}

void caribouHAL::writeMemory(memory_map mem, size_t offset, uintptr_t value) {
  iface_mem& imem = InterfaceManager::getInterface<iface_mem>(iface_mem::configuration_type(MEM_PATH, mem));
  imem.write(std::make_pair(offset, value));
}

uintptr_t caribouHAL::readMemory(memory_map mem) {
  return readMemory(mem, 0);
}

uintptr_t caribouHAL::readMemory(memory_map mem, size_t offset) {
  iface_mem& imem = InterfaceManager::getInterface<iface_mem>(iface_mem::configuration_type(MEM_PATH, mem));

  return imem.read(offset, 1).front();
}



// Function to read N words from memory and store them in a text file
void caribouHAL::streamMemoryToFile(memory_map mem, const size_t offset, const unsigned int N, const std::string& filename) {

  iface_mem& imem = InterfaceManager::getInterface<iface_mem>(iface_mem::configuration_type(MEM_PATH, mem));
  
  // Read N words from memory
  std::vector<uintptr_t> words = imem.read_opt(offset, N);

  // Open the file for writing
  std::ofstream outFile(filename);

  if (!outFile.is_open()) {
    LOG(TRACE) << "Failed to open the file for writing: " << filename << std::endl;
    return;
  }

  // Write the words to the file, one per line
  for (unsigned int i = 0; i < N; ++i) {
    //outFile << "0x" << std::hex << words[i] << std::dec << std::endl;
    outFile << words[i] << "\n";
  }
  
  // Close the file
  outFile.close();
  LOG(TRACE) << "Successfully wrote " << N << " words to " << filename << std::endl;
}
