/**
 * Caribou Auxiliary implementation
 */

#ifndef CARIBOU_DEVICE_AUXILIARY_IMPL
#define CARIBOU_DEVICE_AUXILIARY_IMPL

namespace caribou {

  template <typename T>

  AuxiliaryDevice<T>::AuxiliaryDevice(const caribou::Configuration config,
                                      const typename T::configuration_type interfaceConfig)
      : Device(config), _config(config), _interfaceConfig(interfaceConfig) {

    T& dev_iface = InterfaceManager::getInterface<T>(_interfaceConfig);
    LOG(DEBUG) << "Auxiliary device initialized at " << dev_iface.devicePath();
  }

  template <typename T> AuxiliaryDevice<T>::~AuxiliaryDevice() {}

  template <typename T> std::string AuxiliaryDevice<T>::getType() {
#ifdef PEARY_DEVICE_NAME
    return PEARY_DEVICE_NAME;
#else
    return "";
#endif
  }

  // Data return functions, for raw or decoded data
  template <typename T> pearyRawData AuxiliaryDevice<T>::getRawData() {
    LOG(FATAL) << "Raw data readback not implemented for this device";
    throw caribou::DeviceImplException("Raw data readback not implemented for this device");
  }

  template <typename T> pearyRawDataVector AuxiliaryDevice<T>::getRawData(const unsigned int noFrames [[maybe_unused]]) {
    LOG(FATAL) << "Multi-frame raw data readback not implemented for this device";
    throw caribou::DeviceImplException("Multi-frame raw data readback not implemented for this device");
  }

  template <typename T> pearydata AuxiliaryDevice<T>::getData() {
    LOG(FATAL) << "Decoded data readback not implemented for this device";
    throw caribou::DeviceImplException("Decoded data readback not implemented for this device");
  }

  template <typename T> pearydataVector AuxiliaryDevice<T>::getData(const unsigned int noFrames [[maybe_unused]]) {
    LOG(FATAL) << "Decoded data readback not implemented for this device";
    throw caribou::DeviceImplException("Decoded data readback not implemented for this device");
  }

  template <typename T> typename T::data_type AuxiliaryDevice<T>::send(const typename T::data_type& data) {
    return InterfaceManager::getInterface<T>(_interfaceConfig).write(data);
  }

  template <typename T>
  std::vector<typename T::data_type> AuxiliaryDevice<T>::send(const std::vector<typename T::data_type>& data) {
    return InterfaceManager::getInterface<T>(_interfaceConfig).write(data);
  }

  template <typename T>
  std::pair<typename T::reg_type, typename T::data_type>
  AuxiliaryDevice<T>::send(const std::pair<typename T::reg_type, typename T::data_type>& data) {
    return InterfaceManager::getInterface<T>(_interfaceConfig).write(data);
  }

  template <typename T>
  std::vector<typename T::data_type> AuxiliaryDevice<T>::send(const typename T::reg_type& reg,
                                                              const std::vector<typename T::data_type>& data) {
    return InterfaceManager::getInterface<T>(_interfaceConfig).write(reg, data);
  }

  template <typename T>
  std::vector<std::pair<typename T::reg_type, typename T::data_type>>
  AuxiliaryDevice<T>::send(const std::vector<std::pair<typename T::reg_type, typename T::data_type>>& data) {
    return InterfaceManager::getInterface<T>(_interfaceConfig).write(data);
  }

  template <typename T> typename T::dataVector_type AuxiliaryDevice<T>::receive(const unsigned int length) {
    return InterfaceManager::getInterface<T>(_interfaceConfig).read(length);
  }

  template <typename T>
  typename T::dataVector_type AuxiliaryDevice<T>::receive(const typename T::reg_type reg, const unsigned int length) {
    return InterfaceManager::getInterface<T>(_interfaceConfig).read(reg, length);
  }

} // namespace caribou

#endif /* CARIBOU_DEVICE_AUXILIARY_IMPL */
