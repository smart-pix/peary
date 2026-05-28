/**
 * Caribou Device API class implementation
 */

#ifndef CARIBOU_MIDDLEWARE_IMPL
#define CARIBOU_MIDDLEWARE_IMPL

#include "Device.hpp"
#include "hardware_abstraction/HAL.hpp"
#include "utils/constants.hpp"
#include "utils/dictionary.hpp"
#include "utils/log.hpp"

#include <string>

namespace caribou {

  template <typename B, typename SCI, typename DRI>
  CaribouDevice<B, SCI, DRI>::CaribouDevice(const caribou::Configuration& config,
                                            const typename SCI::configuration_type& sciConfig)
      : CaribouDevice(config, sciConfig, sciConfig) {}

  template <typename B, typename SCI, typename DRI>
  CaribouDevice<B, SCI, DRI>::CaribouDevice(const caribou::Configuration config,
                                            const typename SCI::configuration_type sciConfig,
                                            const typename DRI::configuration_type driConfig)
      : Device(config), _hal(nullptr), _config(config), _registers("Registers"), _periphery("Component"),
        _memory("Memory page"), _sciConfig(sciConfig), _driConfig(driConfig), _is_powered(false), _is_configured(false) {

    SCI& dev_ifaceSCI = InterfaceManager::getInterface<SCI>(_sciConfig);
    LOG(DEBUG) << "Prepared HAL for accessing device with slow control interface at " << dev_ifaceSCI.devicePath();

    DRI& dev_ifaceDRI = InterfaceManager::getInterface<DRI>(_driConfig);
    LOG(DEBUG) << "Prepared HAL for accessing device with readout interface at " << dev_ifaceDRI.devicePath();

    _hal = std::make_shared<B>();

    _dispatcher.add("monitorAlertStatus", &CaribouDevice::monitorAlertStatus, this);
    _dispatcher.add("writeEEPROM", &CaribouDevice::writeEEPROM, this);
    _dispatcher.add("readEEPROM", &CaribouDevice::readEEPROM, this);
  }

  template <typename B, typename SCI, typename DRI> std::string CaribouDevice<B, SCI, DRI>::getType() {
#ifdef PEARY_DEVICE_NAME
    return PEARY_DEVICE_NAME;
#else
    return "";
#endif
  }

  template <typename B, typename SCI, typename DRI> std::string CaribouDevice<B, SCI, DRI>::getFirmwareVersion() {
    return _hal->getFirmwareVersion();
  }

  template <typename B, typename SCI, typename DRI> uint8_t CaribouDevice<B, SCI, DRI>::getBoardID() {
    return _hal->getBoardID();
  }

  template <typename B, typename SCI, typename DRI> uint8_t CaribouDevice<B, SCI, DRI>::getBoardRev() {
    return _hal->getBoardRev();
  }

  template <typename B, typename SCI, typename DRI> uint8_t CaribouDevice<B, SCI, DRI>::getFirmwareID() {
    return _hal->getFirmwareRegister(ADDR_FW_ID);
  }

  template <typename B, typename SCI, typename DRI> uint8_t CaribouDevice<B, SCI, DRI>::readEEPROM(uint16_t addr) {
    return _hal->readEEPROM(addr);
  }

  template <typename B, typename SCI, typename DRI> void CaribouDevice<B, SCI, DRI>::writeEEPROM(uint16_t addr, int val) {
    _hal->writeEEPROM(addr, val);
  }

  template <typename B, typename SCI, typename DRI> std::string CaribouDevice<B, SCI, DRI>::getDeviceName() {
    return std::string();
  }

  template <typename B, typename SCI, typename DRI> void CaribouDevice<B, SCI, DRI>::powerOn() {
    if(_is_powered) {
      LOG(WARNING) << "Device " << getName() << " already powered.";
    } else {
      this->powerUp();
      _is_powered = true;
    }
  }

  template <typename B, typename SCI, typename DRI> void CaribouDevice<B, SCI, DRI>::powerOff() {
    if(!_is_powered) {
      LOG(WARNING) << "Device " << getName() << " already off.";
    } else {
      this->powerDown();
      _is_powered = false;
      _is_configured = false;
    }
  }

