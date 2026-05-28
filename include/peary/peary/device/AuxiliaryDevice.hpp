/**
 * Caribou Auxiliary Device header
 */

#ifndef CARIBOU_DEVICE_AUXILIARY_H
#define CARIBOU_DEVICE_AUXILIARY_H

#include "Device.hpp"
#include "interfaces/Interface.hpp"
#include "interfaces/InterfaceManager.hpp"

#include "utils/configuration.hpp"

#include <string>
#include <vector>

namespace caribou {

  /** Caribou Auxiliary Device class definition
   *
   */
  template <typename T> class AuxiliaryDevice : public Device {

  public:
    /** Default constructor for Caribou devices
     *
     */
    AuxiliaryDevice(const caribou::Configuration config, const typename T::configuration_type interfaceConfig);

    /** Default destructor for Caribou devices
     */
    virtual ~AuxiliaryDevice();

    /** Return the human-readable device name
     */
    virtual std::string getName() override { return getType(); };

    /** Return the human-readable device type
     */
    std::string getType() override;

    virtual std::string getFirmwareVersion() override { return std::string(); };

    // Raw and decoded data readback
    pearyRawData getRawData() override;
    pearyRawDataVector getRawData(const unsigned int noFrames) override;
    pearydata getData() override;
    pearydataVector getData(const unsigned int noFrames) override;

    virtual void powerOn() override{};
    virtual void powerOff() override{};

    virtual void daqStart() override{};
    virtual void daqStop() override{};
    virtual void setRegister(std::string, uintptr_t) override{};
    virtual uintptr_t getRegister(std::string) override { return uintptr_t(); };
    virtual std::vector<std::pair<std::string, uintptr_t>> getRegisters() override {
      return std::vector<std::pair<std::string, uintptr_t>>();
    };
    virtual std::vector<std::string> listMemories() override { return std::vector<std::string>(); }
    virtual std::vector<std::string> listRegisters() override { return std::vector<std::string>(); }
    virtual std::vector<std::pair<std::string, std::string>> listComponents() override {
      return std::vector<std::pair<std::string, std::string>>();
    };

    virtual void setVoltage(std::string, double, double) override{};
    virtual void setVoltage(std::string, double) override{};

    virtual void switchOn(std::string) override{};
    virtual void switchOff(std::string) override{};
    virtual void setCurrent(std::string, unsigned int, bool) override{};

    virtual double getVoltage(std::string) override { return double(); };
    virtual double getCurrent(std::string) override { return double(); };
    virtual double getPower(std::string) override { return double(); };
    virtual double getADC(std::string) override { return double(); };
    virtual double getADC(uint8_t) override { return double(); };

    virtual void configure() override{};

    // Controlling the device
    virtual void set(std::string, uintptr_t){};
    virtual double get(std::string) { return double(); };

    /** Sending reset signal to the device
     */
    virtual void reset() override{};

    virtual void setMemory(const std::string&, size_t, uintptr_t) override{};
    virtual void setMemory(const std::string&, uintptr_t) override{};
    virtual uintptr_t getMemory(const std::string&, size_t) override { return uintptr_t(); };
    virtual uintptr_t getMemory(const std::string&) override { return uintptr_t(); };
    virtual std::vector<std::pair<std::string, uintptr_t>> getMemories() override {
      return std::vector<std::pair<std::string, uintptr_t>>();
    };

  protected:
    // Write data to a device which does not contain internal register
    // If readout is intergralpart of write operations, the read values a returned by function.
    typename T::data_type send(const typename T::data_type& data);

    // Write data to a device which does not contain internal register
    // If readout is intergralpart of write operations, the read values a returned by function.
    std::vector<typename T::data_type> send(const std::vector<typename T::data_type>& data);

    // Write data to a device containing internal registers
    // If readout is intergralpart of write operations, the read values a returned by function.
    std::pair<typename T::reg_type, typename T::data_type>
    send(const std::pair<typename T::reg_type, typename T::data_type>& data);

    // Write data to a device containing internal registers
    // If readout is intergralpart of write operations, the read values a returned by function.
    std::vector<typename T::data_type> send(const typename T::reg_type& reg, const std::vector<typename T::data_type>& data);

    // Write data to a device containing internal registers
    // If readout is intergralpart of write operations, the read values a returned by function.
    std::vector<std::pair<typename T::reg_type, typename T::data_type>>
    send(const std::vector<std::pair<typename T::reg_type, typename T::data_type>>& data);

    // Read data from a device which does not contain internal register
    typename T::dataVector_type receive(const unsigned int length = 1);

    // Read data from a device containing internal registers
    typename T::dataVector_type receive(const typename T::reg_type reg, const unsigned int length = 1);

    /** Device configuration object
     */
    caribou::Configuration _config;

    /** interface configuration
     */
    const typename T::configuration_type _interfaceConfig;
  }; // class AuxiliaryDevice

} // namespace caribou

#include "AuxiliaryDevice.tcc"

#endif /* CARIBOU_DEVICE_AUXILIARY_H */
