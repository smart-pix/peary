/** Caribou datatypes to access regulators and sources
 */

#ifndef CARIBOU_DATATYPES_H
#define CARIBOU_DATATYPES_H

#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <strings.h>
#include <sys/mman.h>
#include <tuple>
#include <vector>

namespace caribou {

  /** Basic pixel class
   *
   *  Storage element for pixel configurations and pixel data.
   *  To be implemented by the individual device classes, deriving
   *  from this common base class.
   */
  class pixel {
  public:
    pixel() = default;
    /**
     * @brief Required virtual destructor
     */
    virtual ~pixel() = default;

    /// @{
    /**
     * @brief Use default copy behaviour
     */
    pixel(const pixel&) = default;
    pixel& operator=(const pixel&) = default;
    /// @}

    /// @{
    /**
     * @brief Use default move behaviour
     */
    pixel(pixel&&) = default;
    pixel& operator=(pixel&&) = default;
    /// @}

    /** Overloaded ostream operator for printing of pixel data
     */
    friend std::ostream& operator<<(std::ostream& out, const pixel& px) {
      px.print(out);
      return out;
    }

  protected:
    virtual void print(std::ostream&) const {};
  };

  /** Data returned by the peary device interface
   *
   *  Depending on the detector type operated, this can either be one frame
   *  read from the device, or one triggered event. The caribou::pixel pointer
   *  can be any type of data deriving from the pixel base class
   */
  using pearydata = std::map<std::pair<uint16_t, uint16_t>, std::unique_ptr<pixel>>;
  using pearydataVector = std::vector<pearydata>;

  struct pearyRawDataWord {

    uintptr_t word;

    // Do not initialise memory;
    pearyRawDataWord() = default;
    pearyRawDataWord(uintptr_t w) : word(w){};

    operator uintptr_t() const { return word; }
  };

  using pearyRawData = std::vector<pearyRawDataWord>;
  using pearyRawDataVector = std::vector<pearyRawData>;

  /** class to store a register configuration
   *
   *  @param address Address of the register in question
   *  @param mask    Mask identifying which bits belong to the register.
   *                 This allows to set registers which only occupy a
   *                 fraction of the full-size register to be written
   *  @param special Flag this register as "special", which forbids reading
   */
  template <typename REG_T = uint8_t, typename MASK_T = REG_T> class register_t {
  public:
    // If no address is given, also set the mask to zero:
    register_t() = default;

    // If no mask is given, default to accessing the full register:
    explicit register_t(REG_T address) : _address(address), _mask(std::numeric_limits<MASK_T>::max()){};
    register_t(REG_T address, bool readable, bool writable)
        : _address(address), _mask(std::numeric_limits<MASK_T>::max()), _readable(readable), _writable(writable){};

    explicit register_t(REG_T address, MASK_T mask) : _address(address), _mask(mask){};
    explicit register_t(REG_T address, MASK_T mask, bool readable, bool writable, bool special = false)
        : _address(address), _mask(mask), _special(special), _readable(readable), _writable(writable){};
    explicit register_t(
      REG_T address, MASK_T mask, MASK_T value, bool readable = true, bool writable = true, bool special = false)
        : _address(address), _mask(mask), _default(value), _has_default(true), _special(special), _readable(readable),
          _writable(writable){};

    REG_T address() const { return _address; };
    MASK_T mask() const { return _mask; };
    MASK_T value() const { return _default; };
    bool hasValue() const { return _has_default; };
    bool special() const { return _special; };
    bool writable() const { return _writable; }
    bool readable() const { return _readable; }

    MASK_T shift() const {
      if(_mask > 0) {
        return static_cast<MASK_T>(ffs(static_cast<int>(_mask)) - 1);
      } else {
        return 0;
      }
    };

    template <typename T1, typename T2> friend std::ostream& operator<<(std::ostream& os, const register_t<T1, T2>& rg);

  private:
    REG_T _address{};
    MASK_T _mask{};
    MASK_T _default{};
    bool _has_default{false};
    bool _special{false};
    bool _readable{true};
    bool _writable{true};
  };

