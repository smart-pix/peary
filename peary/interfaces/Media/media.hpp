#ifndef CARIBOU_HAL_MEDIA_HPP
#define CARIBOU_HAL_MEDIA_HPP

#include <mutex>
#include <utility>
#include <vector>
#include "interfaces/Interface.hpp"
#include "interfaces/InterfaceManager.hpp"
#include "utils/datatypes.hpp"
#include "utils/exceptions.hpp"

#ifndef MEDIA_EMULATED
extern "C" {
#include "mediactl/mediactl.h"
#include "mediactl/v4l2subdev.h"
}
#endif

// The media interface abstracts access to a Xilinx Video Composite Device, allowing configuration of the
// video pipeline and readout of the video frames.
// The configuration strings are equivalent in functionality and formats to a options passed to media-ctl commands:
// outputEntity : enity on the pipiline from which to read data with the required video format, following medita-ctl
// formatting, eg. "falcon_video output 1" [fmt:Y16/32x60] links : media-ctl -l (comma-separated list of link descriptors to
// setup) formats : media-ctl -V (comma-separated list of formats to setup) routes : comma-separated list of routes to setup
// (like Xilinx AXI-S switches), formatting: "a0010000.axis_switch" 0 -> 2 timeout: timeout for each readout in seconds,
// default 1s

namespace caribou {

  using media_reg_t = uint8_t;
  using media_t = pearyRawData;

  class iface_media_config : public InterfaceConfiguration {
  public:
    iface_media_config(const std::string& devpath,
                       std::string outputEntity,
                       std::string links,
                       std::string routes,
                       std::string formats,
                       const unsigned int timeout = 1,
                       const unsigned int noBuffers = 10);

    std::string _outputEntity;
    std::string _links;
    std::string _routes;
    std::string _formats;
    int unsigned _timeout;
    int unsigned _noBuffers;

    virtual bool operator<(const iface_media_config& rhs) const;
  };

  // The interface privdes access to video piplines through media-ctl API
  class iface_media : public Interface<media_reg_t, media_t, iface_media_config> {

  protected:
    // Configuration
    std::string const _outputEntity;
    std::string const _links;
    std::string const _routes;
    std::string const _formats;
    // Timeout for readout
    const int unsigned _timeout;

    // Protects access to the video deivce
    std::mutex mutex;

    // Size of the video frame in bytes
    unsigned int frameSize;

    // Pool of the memory buffers
    std::vector<std::pair<void*, std::size_t>> buffersPool;

    // Default constructor: private (only created by InterfaceManager)
    // It can throw DeviceException
    explicit iface_media(const configuration_type& config);

    ~iface_media() override;

    // Finds path of the subDevice
    const char* findPathOfSubDevice(std::string const& subdevName);

    // The method parse and initalizes outputEntity (videoDesc)
    void parse_setup_outputEntity(std::string const& outputEntity);

    // The method parsing and setting V4L2 routing
    void parse_setup_routes(std::string const& routes);

    // Configure the full video pipeline:
    // - it calls configureMediaPipeline
    // - negotiate the proper format with the video device
    void configureFullPipeline();

    // File descriptor to the video device
    int videoDesc;

#ifndef MEDIA_EMULATED
    struct media_device* media;
#endif

    GENERATE_FRIENDS()

  public:
    std::string outputEntity() const {
      return _outputEntity;
    }
    std::string links() const {
      return _links;
    }
    std::string routes() const {
      return _routes;
    }
    std::string formats() const {
      return _formats;
    }

    // Configure the media pipeline
    void configureMediaPipeline();

    // Should be called before readout to block the video device
    void readStart();
    // Should be called after readout to realease the video device
    void readStop();
    // Check if the video deive is blocked
    bool isStreaming() const;

    // Write registers of the Xilinx video device
    void setSubDevice(std::string const& name, const std::pair<size_t, uintptr_t> reg);

    // Read registers of the Xilinx video device
    uintptr_t getSubDevice(std::string const& name, const size_t offset);

    // Function to read a video frame
    media_t read() override;

    // Read number of video frames
    dataVector_type read(const unsigned int numberOfFrames) override;

    // Unused constructor
    iface_media() = delete;

    // only this function can create the interface
    friend iface_media& InterfaceManager::getInterface<iface_media>(const configuration_type&);

    // only this function can delete the interface
    friend void InterfaceManager::deleteInterface<iface_media>(iface_media*);
  };

} // namespace caribou

#endif
