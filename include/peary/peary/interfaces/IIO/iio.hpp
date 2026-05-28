#ifndef CARIBOU_HAL_IIO_HPP
#define CARIBOU_HAL_IIO_HPP

#include "interfaces/Interface.hpp"
#include "interfaces/InterfaceManager.hpp"
#include "utils/datatypes.hpp"
#include "utils/exceptions.hpp"

#include <map>
#ifndef IIO_EMULATED
#include "iio.h"
#endif

namespace caribou {

  using iio_converter_t = component_converter_t;
  using iio_reg_t = uint8_t;

  struct iio_t {
    // command's type:
    // - Set value in the device
    // - enable the device
    enum { ENABLE, SET } cmd;

    union {
      // enable/disable the channel
      bool enable;

      /** Voltage/Current of a given channel in SI Volts/Ampers
       */
      double value = 0;
    };
  };

  enum iface_iio_direction_t : bool { read = false, write = true };

  class iface_iio_config : public InterfaceConfiguration {
  public:
    iface_iio_config(const std::string& devpath, std::string chantype, const component_dac_t& dac);
    iface_iio_config(const std::string& devpath, std::string chantype, const component_adc_t& adc);

    iio_converter_t _converter;
    std::string _chantype;
    iface_iio_direction_t _direction;

    virtual bool operator<(const iface_iio_config& rhs) const;
  };

  // The interface to serve simple devices (e.g. DACs) thorugh IIO library
  class iface_iio : public Interface<iio_reg_t, iio_t, iface_iio_config> {

  private:
    iio_converter_t const _converter;

    std::string const _chantype;

    iface_iio_direction_t const _direction;

  protected:
    // Default constructor: private (only created by InterfaceManager)
    // It can throw DeviceException
    explicit iface_iio(const configuration_type& config);

    ~iface_iio() override;

#ifndef IIO_EMULATED

    // IIO context used by the interface
    struct iio_context* iio_ctx;

    // The iio channel
    iio_channel* _channel;

#endif

    // Indicates whetehr DAC (direction==write) channel is activate
    bool _isOn;

    // Voltage requested for the given DAC (direction==write) channel
    double _voltage;

    void getIIOChannel();
    void setIIOChannel();
    void powerDownIIOChannel();

    GENERATE_FRIENDS()

  public:
    // Returns the IIO chanel type
    std::string chanType() const {
      return _chantype;
    }

    // Returns coveter channel served by this interface
    iio_converter_t converter() const {
      return _converter;
    }

    iface_iio_direction_t direction() const {
      return _direction;
    }

    // Function used by DAC
    iio_t write(const iio_t&) override;

    // Function used by ADC
    iio_t read() override;

    // Unused constructor
    iface_iio() = delete;

    // only this function can create the interface
    friend iface_iio& InterfaceManager::getInterface<iface_iio>(const configuration_type&);

    // only this function can delete the interface
    friend void InterfaceManager::deleteInterface<iface_iio>(iface_iio*);
  };

} // namespace caribou

#endif
