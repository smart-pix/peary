// Caribou media bus emulator implementation

#include "media.hpp"

using namespace caribou;

iface_media::iface_media(const configuration_type& config)
    : Interface(config), _outputEntity(config._outputEntity), _links(config._links), _routes(config._routes),
      _formats(config._formats), _timeout(config._timeout), buffersPool(config._noBuffers), videoDesc(-1) {

  LOG(DEBUG) << "Opening media-ctl device";
}

iface_media::~iface_media() {
  LOG(TRACE) << "Destroying media-ctl device";
  if(isStreaming()) {
    readStop();
  }
}

const char* iface_media::findPathOfSubDevice(std::string const& subdevName) {
  LOG(DEBUG) << "Searching for subdevice " << subdevName;
  return "";
}

void iface_media::parse_setup_outputEntity(std::string const& outputEntity) {
  LOG(TRACE) << "Parsing and setting up output entity: " << outputEntity;
  videoDesc = 0;
}

void iface_media::parse_setup_routes(std::string const& routes) {
  LOG(TRACE) << "Parsing and setting up routes: " << routes;
}

void iface_media::configureMediaPipeline() {
  LOG(TRACE) << "Configureing media pipeline";
}

void iface_media::configureFullPipeline() {
  LOG(TRACE) << "Configureing full pipeline";
}

void iface_media::readStart() {
  LOG(DEBUG) << "Starting streaming";
}

void iface_media::readStop() {
  LOG(DEBUG) << "Stopping streaming";
  videoDesc = -1;
}

bool iface_media::isStreaming() const {
  return videoDesc != -1;
}

void iface_media::setSubDevice(std::string const& name, const std::pair<std::uintptr_t, std::uintptr_t> reg) {

  LOG(TRACE) << "Subdevice " << name << " address " << to_hex_string(reg.first) << ": writing data \""
             << to_hex_string(reg.second) << "\"";
}

uintptr_t iface_media::getSubDevice(std::string const& name, const std::uintptr_t offset) {
  LOG(TRACE) << "Subdevice " << name << " address " << to_hex_string(offset) << ": read data \"" << to_hex_string(0) << "\"";

  return 0;
}

media_t iface_media::read() {

  LOG(DEBUG) << "Reading the video device";

  if(!isStreaming()) {
    throw DeviceException("The interface is not streaming.");
  }

  media_t frame;

  return frame;
}

iface_media::dataVector_type iface_media::read(const unsigned int numberOfFrames) {

  dataVector_type result(numberOfFrames);

  for(std::size_t i = 0; i < numberOfFrames; i++) {
    result.push_back(read());
  }

  return result;
}
