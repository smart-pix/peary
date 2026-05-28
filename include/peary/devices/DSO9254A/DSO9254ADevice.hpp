/**
 * Caribou Dso9254a Device implementation
 *
 *  Use this class as a sarting point to implement your own caribou device
 */

#ifndef DEVICE_DSO9254A_H
#define DEVICE_DSO9254A_H

#include <thread>
#include <vector>

#include "device/AuxiliaryDevice.hpp"
#include "interfaces/IPSocket/ipsocket.hpp"
#include "utils/constants.hpp"

#define DEFAULT_DEVICEPATH "192.168.5.7:5025"

namespace caribou {

  /** Dso9254a Device class definition
   *
   *  This class implements all purely virtual functions of caribou::Device.
   *  Applications can then control this device via the Caribou device class interface
   *  by using the device manager to instanciate the device object.
   */
  class DSO9254ADevice : public AuxiliaryDevice<iface_ipsocket> {

  public:
    /** Device constructor
     */
    DSO9254ADevice(const caribou::Configuration config);
    ~DSO9254ADevice();

    void configure() override;

    void send(std::string command);
    std::string query(const std::string query);

    pearydata getData() override;
    pearydataVector getData(const unsigned int) override;

    pearyRawData getRawData() override;
    int waitForTrigger();

    /** Start the data acquisition
     */
    void daqStart() override;

    /** Stop the data acquisition
     */
    void daqStop() override;

    void getBinaryData(std::string out);

    void printErrors();

  private:
    std::atomic_flag _daqContinue{};
    std::thread _daqThread;

    bool daqRunning = false;

    void runDaq();

    pearyRawData fetch_channel_data(int channel);

    std::vector<int> channels_;

    std::chrono::time_point<std::chrono::system_clock> scope_acq_start{};
    bool scope_acq_running = false;
    size_t triggers_seen_{};
  };

} // namespace caribou

#endif /* DEVICE_DSO9254A_H */
