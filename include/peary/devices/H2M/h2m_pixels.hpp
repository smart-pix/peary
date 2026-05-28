// Defines H2M pixels types
#pragma once

#include <ostream>
#include "utils/datatypes.hpp"
#include "utils/exceptions.hpp"

namespace caribou {

  // Basic pixel class
  // The information is internally stored in the same way, the chip stores it, as
  // a 8bit register.
  //
  // The individual values are set via the member functions of a specialized
  // classes

  class h2m_pixel : public pixel {
  public:
    h2m_pixel() = default;
    h2m_pixel(uint8_t data) : m_data(data){};
    virtual ~h2m_pixel(){};

    /* get functions for private data members
     */
    uint8_t GetData() { return m_data; }

  protected:
    /* hit data
     */
    uint8_t m_data{};
  };

  /* H2M pixel configuration class
   *
   * Class to hold all information required to fully configure one pixel.
   * The information is internally stored in the same way, the chip stores it, as
   * a 8bit register. The individual values are set via the member functions
   * and can be retrieved bitwise for convenience.
   */
  class h2m_pixel_conf : public h2m_pixel {
  public:
    /* Default constructor
     *
     * Initializes the pixel. Default:
     * Testpulse: OFF
     * Maksed: NOT
     * Tuning daq: MAX VAULE
     * Not top pixel: FALSE
     */
    h2m_pixel_conf() : h2m_pixel(0b00111100){};
    h2m_pixel_conf(uint8_t data) : h2m_pixel(data){};
    h2m_pixel_conf(bool tp_enable, bool mask, uint8_t tuning_dac, bool not_top_pixel) {
      SetTP(tp_enable);
      SetMask(mask);
      SetTuningDAC(tuning_dac);
      SetNotTopPixel(not_top_pixel);
    };
    virtual ~h2m_pixel_conf(){};
    /* Test pulse setting of the pixel
     */
    void SetTP(bool tp_enable) {
      if(tp_enable) {
        m_data |= 0x01;
      } else {
        m_data &= static_cast<uint8_t>(~0x01);
      }
    }
    bool GetTP() const { return m_data & 0x1; }

    /* Mask setting of the pixel
     */
    void SetMask(bool mask) {
      if(mask) {
        m_data |= 0x02;
      } else {
        m_data &= static_cast<uint8_t>(~0x02);
      }
    }
    bool GetMask() const { return (m_data >> 1) & 0x1; }

    /* Trim DAC control setting of the pixel
     */
    void SetTuningDAC(uint8_t tuning_dac) {

      tuning_dac &= 0x0F;
      // map to chip internal convetion
      tuning_dac = dac_mapping.at(tuning_dac);

      m_data &= 0xC3;
      m_data |= static_cast<uint8_t>(tuning_dac << 2);
    }
    uint8_t GetTuningDAC() const {
      uint8_t stripped = (m_data >> 2) & 0xF;
      return static_cast<uint8_t>(find(dac_mapping.begin(), dac_mapping.end(), stripped) - dac_mapping.begin());
    }

    /* "Not top pixel" setting of the pixel
     */
    void SetNotTopPixel(bool not_top_pixel) {
      if(not_top_pixel) {
        m_data |= 0x40;
      } else {
        m_data &= static_cast<uint8_t>(~0x40);
      }
    }
    bool GetNotTopPixel() const { return (m_data >> 6) & 0x1; }

    /** Overloaded print function for ostream operator
     */
    virtual void print(std::ostream& out) const {
      out << "px [" << this->GetTP() << "|" << this->GetMask() << "|" << this->GetTuningDAC() << "|"
          << this->GetNotTopPixel() << "]";
    }

  private:
    std::vector<uint8_t> dac_mapping{1, 3, 5, 7, 9, 11, 13, 14, 12, 10, 8, 6, 4, 2, 0, 0};
  };

  // H2M pixel readout class
  // The individual values are set via the member functions
  class h2m_pixel_readout : public h2m_pixel {
  public:
    // Default constructor
    h2m_pixel_readout() : h2m_pixel(0x0), m_mode(ACQ_MODE_TOT){};
    h2m_pixel_readout(uint8_t data, uint8_t mode) : h2m_pixel(data), m_mode(mode){};
    virtual ~h2m_pixel_readout(){};

    void SetData(uint8_t data) { m_data = data; }
    void SetMode(uint8_t mode) { m_mode = mode; }
    uint8_t GetMode() { return m_mode; }

    uint8_t GetToA() {
      if(m_mode == ACQ_MODE_TOA) {
        return m_data;
      } else {
        throw WrongDataFormat("Wrong acquisition mode set, no TOA available.");
      }
    }

    uint8_t GetToT() { return (m_mode == ACQ_MODE_TOT ? m_data : 1); }

    uint8_t GetCNT() { return (m_mode == ACQ_MODE_CNT ? m_data : 1); }

    uint8_t GetTRG() {
      if(m_mode == ACQ_MODE_TRG) {
        return m_data;
      } else {
        throw WrongDataFormat("Wrong acquisition mode set, no TRG available.");
      }
    }

    void print(std::ostream& out) const override {
      // layout: mode, data (compatible with pearycli output)
      out << static_cast<int>(m_mode) << "," << static_cast<int>(m_data);
    }

  private:
    /* readout mode flag
     */
    uint8_t m_mode;
  };

} // namespace caribou