  template <typename T1, typename T2> std::ostream& operator<<(std::ostream& os, const caribou::register_t<T1, T2>& rg) {
    os << to_hex_string(rg._address) << " (" << to_bit_string(rg._mask) << ")";
    if(rg._writable && rg._readable) {
      os << " RW";
    } else if(!rg._writable && rg._readable) {
      os << " RO";
    } else if(rg._writable && !rg._readable) {
      os << " WO";
    }
    return os;
  }

  /** Si5345 Rev B Configuration Register data struct
   *
   * struct holding a series of Silicon Labs Si5345 Rev B
   * register writes that can be performed to load a single configuration
   * on a device. To be used with file created by a Silicon Labs ClockBuilder Pro
   * export tool.
   */
  struct SI5345_REG_T {
    unsigned int address; /* 16-bit register address */
    unsigned char value;  /* 8-bit register data */
  };

  using si5345_revb_register_t = SI5345_REG_T;

  /** Component Configuration Base class
   */
  class component_t {
  public:
    explicit component_t(std::string name) : _name(std::move(name)){};
    virtual ~component_t() = default;
    std::string name() const { return _name; };

  private:
    std::string _name;
  };

  class component_io_t : public component_t {
  public:
    component_io_t(std::string name, uint8_t ioaddr) : component_t(std::move(name)), _ioaddr(ioaddr){};
    ~component_io_t() override = default;

    uint8_t ioaddr() const { return _ioaddr; };

  private:
    uint8_t _ioaddr;
  };

  /** Component Configuration class for components using DAC/ADC converters
   */
  class component_converter_t : public component_t {
  public:
    component_converter_t(std::string name, uint8_t addr, uint8_t channel)
        : component_t(std::move(name)), _address(addr), _channel(channel){};
    ~component_converter_t() override = default;

    uint8_t address() const { return _address; };
    uint8_t channel() const { return _channel; };

  private:
    uint8_t _address;
    uint8_t _channel;
  };

  /** Component Configuration class for components using a DAC output
   */
  class component_dac_t : public component_converter_t {
  public:
    component_dac_t(std::string name, uint8_t dacaddr, uint8_t dacout)
        : component_converter_t(std::move(name), dacaddr, dacout){};
    ~component_dac_t() override = default;
  };

  /** DC/DC converter Configuration
   *
   *  The parameters hold:
   *  - the name of the power output
   *  - the DAC address
   *  - the corresponding DAC output pin
   */
  class DCDC_CONVERTER_T : public component_dac_t {
  public:
    DCDC_CONVERTER_T(std::string name, uint8_t dacaddr, uint8_t dacout)
        : component_dac_t(std::move(name), dacaddr, dacout){};
    ~DCDC_CONVERTER_T() override = default;
  };

  /** Voltage Regulator Configuration
   *
   *  The parameters hold:
   *  - the name of the power output
   *  - the DAC address
   *  - the corresponding DAC output pin
   *  - the output pin of the power switch
   *  - the address of the current/power monitor
   */
  class VOLTAGE_REGULATOR_T : public component_dac_t {
  public:
    VOLTAGE_REGULATOR_T(std::string name, uint8_t dacaddr, uint8_t dacout, uint8_t pwrswitch, uint8_t pwrmon)
        : component_dac_t(std::move(name), dacaddr, dacout), _powerswitch(pwrswitch), _powermonitor(pwrmon){};
    ~VOLTAGE_REGULATOR_T() override = default;

    uint8_t pwrswitch() const { return _powerswitch; };
    uint8_t pwrmonitor() const { return _powermonitor; };

  private:
    uint8_t _powerswitch;
    uint8_t _powermonitor;
  };

  /** Current Source Configuration
   *
   *  The parameters hold:
   *  - the name of the current source
   *  - the DAC address
   *  - the corresponding DAC output pin
   *  - the output pin of the polarity switch
   */
  class CURRENT_SOURCE_T : public component_dac_t {
  public:
    CURRENT_SOURCE_T(std::string name, uint8_t dacaddr, uint8_t dacout, uint8_t polswitch)
        : component_dac_t(std::move(name), dacaddr, dacout), _polswitch(polswitch){};
    ~CURRENT_SOURCE_T() override = default;