  template <typename B, typename SCI, typename DRI>
  void CaribouDevice<B, SCI, DRI>::setVoltage(std::string name, double voltage, double currentlimit) {

    // Resolve name against periphery dictionary
    std::shared_ptr<component_t> ptr = _periphery.get<component_t>(name);
    LOG(DEBUG) << "Voltage to be configured: " << name << " on " << ptr->name();

    if(std::dynamic_pointer_cast<VOLTAGE_REGULATOR_T>(ptr)) {
      // Voltage regulators
      _hal->setVoltageRegulator(*std::dynamic_pointer_cast<VOLTAGE_REGULATOR_T>(ptr), voltage, currentlimit);
    } else if(std::dynamic_pointer_cast<BIAS_REGULATOR_T>(ptr)) {
      // Bias regulators
      _hal->setBiasRegulator(*std::dynamic_pointer_cast<BIAS_REGULATOR_T>(ptr), voltage);
    } else if(std::dynamic_pointer_cast<DCDC_CONVERTER_T>(ptr)) {
      // DC/DC converter
      _hal->setDCDCConverter(*std::dynamic_pointer_cast<DCDC_CONVERTER_T>(ptr), voltage);
    } else {
      throw ConfigInvalid("HAL does not provide a voltage configurator for component \"" + name + "\"");
    }
  }

  template <typename B, typename SCI, typename DRI> void CaribouDevice<B, SCI, DRI>::setOutputCMOSLevel(double voltage) {
    _hal->setOutputCMOSLevel(voltage);
  }
  template <typename B, typename SCI, typename DRI> void CaribouDevice<B, SCI, DRI>::setInputCMOSLevel(double voltage) {
    _hal->setInputCMOSLevel(voltage);
  }

  template <typename B, typename SCI, typename DRI>
  void CaribouDevice<B, SCI, DRI>::switchPeripheryComponent(const std::string& name, bool enable) {

    // Resolve name against periphery dictionary
    std::shared_ptr<component_t> ptr = _periphery.get<component_t>(name);
    LOG(DEBUG) << "Periphery to be switched " << (enable ? "on" : "off") << ": " << name << " on " << ptr->name();

    // Send command to appropriate switches via HAL
    if(std::dynamic_pointer_cast<VOLTAGE_REGULATOR_T>(ptr)) {
      // Voltage regulators
      _hal->powerVoltageRegulator(*std::dynamic_pointer_cast<VOLTAGE_REGULATOR_T>(ptr), enable);
    } else if(std::dynamic_pointer_cast<BIAS_REGULATOR_T>(ptr)) {
      // Bias regulators
      _hal->powerBiasRegulator(*std::dynamic_pointer_cast<BIAS_REGULATOR_T>(ptr), enable);
    } else if(std::dynamic_pointer_cast<DCDC_CONVERTER_T>(ptr)) {
      // DC/DC converter
      _hal->powerDCDCConverter(*std::dynamic_pointer_cast<DCDC_CONVERTER_T>(ptr), enable);
    } else if(std::dynamic_pointer_cast<CURRENT_SOURCE_T>(ptr)) {
      // Current sources
      _hal->powerCurrentSource(*std::dynamic_pointer_cast<CURRENT_SOURCE_T>(ptr), enable);
    } else {
      throw ConfigInvalid("HAL does not provide a switch for component \"" + name + "\"");
    }
  }



  template <typename B, typename SCI, typename DRI> double CaribouDevice<B, SCI, DRI>::getVoltage(std::string name) {

    // Resolve name against periphery dictionary
    std::shared_ptr<component_t> ptr = _periphery.get<component_t>(name);
    LOG(DEBUG) << "Voltage to be measured: " << name << " on " << ptr->name();

    // Send command to appropriate switches via HAL
    if(std::dynamic_pointer_cast<VOLTAGE_REGULATOR_T>(ptr)) {
      // Voltage regulators
      return _hal->measureVoltage(*std::dynamic_pointer_cast<VOLTAGE_REGULATOR_T>(ptr));
    } else if(std::dynamic_pointer_cast<BIAS_REGULATOR_T>(ptr)) {
      // Bias regulators
      return _hal->readBias(*std::dynamic_pointer_cast<BIAS_REGULATOR_T>(ptr));
      //LOG(WARNING) << "_hal->readBias is not implemented in the aq_dev branch of peary...";
    } else if(std::dynamic_pointer_cast<SLOW_ADC_CHANNEL_T>(ptr)) {
      // Slow ADC Channels
      return _hal->readSlowADC(*std::dynamic_pointer_cast<SLOW_ADC_CHANNEL_T>(ptr));
    } else {
      throw ConfigInvalid("HAL does not provide voltage measurements for component \"" + name + "\"");
    }
  }






