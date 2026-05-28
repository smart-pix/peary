/**
 * Caribou Device API class header
 */

#ifndef CARIBOU_MIDDLEWARE_H
#define CARIBOU_MIDDLEWARE_H

#include "Device.hpp"
#include "utils/configuration.hpp"
#include "utils/constants.hpp"
#include "utils/dictionary.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace caribou {

  /** Forward declaration of the hardware abstraction layer, not including the header file!
   */
  class caribouHAL;

  /** Caribou Device class definition
   *
   *  this is the central device class from which all device implementations inherit.
   *
   *  Some basic functionality is defined via purely virtual member functions which
   *  have to be implemented by every device instance. This enables the possibility
   *  of interfacing the devices independently via the common set of function alls, e.g.,
   *  from a GUI or a commandline interface.
   */
  template <typename B, typename SCI, typename DRI = SCI> class CaribouDevice : public Device {

  public:
    /** Default constructor for Caribou devices
     *
     */
    CaribouDevice(const caribou::Configuration& config, const typename SCI::configuration_type& sciConfig);

    /** Default constructor for Caribou devices with the Data readout interface
     *
     */
    CaribouDevice(const caribou::Configuration config,
                  const typename SCI::configuration_type sciConfig,
                  const typename DRI::configuration_type driConfig);

    /** Return the firmware version string for reference
     */
    std::string getFirmwareVersion() override;

    /** Return the human-readable device name
     */
    std::string getName() override { return getType(); };

    /** Return the human-readable device type
     */
    std::string getType() override;

    /** Return the identifier of the firmware currently loaded
     */
    uint8_t getFirmwareID();

    /** Return the identifier of the installed board
     */
    uint8_t getBoardID();

    /** Read and return the hardware revision
     */
    uint8_t getBoardRev();

    /** Read byte from onboard memory
     */
    uint8_t readEEPROM(uint16_t addr);

    /** Write byte to onboard memory
     */
    void writeEEPROM(uint16_t addr, int val);

    /** Read the ID from the chip board if available
     *
     *  Some chip boards feature an EPROM which stores a board ID and thus
     *  allows identification of the attached chip board.
     */
    uint16_t getChipID() { return 0; };

    /** Return the human-readable device name of the firmware currently loaded
     */
    std::string getDeviceName();

    /** Call the device's powerUp() function and toggle the powered state
     */
    void powerOn() override;

    /** Turn on the power supply for the attached device
     */
    virtual void powerUp() = 0;

    /** Call the device's powerDown() function and toggle the powered state
     */
    void powerOff() override;

    /** Turn off power for the attached device
     */
    virtual void powerDown() = 0;

    /** Start the data acquisition
     */
    void daqStart() override = 0;

    /** Stop the data acquisition
     */
    void daqStop() override = 0;

    void configure() override;

    // Controlling the device

    /**
     * @brief Set a register on this device
     * @param name  Name of the register
     * @param value Value to be set
     */
    void setRegister(std::string name, uintptr_t value) override;
    virtual void setSpecialRegister(const std::string&, uintptr_t){};
    virtual uintptr_t getSpecialRegister(const std::string&) { return 0; };
    uintptr_t getRegister(std::string name) override;
    std::vector<std::pair<std::string, uintptr_t>> getRegisters() override;

    /** Sending reset signal to the device
     */
    void reset() override;

    // Setting the acquisition clock/device clock?
    // Could be either the supplied clock from DAQ or internal clock divider...
    // virtual void setClockFrequency();

    // Voltage regulators

    // To set supply voltages, same question as above: how to define voltage names?
    // Separate functions to set target voltage and activate?
    // Purely virtual?
    // Do they need to be virtual? Maybe implement in baseclass (always same for CaR)
    // and only look up correct regulator according to name from child class dictionary?
    void setVoltage(std::string name, double voltage, double currentlimit) override;
    void setVoltage(std::string name, double voltage) override { setVoltage(name, voltage, 3.); };
    void setCurrent(std::string name, unsigned int current, bool polarity) override;

    void setOutputCMOSLevel(double voltage);
    void setInputCMOSLevel(double voltage);

    void switchOn(std::string name) override { return switchPeripheryComponent(name, true); };
    void switchOff(std::string name) override { return switchPeripheryComponent(name, false); };

    double getVoltage(std::string name) override;
    double getCurrent(std::string name) override;
    double getPower(std::string name) override;
    bool getAlertStatus(const std::string& name);
    void monitorAlertStatus();
    // virtual double getTemperature();

    /** Read slow-ADC value by name of the input signal as defined by the device
     *
     *  Returns value in SI Volts
     */
    double getADC(std::string name) override;

    std::vector<std::string> listMemories() override;
    std::vector<std::string> listRegisters() override;
    std::vector<std::pair<std::string, std::string>> listComponents() override;

    /** Read slow-ADC value by the input channel number of the ADC device
     *
     *  Returns value in SI Volts
     */
    double getADC(uint8_t channel) override;

    // board CMOS signals
    // void enableSignal();
    // void disableSignal();

    // Retrieving data

    // Raw and decoded data readback
    pearyRawData getRawData() override;
    pearyRawDataVector getRawData(const unsigned int noFrames) override;
    pearydata getData() override;
    pearydataVector getData(const unsigned int noFrames) override;

    // Two types:
    //  * trigger based: "events" are returned
    //  * shutter based: "frames" are returned
    // Both contain pixel(s), timestamp(s)
    // virtual std::vector<caribou::event> getData();
    // If no data available, throw caribou::NoDataAvailable exception instead of returning empty vector!
    // Otherwise synchronization of event-based detectors impossible

    void setMemory(const std::string& name, size_t offset, uintptr_t value) override;
    void setMemory(const std::string& name, uintptr_t value) override;

    template <typename D = uintptr_t> D getMemory(const std::string& name, size_t offset);

    uintptr_t getMemory(const std::string& name, size_t offset) override { return getMemory<uintptr_t>(name, offset); }

    template <typename D = uintptr_t> D getMemory(const std::string& name);

    uintptr_t getMemory(const std::string& name) override { return getMemory<uintptr_t>(name); }

    std::vector<std::pair<std::string, uintptr_t>> getMemories() override;

  protected:
    /**
     * @brief process registers, ingoring sepcial flags
     * @brief reg Register
     * @param value Value of the register to be set
     */
    void process_register_write(register_t<typename SCI::reg_type, typename SCI::data_type> reg, uintptr_t value);

    /**
     * @brief process reading from registers, ingoring sepcial flags
     * @param reg Register
     */
    uintptr_t process_register_read(register_t<typename SCI::reg_type, typename SCI::data_type> reg);

    /** Instance of the Caribou hardware abstraction layer library
     *
     *  All register and hardware access should go through this interface.
     */
    std::shared_ptr<B> _hal;

    /** Device configuration object
     */
    caribou::Configuration _config;

    /** Register dictionary for the devices:
     */
    caribou::dictionary<register_t<typename SCI::reg_type, typename SCI::data_type>> _registers;

    /** Register cache
     */
    std::map<std::string, typename SCI::data_type> _register_cache;

    /** Periphery dictionary to access board components:
     */
    caribou::dictionary<component_t> _periphery;

    /** Memory page dictionary to access FPGA registers:
     */
    caribou::dictionary<std::pair<memory_map, register_t<std::uintptr_t, std::uintptr_t>>> _memory;

    /** Slow control interface configuration
     */
    const typename SCI::configuration_type _sciConfig;

    /** Data Readout interface configuration
     */
    const typename DRI::configuration_type _driConfig;

    // Write data to a device which does not contain internal register
    // If readout is intergralpart of write operations, the read values a returned by function.
    typename SCI::data_type send(const typename SCI::data_type& data);

    // Write data to a device which does not contain internal register
    // If readout is intergralpart of write operations, the read values a returned by function.
    typename SCI::dataVector_type send(const std::vector<typename SCI::data_type>& data);

    // Write data to a device containing internal registers
    // If readout is intergralpart of write operations, the read values a returned by function.
    std::pair<typename SCI::reg_type, typename SCI::data_type>
    send(const std::pair<typename SCI::reg_type, typename SCI::data_type>& data);

    // Write data to a device containing internal registers
    // If readout is intergralpart of write operations, the read values a returned by function.
    typename SCI::dataVector_type send(const typename SCI::reg_type& reg, const std::vector<typename SCI::data_type>& data);

    // Write data to a device containing internal registers
    // If readout is intergralpart of write operations, the read values a returned by function.
    std::vector<std::pair<typename SCI::reg_type, typename SCI::data_type>>
    send(const std::vector<std::pair<typename SCI::reg_type, typename SCI::data_type>>& data);

    // Read data from a device containing internal registers
    typename SCI::dataVector_type receive(const typename SCI::reg_type reg, const unsigned int length = 1);

    // Read data from a readout interface which does not contain internal register
    typename DRI::dataVector_type receiveData(const unsigned int length = 1);

    // Read data from a readout interface containing internal registers
    typename DRI::dataVector_type receiveData(const typename DRI::reg_type reg, const unsigned int length = 1);

    // Apply mask on the regval to be written
    template <typename reg_type, typename data_type>
    data_type obey_mask_write(register_t<reg_type, data_type> reg, data_type regval, data_type regcurrval) const;

    // Apply mask on the read regval
    template <typename reg_type, typename data_type>
    data_type obey_mask_read(register_t<reg_type, data_type> reg, data_type regval) const;

  private:
    /** State indicating powering of the device
     */
    bool _is_powered;

    /** State indicating the configuration of the device
     */
    bool _is_configured;

    /** Switcher function for periphery component (turns them on/off)
     */
    void switchPeripheryComponent(const std::string& name, bool enable);

  }; // class CaribouDevice

} // namespace caribou

#include "CaribouDevice.tcc"

#endif /* CARIBOU_MIDDLEWARE_H */