    uint8_t polswitch() const { return _polswitch; };

  private:
    uint8_t _polswitch;
  };

  /** Current source polarity options
   */
  enum class CURRENT_SOURCE_POLARITY_T { PULL = 0, PUSH = 1 };
  inline std::istream& operator>>(std::istream& str, CURRENT_SOURCE_POLARITY_T& pol) {
    std::string polarity;
    if(str >> polarity) {
      if(polarity == "pull") {
        pol = CURRENT_SOURCE_POLARITY_T::PULL;
      } else if(polarity == "push") {
        pol = CURRENT_SOURCE_POLARITY_T::PUSH;
      }
    }
    return str;
  }

  using i2c_t = uint8_t;

  class GPIO_T : public component_io_t {
  public:
    GPIO_T(std::string name, uint8_t ioaddr, uint8_t bank, uint8_t offset)
        : component_io_t(std::move(name), ioaddr), _bank(bank), _offset(offset){};
    ~GPIO_T() override = default;

    uint8_t getBank() const { return _bank; };
    uint8_t getOffset() const { return _offset; };
    bool status(std::vector<i2c_t>& data) const { return bool((data.at(_bank) >> _offset) & 0b1); };

  private:
    uint8_t _bank;
    uint8_t _offset;
  };

  /** Component Configuration class for components using a ADC input
   */
  class component_adc_t : public component_converter_t {
  public:
    component_adc_t(std::string name, uint8_t adcaddr, uint8_t adcout)
        : component_converter_t(std::move(name), adcaddr, adcout){};
    ~component_adc_t() override = default;
  };

  /** Slow ADC Channel Configuration
   */
  class SLOW_ADC_CHANNEL_T : public component_adc_t {
  public:
    SLOW_ADC_CHANNEL_T(std::string name, uint8_t adcaddr, uint8_t adcout)
        : component_adc_t(std::move(name), adcaddr, adcout){};
    ~SLOW_ADC_CHANNEL_T() override = default;
  };

  /**  Bias Voltage Regulator Configuration
   *
   *  - the name of the bias voltage
   *  - the DAC address
   *  - the corresponding DAC output pin
   */
  class BIAS_REGULATOR_T : public component_dac_t {
  public:
    BIAS_REGULATOR_T(std::string name, uint8_t dacaddr, uint8_t dacout)
        : component_dac_t(std::move(name), dacaddr, dacout){};
    ~BIAS_REGULATOR_T() override = default;
  };

  /**  Injection Bias Voltage Regulator Configuration
   *
   *  - the name of the injection bias
   *  - the DAC address
   *  - the corresponding DAC output pin
   *  - FIXME: INJ_CTRL_X signals from FPGA!
   */
  class INJBIAS_REGULATOR_T : public component_dac_t {
  public:
    INJBIAS_REGULATOR_T(std::string name, uint8_t dacaddr, uint8_t dacout)
        : component_dac_t(std::move(name), dacaddr, dacout){};
    ~INJBIAS_REGULATOR_T() override = default;
  };

  // FIXME
  // MISSING
  // fast ADC: ADC_IN_XY

  class memory_map {
  public:
    memory_map(std::uintptr_t base_address, std::size_t size, int flags = PROT_READ)
        : _base_address(base_address), _size(size), _flags(flags){};
    std::uintptr_t getBaseAddress() const { return _base_address; }
    std::size_t getSize() const { return _size; }
    int getFlags() const { return _flags; }
    bool writable() const { return (_flags & PROT_WRITE) != 0; }

    // Compare memory pages, ingoring the offset
    bool operator<(const memory_map& other) const {
      if(_base_address == other.getBaseAddress()) {
        if(_size == other.getSize()) {
          return _flags < other.getFlags();
        }
        return _size < other.getSize();
      }
      return _base_address < other.getBaseAddress();
    }

  private:
    std::uintptr_t _base_address;
    std::size_t _size;
    int _flags;
  };
} // namespace caribou

#endif /* CARIBOU_DATATYPES_H */
