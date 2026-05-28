/** Caribou bare device example implementation
 */

#include "ExampleCaribouDevice.hpp"
#include "utils/log.hpp"

using namespace caribou;

ExampleCaribouDevice::ExampleCaribouDevice(caribou::Configuration config) : Device(config) {

  _dispatcher.add("frobicate", &ExampleCaribouDevice::frobicate, this);
  _dispatcher.add("unfrobicate", &ExampleCaribouDevice::unfrobicate, this);
}

ExampleCaribouDevice::~ExampleCaribouDevice() {
  LOG(INFO) << "Shutdown, delete device";
}

std::string ExampleCaribouDevice::getFirmwareVersion() {
  return "42.23alpha";
}

std::string ExampleCaribouDevice::getType() {
  return PEARY_DEVICE_NAME;
}

void ExampleCaribouDevice::powerOn() {
  LOG(INFO) << "Power on";
}

void ExampleCaribouDevice::powerOff() {
  LOG(INFO) << "Power off";
}

void ExampleCaribouDevice::daqStart() {
  LOG(INFO) << "DAQ started";
}

void ExampleCaribouDevice::daqStop() {
  LOG(INFO) << "DAQ stopped";
}

pearyRawData ExampleCaribouDevice::getRawData() {
  return {0u, 2u, 4u, 1u, 3u, 5u};
}

pearyRawDataVector ExampleCaribouDevice::getRawData(const unsigned int noFrames) {
  pearyRawDataVector data;
  for(auto i = 0u; i < noFrames; ++i)
    data.push_back(getRawData());
  return data;
}

pearydata ExampleCaribouDevice::getData() {
  pearydata x;
  x[{0u, 0u}] = std::make_unique<pixel>();
  x[{8u, 16u}] = std::make_unique<pixel>();
  return x;
}

pearydataVector ExampleCaribouDevice::getData(const unsigned int noFrames) {
  pearydataVector data;
  for(auto i = 0u; i < noFrames; ++i)
    data.push_back(getData());
  return data;
}

std::vector<std::pair<std::string, uintptr_t>> ExampleCaribouDevice::getRegisters() {
  return {{"reg0", 0u}, {"reg1", 1u}};
}

// custom device functionality exported via the dispatcher

int32_t ExampleCaribouDevice::frobicate(int32_t a) {
  return (a << 2) & 0b010101010101010101010101010101;
}

std::string ExampleCaribouDevice::unfrobicate(int32_t b) {
  return "blub" + std::to_string(b);
}
