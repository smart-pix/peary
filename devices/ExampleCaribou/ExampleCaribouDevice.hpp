/** Example caribou device
 *
 * Use this class as a starting point for a device that does not rely on
 * an underlying hardware abstraction layer.
 */

#ifndef EXAMPLE_CARIBOU_DEVICE_H
#define EXAMPLE_CARIBOU_DEVICE_H

#include "device/Device.hpp"

namespace caribou {
  /** Example caribou device class definition
   *
   * This class directly implements all purely virtual functions of
   * `caribou::Device` without a hardware abstration layer.
   * Most functions are noops. Applications can then control this
   * device via the Caribou device class interface by using the device
   * manager to instanciate the device object.
   */
  class ExampleCaribouDevice final : public Device {
  public:
    ExampleCaribouDevice(caribou::Configuration config);
    ~ExampleCaribouDevice();

    std::string getFirmwareVersion() override;
    std::string getType() override;
    std::string getName() override { return getType(); }

    // Controll the device
    void reset() override{};
    void configure() override{};
    void powerOn() override;
    void powerOff() override;
    void daqStart() override;
    void daqStop() override;
    // Retrieve data
    pearyRawData getRawData() override;
    pearyRawDataVector getRawData(const unsigned int) override;
    pearydata getData() override;
    pearydataVector getData(const unsigned int) override;
    // Configure the device
    void setRegister(std::string, uintptr_t) override{};
    uintptr_t getRegister(std::string) override { return 0u; };
    std::vector<std::pair<std::string, uintptr_t>> getRegisters() override;
    std::vector<std::string> listMemories() override { return std::vector<std::string>(); }
    std::vector<std::string> listRegisters() override { return std::vector<std::string>(); }
    std::vector<std::pair<std::string, std::string>> listComponents() override {
      return std::vector<std::pair<std::string, std::string>>();
    }
    virtual std::vector<std::pair<std::string, uintptr_t>> getMemories() override {
      return std::vector<std::pair<std::string, uintptr_t>>();
    };
    // Voltage regulators and current sources
    void setVoltage(std::string, double, double) override{};
    void setVoltage(std::string, double) override{};
    void setBias(std::string, double){};
    void setInjectionBias(std::string, double){};
    void switchOn(std::string) override{};
    void switchOff(std::string) override{};
    void setCurrent(std::string, unsigned int, bool) override{};
    double getVoltage(std::string) override { return 0.0; };
    double getCurrent(std::string) override { return 0.0; };
    double getPower(std::string) override { return 0.0; };
    // Slow ADCs
    double getADC(std::string) override { return 0.0; };
    double getADC(uint8_t) override { return 0.0; };

    void setMemory(const std::string&, size_t, uintptr_t) override{};
    void setMemory(const std::string&, uintptr_t) override{};
    uintptr_t getMemory(const std::string&, size_t) override { return uintptr_t(); };
    uintptr_t getMemory(const std::string&) override { return uintptr_t(); };

  private:
    int32_t frobicate(int32_t a);
    std::string unfrobicate(int32_t b);
  };

} // namespace caribou

#endif /* EXAMPLE_CARIBOU_DEVICE_H */