  template <typename B, typename SCI, typename DRI>
  bool CaribouDevice<B, SCI, DRI>::getAlertStatus(const std::string& name) {

    // Resolve name against periphery dictionary
    std::shared_ptr<GPIO_T> ptr = _periphery.get<GPIO_T>(name);
    LOG(DEBUG) << "Alert status probed : " << name;

    // Send command to monitor via HAL
    return _hal->getAlertStatus(*ptr);
  }

  template <typename B, typename SCI, typename DRI> void CaribouDevice<B, SCI, DRI>::monitorAlertStatus() {
    // Send command to monitor via HAL
    _hal->monitorAlert();
  }

  template <typename B, typename SCI, typename DRI> double CaribouDevice<B, SCI, DRI>::getCurrent(std::string name) {

    // Resolve name against periphery dictionary
    std::shared_ptr<VOLTAGE_REGULATOR_T> ptr = _periphery.get<VOLTAGE_REGULATOR_T>(name);
    LOG(DEBUG) << "Current to be measured: " << name << " on " << ptr->name();

    // Send command to monitor via HAL
    return _hal->measureCurrent(*ptr);
  }

  template <typename B, typename SCI, typename DRI> double CaribouDevice<B, SCI, DRI>::getPower(std::string name) {

    // Resolve name against periphery dictionary
    std::shared_ptr<VOLTAGE_REGULATOR_T> ptr = _periphery.get<VOLTAGE_REGULATOR_T>(name);
    LOG(DEBUG) << "Power to be measured: " << name << " on " << ptr->name();

    // Send command to monitor via HAL
    return _hal->measurePower(*ptr);
  }

  template <typename B, typename SCI, typename DRI> double CaribouDevice<B, SCI, DRI>::getADC(uint8_t channel) {
    try {
      // Get all the available channel names
      auto channel_name = _periphery.getNames<SLOW_ADC_CHANNEL_T>();
      // Resolve channel name agains the vector of all the channel names
      return getADC(channel_name.at(static_cast<size_t>(channel - 1)));
    } catch(const std::out_of_range&) {
      LOG(FATAL) << "ADC channel " << std::to_string(static_cast<int>(channel)) << " does not exist";
      throw caribou::ConfigInvalid("ADC channel " + std::to_string(static_cast<int>(channel)) + " does not exist");
    }
  }

  template <typename B, typename SCI, typename DRI> double CaribouDevice<B, SCI, DRI>::getADC(std::string name) {

    // Resolve name against periphery dictionary
    std::shared_ptr<SLOW_ADC_CHANNEL_T> ptr = _periphery.get<SLOW_ADC_CHANNEL_T>(name);
    LOG(DEBUG) << "ADC channel to be sampled: " << name << " on " << ptr->name();

    // Read slow ADC
    return _hal->readSlowADC(*ptr);
  }

  template <typename B, typename SCI, typename DRI>
  void CaribouDevice<B, SCI, DRI>::setCurrent(std::string name, unsigned int current, bool polarity) {

    // Resolve name against periphery dictionary
    std::shared_ptr<CURRENT_SOURCE_T> ptr = _periphery.get<CURRENT_SOURCE_T>(name);
    LOG(DEBUG) << "Current source to be configured: " << name << " on " << ptr->name();

    auto pol = static_cast<CURRENT_SOURCE_POLARITY_T>(polarity);

    // Send command to current source via HAL
    _hal->setCurrentSource(*ptr, current, pol);
  }

