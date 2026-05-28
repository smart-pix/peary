#ifndef CARIBOU_HAL_I2C_HPP
#define CARIBOU_HAL_I2C_HPP

#include <vector>

#include "interfaces/Interface.hpp"
#include "interfaces/InterfaceManager.hpp"
#include "utils/exceptions.hpp"

namespace caribou {

  using i2c_address_t = uint8_t;
  using i2c_t = uint8_t;
  using i2c_reg_t = uint8_t;

  class iface_i2c_config : public InterfaceConfiguration {
  public:
    iface_i2c_config(std::string const& devpath, const i2c_address_t devaddress);

    i2c_address_t _devaddress;

    virtual bool operator<(const iface_i2c_config& rhs) const;
  };

  /**
   * @ingroup Interfaces
   * @brief I2C interface via Kernel I2C module
   */
  class iface_i2c : public Interface<i2c_reg_t, i2c_t, iface_i2c_config> {
  private:
    /**
     * @brief Private constructor, only to be created by the interface manager. Opens the file descriptor for the I2C module
     * @param device_path Path the device listens at
     * @throws DeviceException if device cannot be reached
     */
    explicit iface_i2c(const configuration_type& config);

    /**
     * @brief Destructor closes the file handle for the I2C module
     */
    ~iface_i2c() override;

    /**
     * @brief Sets endpoint address before communication
     * @param address I2C address to address
     * @throws CommunicationError if device cannot be contacted
     */
    inline void setAddress(i2c_address_t const address);

    // File descriptor for the I2C module
    int i2cDesc;

    i2c_address_t const _devAddress;

    // caribouHAL is allowed to access private members for reading and writing
    GENERATE_FRIENDS()

  public:
    /**
     * @brief Default constructor deleted, cannot have I2C interface without bus address
     */
    iface_i2c() = delete;

    i2c_address_t devAddress() const { return _devAddress; }

    i2c_t write(const i2c_t& data) override;
    std::pair<i2c_reg_t, i2c_t> write(const std::pair<i2c_reg_t, i2c_t>& data) override;
    dataVector_type write(const i2c_reg_t& reg, const dataVector_type& data) override;

    // length must be 1
    dataVector_type read(const unsigned int length) override;
    // length must be 32
    dataVector_type read(const i2c_reg_t& reg, const unsigned int length) override;

  private:
    // Special functions to read/write to devices with up to 16bit register
    dataVector_type wordwrite(const uint16_t& reg, const dataVector_type& data);
    dataVector_type wordread(const uint16_t reg, const unsigned int length);

    bool probe(const i2c_reg_t& reg) const;

    // only this function can create the interface
    friend iface_i2c& InterfaceManager::getInterface<iface_i2c>(const configuration_type&);

    // only this function can delete the interface
    friend void InterfaceManager::deleteInterface<iface_i2c>(iface_i2c*);
  };

} // namespace caribou

#endif /* CARIBOU_HAL_I2C_HPP */
