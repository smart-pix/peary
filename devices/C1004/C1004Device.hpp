/**
 * Caribou Device implementation for C1004
 */

#ifndef DEVICE_C1004_H
#define DEVICE_C1004_H

#include <string>
#include <vector>
#include "device/CaribouDevice.hpp"
#include "hardware_abstraction/falconboard/Falconboard.hpp"
#include "interfaces/Media/media.hpp"
#include "interfaces/SPI_BUS/spi_bus.hpp"

#include "C1004Defaults.hpp"

namespace caribou {

  /** C1004 Device class definition
   *
   *  this class implements the required functionality to operate C1004 chips via the
   *  Caribou device class interface.
   */
  class C1004Device : public CaribouDevice<falconboard::Falconboard, iface_spi_bus, iface_media> {

  public:
    C1004Device(const caribou::Configuration config);
    ~C1004Device();

    /** Initializer function for C1004
     */
    void configure() override;

    /** Turn on the power supply for the C1004 chip
     */
    void powerUp() override;

    /** Turn off the C1004 power
     */
    void powerDown() override;

    /** Start the data acquisition
     */
    void daqStart() override;

    /** Stop the data acquisition
     */
    void daqStop() override;

    /**
     */
    pearyRawData getRawData() override;

    /**
     */
    pearyRawDataVector getRawData(const unsigned int noFrames) override;

    /**
     */
    pearydata getData() override;

    /**
     */
    pearydataVector getData(const unsigned int noFrames) override;

    void setSpecialRegister(const std::string& name, uintptr_t value) override;
    uintptr_t getSpecialRegister(const std::string& name) override;

    // Reset the chip
    // The reset signal is asserted for ~1us
    void reset() override;

    // This method configures the sequencer
    void configureSequencer(const bool directControl,
                            const uint32_t duration,
                            const uint32_t repetition_rate,
                            const uint32_t latch_duration);

    // This method returns string with the sequencer status
    void getSequencerStatus();

    // This method sets voltage of the laser DC/DC converter
    void setLaserHV(const double voltage);

    // The method returns string with the laser status
    void getLaserHVStatus();

    // Sets _videoStreamConsumer
    void setVideoStreamConsumer(bool videoStreamConsumer);

    // Gets _videoStreamConsumer
    bool getVideoStreamConsumer();

    // Type defining data procssor used to process data incoming from C1004
    enum dataProcessor_t { distance = 0, user = 1, intensity = 2, rawTimestamps = 3 };

    // Select data procssor for the C1004 readout
    void setDataProcessor(const dataProcessor_t dataProcessor);

    // get the current data procssor of the C1004 readout
    dataProcessor_t getDataProcessor();

    // Set data processor register
    void setDataProcessorRegister(const std::string& name, uintptr_t value);

    // Get data processor register
    uintptr_t getDataProcessorRegister(const std::string& name);

    // Type defining clock source
    enum clockSource_t { internal = 0, external = 1 };

    // Select clock source
    void setClockSource(const clockSource_t source);

    // Get clock source
    clockSource_t getClockSource();

  protected:
    // The below objects gives the class write access to critical registers
    // Which are not exposed (with write permission) through the device memory dictionary

    // Write access to the sequencer register
    memory_map reg_sequencer{C1004_CONTROL_BASE_ADDRESS, C1004_CONTROL_MAP_SIZE, PROT_WRITE};

    // if set, the class enables access to the video stream
    bool _videoStreamConsumer;

    // Data procssor used to process data incoming from C1004
    dataProcessor_t _dataProcessor;

    // Data readout interface configuration currently being used
    // _driConfig from the derived from CaribouDevice is used as default for singleBalance data processor
    iface_media::configuration_type _activeDriConfig;

    // Registers associated with the given data processor
    typedef caribou::dictionary<register_t<std::uintptr_t, std::uintptr_t>> dataProcessorRegister_t;
    std::map<dataProcessor_t, dataProcessorRegister_t> _dataProcessorRegisters;

    // Returns the readout interface configuration specific for the given data processor
    iface_media::configuration_type getDriConfiguration(const dataProcessor_t dataProcessor);

    // Decode a set of frames
    pearydataVector decodeFrame(const typename iface_media::dataVector_type& frames);

    // This method enables/disables the sequecner
    void enableSequencer(const bool enable);

    // Check if the requested laser HV module voltage is safe in the given configuration.
    // If not, the function throws ConfigInvalid exception.
    void checkLaserHV();
  };

  std::istream& operator>>(std::istream&, C1004Device::dataProcessor_t&);
  std::ostream& operator<<(std::ostream&, C1004Device::dataProcessor_t const&);
  std::istream& operator>>(std::istream&, C1004Device::clockSource_t&);
  std::ostream& operator<<(std::ostream&, C1004Device::clockSource_t const&);

} // namespace caribou

#endif /* DEVICE_C1004_H */