  template <typename B, typename SCI, typename DRI>
  void CaribouDevice<B, SCI, DRI>::setRegister(std::string name, uintptr_t value) {

    // Resolve name against register dictionary:
    register_t<typename SCI::reg_type, typename SCI::data_type> reg = _registers.get(name);

    if(!reg.writable()) {
      throw caribou::RegisterTypeMismatch("Trying to write to register with \"nowrite\" flag: " + name);
    }
    if(reg.special()) {
      // Defer to special register treatment function of the derived classes:
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      setSpecialRegister(name, value);
      return;
    }

    LOG(DEBUG) << "Register to be set: " << name << " (" << to_hex_string(reg.address()) << ")";
    process_register_write(reg, value);

    // Cache the current value of this register:
    _register_cache[name] = static_cast<typename SCI::data_type>(value);
  }

  template <typename B, typename SCI, typename DRI>
  void CaribouDevice<B, SCI, DRI>::process_register_write(register_t<typename SCI::reg_type, typename SCI::data_type> reg,
                                                          uintptr_t value) {

    auto regval = static_cast<typename SCI::data_type>(value);

    auto current_reg = (reg.readable() ? receive(reg.address()).front() : typename SCI::data_type());

    regval = obey_mask_write<typename SCI::reg_type, typename SCI::data_type>(reg, regval, current_reg);

    LOG(DEBUG) << "Register value to be set: " << to_hex_string(regval);
    send(std::make_pair(reg.address(), regval));
  }

  template <typename B, typename SCI, typename DRI>
  template <typename reg_type, typename data_type>
  data_type CaribouDevice<B, SCI, DRI>::obey_mask_write(register_t<reg_type, data_type> reg,
                                                        data_type regval,
                                                        data_type regcurrval) const {
    auto newregval = regval;
    // Obey the mask:
    if(reg.mask() < std::numeric_limits<data_type>::max()) {
      // We need to read the register in order to preserve the nonaffected bits:
      LOG(DEBUG) << "Reg. mask:   " << to_bit_string(reg.mask());
      LOG(DEBUG) << "Shift by:    " << static_cast<int>(reg.shift());
      LOG(DEBUG) << "new_val    = " << to_bit_string(regval);
      LOG(DEBUG) << "value (sh) = " << to_bit_string(static_cast<data_type>(regval << reg.shift()));
      LOG(DEBUG) << "curr_val   = " << to_bit_string(regcurrval);
      newregval = static_cast<data_type>((regcurrval & ~reg.mask()) | ((regval << reg.shift()) & reg.mask()));
      LOG(DEBUG) << "updated    = " << to_bit_string(newregval);
    } else {
      LOG(DEBUG) << "Mask covering full register: " << to_bit_string(reg.mask());
    }

    return newregval;
  }

  template <typename B, typename SCI, typename DRI>
  std::vector<std::pair<std::string, uintptr_t>> CaribouDevice<B, SCI, DRI>::getRegisters() {

    std::vector<std::pair<std::string, uintptr_t>> regvalues;

    // Retrieve all registers from the device:
    std::vector<std::string> regs = _registers.getNames();
    for(const auto& r : regs) {
      try {
        regvalues.push_back(std::make_pair(r, this->getRegister(r)));
        LOG(DEBUG) << "Retrieved register \"" << r << "\" = " << static_cast<int>(regvalues.back().second) << " ("
                   << to_hex_string(regvalues.back().second) << ")";
      } catch(RegisterTypeMismatch& e) {
        LOG(DEBUG) << "Omitting writeonly register \"" << r << "\"";
      } catch(CommunicationError& e) {
        LOG(DEBUG) << "Failed to retrieve register \"" << r << "\" from the device.";
      }
    }

    return regvalues;
  }

  template <typename B, typename SCI, typename DRI> uintptr_t CaribouDevice<B, SCI, DRI>::getRegister(std::string name) {

    // Resolve name against register dictionary:
    register_t<typename SCI::reg_type, typename SCI::data_type> reg = _registers.get(name);

    if(!reg.readable()) {
      // This register cannot be read back from the device:
      throw caribou::RegisterTypeMismatch("Trying to read register with \"noread\" flag: " + name);
    }
    if(reg.special()) {
      // Defer to special register treatment function of the derived classes:
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      return getSpecialRegister(name);
    }

    LOG(DEBUG) << "Register to be read: " << name << " (" << to_hex_string(reg.address()) << ")";
    return process_register_read(reg);
  }

