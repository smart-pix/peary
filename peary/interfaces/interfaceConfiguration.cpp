#include "interfaceConfiguration.hpp"

using namespace caribou;

InterfaceConfiguration::InterfaceConfiguration(std::string devpath) : _devpath(std::move(devpath)) {}

bool InterfaceConfiguration::operator<(const InterfaceConfiguration& rhs) const {
  return _devpath < rhs._devpath;
}
