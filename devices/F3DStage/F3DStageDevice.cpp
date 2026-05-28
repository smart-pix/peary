#include "F3DStageDevice.hpp"
#include <cmath>

using namespace caribou;

F3DStageDevice::F3DStageDevice(const caribou::Configuration config)
    : AuxiliaryDevice(config, iface_ipsocket::configuration_type(config.Get("devicepath", std::string(DEFAULT_DEVICEPATH)))),
      _registers("Registers") {

  _registers.add(F3DStage_REGISTERS);

  LOG(DEBUG) << "New F3D stage interface, trying to connect...";

  auto firmware = getRegister("firmware");

  LOG(DEBUG) << "Connected to F3D stage.";
  LOG(DEBUG) << "Fastree3D stage firmware version : " << firmware;
}

F3DStageDevice::~F3DStageDevice() {
  LOG(INFO) << "Shutdown, delete device.";
}

void F3DStageDevice::reset() {
  LOG(INFO) << "Resetting the stage";
  setRegister("reset", 0);
}

void F3DStageDevice::configure() {
  std::vector<std::string> regs = _registers.getNames();
  LOG(INFO) << "Setting F3DStage registers from configuration:";
  for(auto i : regs) {
    try {
      uintptr_t value = _config.Get<uintptr_t>(i);
      this->setRegister(i, value);
      LOG(INFO) << "Set register \"" << i << "\" = " << static_cast<int>(value);
    } catch(ConfigMissingKey& e) {
      LOG(DEBUG) << "Could not find key \"" << i << "\" in the configuration, skipping.";
    }
  }
}

void F3DStageDevice::setRegister(std::string name, uintptr_t value) {
  LOG(DEBUG) << "Setting register " << name << " to value " << value;

  auto reg = _registers.get(name);

  if(!reg.writable()) {
    throw caribou::RegisterTypeMismatch("Trying to write to register with \"nowrite\" flag: " + name);
  }

  // The numbers need to be represnted on four digits
  std::string cmd =
    "s" + reg.address() + std::string(3 - static_cast<unsigned int>(std::log10(value)), '0') + std::to_string(value);

  std::string resp = AuxiliaryDevice<iface_ipsocket>::receive(cmd).front();

  if(resp.compare("Done\n"))
    throw DeviceException("Falied to set register " + name + " of the F3DStage to value " + std::to_string(value) + ": " +
                          resp);
};

uintptr_t F3DStageDevice::getRegister(std::string name) {
  LOG(DEBUG) << "Reading register " << name;

  auto reg = _registers.get(name);

  if(!reg.readable()) {
    throw caribou::RegisterTypeMismatch("Trying to read register with \"noread\" flag: " + name);
  }

  std::string cmd = "g" + reg.address();

  std::string resp = AuxiliaryDevice<iface_ipsocket>::receive(cmd).front();
  try {
    return static_cast<uintptr_t>(std::stoi(resp));
  } catch(const std::invalid_argument& e) {
    throw DeviceException("Failed to read register " + name + " of the F3DStage: invalid response:" + resp);
  }
};