  template <typename B, typename SCI, typename DRI>
  uintptr_t
  CaribouDevice<B, SCI, DRI>::process_register_read(register_t<typename SCI::reg_type, typename SCI::data_type> reg) {

    typename SCI::data_type regval = receive(reg.address()).front();

    // Obey the mask:
    return static_cast<uintptr_t>(obey_mask_read<typename SCI::reg_type, typename SCI::data_type>(reg, regval));
  }

  template <typename B, typename SCI, typename DRI>
  template <typename reg_type, typename data_type>
  data_type CaribouDevice<B, SCI, DRI>::obey_mask_read(register_t<reg_type, data_type> reg, data_type regval) const {
    auto newregval = regval;
    LOG(DEBUG) << "raw value  = " << to_bit_string(regval);
    newregval = static_cast<data_type>(regval & reg.mask());
    LOG(DEBUG) << "masked val = " << to_bit_string(newregval);
    newregval = static_cast<data_type>(newregval >> reg.shift());
    LOG(DEBUG) << "shifted val = " << static_cast<data_type>(newregval);

    return newregval;
  }

  template <typename B, typename SCI, typename DRI> void CaribouDevice<B, SCI, DRI>::reset() {
    LOG(FATAL) << "Reset functionality not implemented for this device";
    throw caribou::DeviceImplException("Reset functionality not implemented for this device");
  }

  // Data return functions, for raw or decoded data
  template <typename B, typename SCI, typename DRI> pearyRawData CaribouDevice<B, SCI, DRI>::getRawData() {
    LOG(FATAL) << "Raw data readback not implemented for this device";
    throw caribou::DeviceImplException("Raw data readback not implemented for this device");
  }

  template <typename B, typename SCI, typename DRI>
  pearyRawDataVector CaribouDevice<B, SCI, DRI>::getRawData(const unsigned int noFrames [[maybe_unused]]) {
    LOG(FATAL) << "Multi-frame raw data readback not implemented for this device";
    throw caribou::DeviceImplException("Multi-frame raw data readback not implemented for this device");
  }

  template <typename B, typename SCI, typename DRI> pearydata CaribouDevice<B, SCI, DRI>::getData() {
    LOG(FATAL) << "Decoded data readback not implemented for this device";
    throw caribou::DeviceImplException("Decoded data readback not implemented for this device");
  }

  template <typename B, typename SCI, typename DRI>
  pearydataVector CaribouDevice<B, SCI, DRI>::getData(const unsigned int noFrames [[maybe_unused]]) {
    LOG(FATAL) << "Decoded data readback not implemented for this device";
    throw caribou::DeviceImplException("Decoded data readback not implemented for this device");
  }

  template <typename B, typename SCI, typename DRI> void CaribouDevice<B, SCI, DRI>::configure() {
    if(!_is_powered) {
      LOG(ERROR) << "Device " << getName() << " is not powered!";
      return;
    }

    // Set all registers provided in the configuratio file, skip those which are not set:
    std::vector<std::string> dacs = _registers.getNames();
    LOG(INFO) << "Setting ASIC registers from configuration:";
    for(const auto& i : dacs) {
      try {
        try {
          auto value = _config.Get<uintptr_t>(i);
          this->setRegister(i, value);
          LOG(INFO) << "Set register \"" << i << "\" = " << value << " (" << to_hex_string(value) << ")";
        } catch(ConfigMissingKey& e) {
          // If this register has no default value, skip:
          if(!_registers.get(i).hasValue()) {
            continue;
          }

          // Register has default value, configure it:
          auto value = _registers.get(i).value();
          this->setRegister(i, value);
          LOG(DEBUG) << "Could not find key \"" << i << "\" in the configuration, setting to default value " << value << " ("
                     << to_hex_string(value) << ")";
        }
      } catch(RegisterTypeMismatch&) {
      }
    }

    LOG(INFO) << "Setting FPGA memory registers from configuration:";
    auto mems = _memory.getNames();
    for(const auto& i : mems) {
      try {
        try {
          auto value = _config.Get<uintptr_t>(i);
          this->setMemory(i, value);
          LOG(INFO) << "Set memory \"" << i << "\" = " << value << " (" << to_hex_string(value) << ")";
        } catch(ConfigMissingKey& e) {
          // If this memory register has no default value, skip:
          if(!_memory.get(i).second.hasValue()) {
            continue;
          }

          // Memory register has default value, configure it:
          auto value = _memory.get(i).second.value();
          this->setMemory(i, value);
          LOG(DEBUG) << "Could not find key \"" << i << "\" in the configuration, setting to default value " << value << " ("
                     << to_hex_string(value) << ")";
        }
      } catch(RegisterTypeMismatch&) {
      }
    }

    _is_configured = true;
  }

  template <typename B, typename SCI, typename DRI>
  void CaribouDevice<B, SCI, DRI>::setMemory(const std::string& name, size_t offset, uintptr_t value) {
    if(!_memory.get(name).first.writable() || !_memory.get(name).second.writable()) {
      throw caribou::RegisterTypeMismatch("Trying to write to memory with \"nowrite\" flag: " + name);
    }

    auto newvalue = obey_mask_write<std::uintptr_t, std::uintptr_t>(
      _memory.get(name).second,
      value,
      _hal->readMemory(_memory.get(name).first, _memory.get(name).second.address() + offset));
    _hal->writeMemory(_memory.get(name).first, _memory.get(name).second.address() + offset, newvalue);
  }

  template <typename B, typename SCI, typename DRI>
  void CaribouDevice<B, SCI, DRI>::setMemory(const std::string& name, uintptr_t value) {
    if(!_memory.get(name).first.writable() || !_memory.get(name).second.writable()) {
      throw caribou::RegisterTypeMismatch("Trying to write to memory with \"nowrite\" flag: " + name);
    }
    auto newvalue = obey_mask_write<std::uintptr_t, std::uintptr_t>(
      _memory.get(name).second, value, _hal->readMemory(_memory.get(name).first, _memory.get(name).second.address()));
    _hal->writeMemory(_memory.get(name).first, _memory.get(name).second.address(), newvalue);
  }

  template <typename B, typename SCI, typename DRI>
  template <typename D>
  D CaribouDevice<B, SCI, DRI>::getMemory(const std::string& name, size_t offset) {
    return static_cast<D>(obey_mask_read<std::uintptr_t, std::uintptr_t>(
      _memory.get(name).second, _hal->readMemory(_memory.get(name).first, _memory.get(name).second.address() + offset)));
  }

  template <typename B, typename SCI, typename DRI>
  template <typename D>
  D CaribouDevice<B, SCI, DRI>::getMemory(const std::string& name) {
    return static_cast<D>(obey_mask_read<std::uintptr_t, std::uintptr_t>(
      _memory.get(name).second, _hal->readMemory(_memory.get(name).first, _memory.get(name).second.address())));
  }

  template <typename B, typename SCI, typename DRI>
  std::vector<std::pair<std::string, uintptr_t>> CaribouDevice<B, SCI, DRI>::getMemories() {

    std::vector<std::pair<std::string, uintptr_t>> memvalues;

    // Retrieve all registers from the device:
    std::vector<std::string> mems = _memory.getNames();
    for(const auto& m : mems) {
      try {
        memvalues.push_back(std::make_pair(m, this->getMemory(m)));
        LOG(DEBUG) << "Retrieved memory \"" << m << "\" = " << static_cast<int>(memvalues.back().second) << " ("
                   << to_hex_string(memvalues.back().second) << ")";
      } catch(RegisterTypeMismatch& e) {
        LOG(DEBUG) << "Omitting writeonly memory \"" << m << "\"";
      } catch(CommunicationError& e) {
        LOG(DEBUG) << "Failed to retrieve memory \"" << m << "\" from the device.";
      }
    }

    return memvalues;
  }

  template <typename B, typename SCI, typename DRI> std::vector<std::string> CaribouDevice<B, SCI, DRI>::listMemories() {
    return _memory.getNames();
  }

  template <typename B, typename SCI, typename DRI> std::vector<std::string> CaribouDevice<B, SCI, DRI>::listRegisters() {
    return _registers.getNames();
  }

  template <typename B, typename SCI, typename DRI>
  std::vector<std::pair<std::string, std::string>> CaribouDevice<B, SCI, DRI>::listComponents() {
    std::vector<std::pair<std::string, std::string>> component_list;
    auto component_names = _periphery.getNames();
    for(const auto& name : component_names) {
      std::shared_ptr<component_t> ptr = _periphery.get<component_t>(name);
      std::string type;
      if(std::dynamic_pointer_cast<VOLTAGE_REGULATOR_T>(ptr)) {
        type = "voltage_reg";
      } else if(std::dynamic_pointer_cast<BIAS_REGULATOR_T>(ptr)) {
        type = "bias_reg";
      } else if(std::dynamic_pointer_cast<CURRENT_SOURCE_T>(ptr)) {
        type = "current_reg";
      } else if(std::dynamic_pointer_cast<DCDC_CONVERTER_T>(ptr)) {
        type = "dcdc_conv";
      } else if(std::dynamic_pointer_cast<SLOW_ADC_CHANNEL_T>(ptr)) {
        type = "adc_channel";
      } else if(std::dynamic_pointer_cast<INJBIAS_REGULATOR_T>(ptr)) {
        type = "injection_bias";
      } else {
        type = "UNKNOWN";
      }
      component_list.emplace_back(name, type);
    }
    return component_list;
  }

  template <typename B, typename SCI, typename DRI>
  typename SCI::data_type CaribouDevice<B, SCI, DRI>::send(const typename SCI::data_type& data) {
    return InterfaceManager::getInterface<SCI>(_sciConfig).write(data);
  }

  template <typename B, typename SCI, typename DRI>
  typename SCI::dataVector_type CaribouDevice<B, SCI, DRI>::send(const std::vector<typename SCI::data_type>& data) {
    return InterfaceManager::getInterface<SCI>(_sciConfig).write(data);
  }

  template <typename B, typename SCI, typename DRI>
  std::pair<typename SCI::reg_type, typename SCI::data_type>
  CaribouDevice<B, SCI, DRI>::send(const std::pair<typename SCI::reg_type, typename SCI::data_type>& data) {
    return InterfaceManager::getInterface<SCI>(_sciConfig).write(data);
  }

  template <typename B, typename SCI, typename DRI>
  typename SCI::dataVector_type CaribouDevice<B, SCI, DRI>::send(const typename SCI::reg_type& reg,
                                                                 const std::vector<typename SCI::data_type>& data) {
    return InterfaceManager::getInterface<SCI>(_sciConfig).write(reg, data);
  }

  template <typename B, typename SCI, typename DRI>
  std::vector<std::pair<typename SCI::reg_type, typename SCI::data_type>>
  CaribouDevice<B, SCI, DRI>::send(const std::vector<std::pair<typename SCI::reg_type, typename SCI::data_type>>& data) {
    return InterfaceManager::getInterface<SCI>(_sciConfig).write(data);
  }

  template <typename B, typename SCI, typename DRI>
  typename SCI::dataVector_type CaribouDevice<B, SCI, DRI>::receive(const typename SCI::reg_type reg,
                                                                    const unsigned int length) {
    return InterfaceManager::getInterface<SCI>(_sciConfig).read(reg, length);
  }

  template <typename B, typename SCI, typename DRI>
  typename DRI::dataVector_type CaribouDevice<B, SCI, DRI>::receiveData(const unsigned int length) {
    return InterfaceManager::getInterface<DRI>(_driConfig).read(length);
  }

  template <typename B, typename SCI, typename DRI>
  typename DRI::dataVector_type CaribouDevice<B, SCI, DRI>::receiveData(const typename DRI::reg_type reg,
                                                                        const unsigned int length) {
    return InterfaceManager::getInterface<DRI>(_driConfig).read(reg, length);
  }

} // namespace caribou

#endif /* CARIBOU_MIDDLEWARE_IMPL */
