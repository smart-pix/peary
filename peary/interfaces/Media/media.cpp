/**
 * Caribou Media interface class implementation
 */

#include "media.hpp"
extern "C" {
#include <linux/v4l2-subdev.h>
#include <linux/xilinx-hls.h>
}
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>

#include <regex>

using namespace caribou;

/*
 * The max value comes from a check in the kernel source code
 * drivers/media/v4l2-core/v4l2-ioctl.c check_array_args()
 */
#define NUM_ROUTES_MAX 256

iface_media::iface_media(const configuration_type& config)
    : Interface(config), _outputEntity(config._outputEntity), _links(config._links), _routes(config._routes),
      _formats(config._formats), _timeout(config._timeout), buffersPool(config._noBuffers), videoDesc(-1) {
  LOG(TRACE) << "Opening media-ctl device";
  int ret = 0;

  media = media_device_new(devicePath().c_str());

  if(media == nullptr) {
    throw DeviceException("Failed to open media-ctl device");
  }

  LOG(DEBUG) << "Enumerating media-ctl device";
  ret = media_device_enumerate(media);
  if(ret) {
    throw DeviceException("Failed to enumerate media-ctl device (" + std::to_string(-ret) + "): " + std::strerror(-ret));
  }
}

iface_media::~iface_media() {
  LOG(TRACE) << "Destroying media-ctl device";
  if(isStreaming()) {
    readStop();
  }

  media_device_unref(media);
}

const char* iface_media::findPathOfSubDevice(std::string const& subdevName) {
  LOG(DEBUG) << "Searching for subdevice " << subdevName;
  auto mediaEntity = media_get_entity_by_name(media, subdevName.c_str());
  if(mediaEntity == nullptr) {
    throw ConfigInvalid("Failed to find subdeivce " + subdevName);
  }

  auto subdevPath = media_entity_get_devname(mediaEntity);
  if(subdevPath == nullptr) {
    throw ConfigInvalid("The selected routing subdevice (" + subdevName + ") doesn't have an associated device node");
  }

  return subdevPath;
}

void iface_media::parse_setup_outputEntity(std::string const& outputEntity) {
  std::smatch m;
  if(!std::regex_match(outputEntity, m, std::regex("^\\s*\"([^\"]*)\"\\s*\\[fmt:([A-Z0-9]{1,4})/(\\d*)x(\\d*)\\]\\s*$"))) {
    throw ConfigInvalid("Failed to parse video output entity configuration.");
  }

  //////////////////
  // Open device
  //////////////////
  auto videoEntity = media_get_entity_by_name(media, m[1].str().c_str());
  if(videoEntity == nullptr) {
    throw ConfigInvalid("Failed to find the output video entity");
  }
  auto videoPath = media_entity_get_devname(videoEntity);
  if(videoPath == nullptr) {
    throw ConfigInvalid("The video device doesn't have an associated device node");
  }

  LOG(TRACE) << "Opening the video device" << videoPath;
  videoDesc = open(videoPath, O_RDWR /* required */ | O_NONBLOCK, 0);
  if(videoDesc == -1) {
    throw DeviceException("Failed to open the video device " + std::string(videoPath) + " :" + std::strerror(errno));
  }

  ////////////////////
  // Init device
  ////////////////////
  struct v4l2_capability cap;
  if(ioctl(videoDesc, VIDIOC_QUERYCAP, &cap) == -1) {
    throw DeviceException("Failed to query capabilites of the video device " + std::string(videoPath) + " : " +
                          std::strerror(errno));
  }

  if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    throw DeviceException("The video device " + std::string(videoPath) + " is not a capture device");
  }

  if(!(cap.capabilities & V4L2_CAP_STREAMING)) {
    throw DeviceException("The video device " + std::string(videoPath) + " is does not support streaming I/O");
  }

  // In case device supports croping set it to default
  struct v4l2_cropcap cropcap = {};
  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if(ioctl(videoDesc, VIDIOC_CROPCAP, &cropcap) == 0) {
    struct v4l2_crop crop = {};

    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect; /* reset to default */

    ioctl(videoDesc, VIDIOC_S_CROP, &crop);
  }

  // Negotiate video format
  struct v4l2_format fmt = {};
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if(ioctl(videoDesc, VIDIOC_G_FMT, &fmt) == -1) {
    throw DeviceException("Failed to querry format for the video device " + std::string(videoPath) + " : " +
                          std::strerror(errno));
  }
  std::string format = m[2];
  format.resize(4, ' ');
#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN || defined(__BIG_ENDIAN__)
  fmt.fmt.pix.pixelformat = v4l2_fourcc_be(format[0], format[1], format[2], format[3]);
#else
  fmt.fmt.pix.pixelformat = v4l2_fourcc(format[0], format[1], format[2], format[3]);
#endif
  fmt.fmt.pix.width = static_cast<unsigned int>(std::stoi(m[3]));
  fmt.fmt.pix.height = static_cast<unsigned int>(std::stoi(m[4]));
  fmt.fmt.pix.bytesperline = 0; // G_FMT might return a bytesperline value > width,
                                // reset this to 0 to force the driver to update it
                                // to the closest value for the new width.

  if(ioctl(videoDesc, VIDIOC_S_FMT, &fmt) == -1) {
    throw DeviceException("Failed to set format for the video device " + std::string(videoPath) + " : " +
                          std::strerror(errno));
  }

  frameSize = fmt.fmt.pix.sizeimage;
  LOG(DEBUG) << "Frame size for the video entity " << std::string(videoPath) << " set to " << frameSize << " B";

  // Allocate buffers
  struct v4l2_requestbuffers req = {};

  req.count = static_cast<__u32>(buffersPool.size());
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  LOG(DEBUG) << "Number of the requested mmap buffers: " << req.count;

  if(ioctl(videoDesc, VIDIOC_REQBUFS, &req) == -1) {
    throw DeviceException("Failed to set memory mapped I/O access for the video device " + std::string(videoPath) + " : " +
                          std::strerror(errno));
  }

  if(req.count != buffersPool.size()) {
    throw DeviceException("Reqested " + std::to_string(buffersPool.size()) + " and got " + std::to_string(req.count) +
                          " video buffers.");
  }

  // mmap bufers
  for(std::size_t i = 0; i < buffersPool.size(); i++) {
    struct v4l2_buffer buf = {};

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = static_cast<__u32>(i);

    if(ioctl(videoDesc, VIDIOC_QUERYBUF, &buf) == -1) {
      throw DeviceException("Failed to query the video buffer (VIDIOC_QUERYBUF).");
    }

    buffersPool[i] =
      std::make_pair(mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, videoDesc, buf.m.offset), buf.length);

    if(buffersPool[i].first == MAP_FAILED) {
      throw DeviceException("mmap failed");
    }
  }
}

void iface_media::parse_setup_routes(std::string const& routes) {

  struct v4l2_subdev_routing routing = {};
  struct v4l2_subdev_route routingTab[NUM_ROUTES_MAX] = {};

  routing.routes = routingTab;

  if(!std::regex_match(
       routes,
       std::regex("^(?:\\s*\"[^\"]*\"(?:\\s*\\d+\\s*->\\s*\\d+\\s*)+,)*\\s*\"[^\"]*\"(?:\\s*\\d+\\s*->\\s*\\d+\\s*)+$"))) {
    throw ConfigInvalid("Failed to parse routing configuration.");
  }

  std::regex subdevEntry("\"(.*?)\"(.*?)(?:,|$)");
  std::regex subdevRoute("\\s*(\\d+)\\s*->\\s*(\\d+)\\s*");

  for(std::regex_iterator s = std::sregex_iterator(routes.begin(), routes.end(), subdevEntry); s != std::sregex_iterator();
      ++s) {
    std::smatch m = *s;
    routing.num_routes = 0;
    std::string const& subdevName = m[1];
    std::string const& subdevRoutes = m[2];

    for(std::regex_iterator r = std::sregex_iterator(subdevRoutes.begin(), subdevRoutes.end(), subdevRoute);
        r != std::sregex_iterator();
        ++r) {
      routingTab[routing.num_routes].sink = static_cast<unsigned int>(std::stoi((*r)[1]));
      routingTab[routing.num_routes].source = static_cast<unsigned int>(std::stoi((*r)[2]));
      routing.num_routes++;
    }

    auto subdevPath = findPathOfSubDevice(subdevName);

    LOG(TRACE) << "Opening subdevice " << subdevPath;
    auto subdev = open(subdevPath, O_RDWR);
    if(subdev < 0) {
      throw DeviceException("Open " + std::string(subdevPath) + " routing device failed ( " + std::to_string(subdev) +
                            "): " + std::strerror(subdev));
    }

    LOG(DEBUG) << "Setting routing on subdevice " << subdevPath;
    if(ioctl(subdev, VIDIOC_SUBDEV_S_ROUTING, &routing) < 0) {
      throw CommunicationError("Failed to set routing (" + subdevRoutes + ") for " + subdevName +
                               " device: " + std::strerror(errno));
    }
    close(subdev);
  }
}

void iface_media::configureMediaPipeline() {
  int ret = 0;

  LOG(DEBUG) << "Resetting all links";
  ret = media_reset_links(media);
  if(ret) {
    throw DeviceException("Failed to reset links of media-ctl device (" + std::to_string(-ret) +
                          "): " + std::strerror(-ret));
  }

  LOG(DEBUG) << "Setting up the requested links:\n" << links();
  ret = media_parse_setup_links(media, links().c_str());
  if(ret) {
    throw DeviceException("Failed to set up the reqesuted links (" + std::to_string(-ret) + "): " + std::strerror(-ret));
  }

  LOG(DEBUG) << "Setting up the requested routing:\n" << routes();
  parse_setup_routes(routes());

  LOG(DEBUG) << "Setting up the requested formats:\n" << formats();
  ret = v4l2_subdev_parse_setup_formats(media, formats().c_str());
  if(ret) {
    throw DeviceException("Failed to set up the reqesuted formats (" + std::to_string(-ret) + "): " + std::strerror(-ret));
  }
}

void iface_media::configureFullPipeline() {
  configureMediaPipeline();
  LOG(DEBUG) << "Setting up the output video entity: " << outputEntity();
  parse_setup_outputEntity(outputEntity());
}

void iface_media::readStart() {
  // lock access
  mutex.lock();

  LOG(DEBUG) << "Starting streaming";

  // Configure the pipeline
  configureFullPipeline();

  // Queue the buffers
  for(std::size_t i = 0; i < buffersPool.size(); i++) {
    struct v4l2_buffer buf = {};

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = static_cast<__u32>(i);

    if(ioctl(videoDesc, VIDIOC_QBUF, &buf) == -1) {
      throw DeviceException("Failed to queue the buffer (VIDIOC_QBUF).");
    }
  }

  // Start streaming
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if(ioctl(videoDesc, VIDIOC_STREAMON, &type) == -1) {
    throw DeviceException("Failed to start streaming from the video device: " + std::string(std::strerror(errno)));
  }
}

void iface_media::readStop() {

  LOG(DEBUG) << "Stopping streaming";

  // Stop streaming
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if(ioctl(videoDesc, VIDIOC_STREAMOFF, &type) == -1) {
    throw DeviceException("Failed to stop streaming from the video device: " + std::string(std::strerror(errno)));
  }

  // Unmap buffers
  for(auto& buf : buffersPool) {
    if(munmap(buf.first, buf.second) == -1) {
      throw DeviceException("Failed to unmap the video buffer.");
    }
  }

  if(close(videoDesc) == -1) {
    throw DeviceException("Failed to close the video device:" + std::string(std::strerror(errno)));
  }
  videoDesc = -1;

  // Unlock access;
  mutex.unlock();
}

bool iface_media::isStreaming() const {
  return videoDesc != -1;
}

void iface_media::setSubDevice(std::string const& name, const std::pair<std::uintptr_t, std::uintptr_t> reg) {

  auto subdevPath = findPathOfSubDevice(name);

  LOG(TRACE) << "Opening subdevice " << subdevPath;
  auto subdev = open(subdevPath, O_RDWR);
  if(subdev < 0) {
    throw DeviceException("Open " + std::string(subdevPath) + " subdevice failed ( " + std::to_string(subdev) +
                          "): " + std::strerror(subdev));
  }

  LOG(TRACE) << "Subdevice " << name << " address " << to_hex_string(reg.first) << ": writing data \""
             << to_hex_string(reg.second) << "\"";

  xilinx_axi_hls_register xreg;
  xreg.offset = static_cast<__u32>(reg.first);
  xreg.value = static_cast<__u32>(reg.second);
  xilinx_axi_hls_registers xregs;
  xregs.num_regs = 1;
  xregs.regs = &xreg;

  if(-1 == ioctl(subdev, XILINX_AXI_HLS_WRITE, &xregs))
    throw CommunicationError("Failed to set register " + to_hex_string(reg.first) + " of subdevice " + name +
                             " with value " + to_hex_string(reg.second));

  close(subdev);
}

uintptr_t iface_media::getSubDevice(std::string const& name, const std::uintptr_t offset) {
  auto subdevPath = findPathOfSubDevice(name);

  LOG(TRACE) << "Opening subdevice " << subdevPath;
  auto subdev = open(subdevPath, O_RDWR);
  if(subdev < 0) {
    throw DeviceException("Open " + std::string(subdevPath) + " subdevice failed ( " + std::to_string(subdev) +
                          "): " + std::strerror(subdev));
  }

  xilinx_axi_hls_register xreg;
  xreg.offset = static_cast<__u32>(offset);
  xilinx_axi_hls_registers xregs;
  xregs.num_regs = 1;
  xregs.regs = &xreg;

  if(-1 == ioctl(subdev, XILINX_AXI_HLS_READ, &xregs))
    throw CommunicationError("Failed to read register " + to_hex_string(offset) + " of subdevice " + name);

  LOG(TRACE) << "Subdevice " << name << " address " << to_hex_string(offset) << ": read data \"" << to_hex_string(xreg.value)
             << "\"";

  close(subdev);

  return xreg.value;
}

media_t iface_media::read() {

  LOG(TRACE) << "Reading the video device";

  if(!isStreaming()) {
    throw DeviceException("The interface is not streaming.");
  }

  // Set timeout descriptors
  fd_set videoDesc_set;
  FD_ZERO(&videoDesc_set);
  FD_SET(videoDesc, &videoDesc_set);

  struct timeval timeout = {_timeout, 0};

  int ret = select(videoDesc + 1, &videoDesc_set, nullptr, nullptr, &timeout);

  if(ret == -1) {
    throw DeviceException("Failed to read data (select call) from the the video device: " +
                          std::string(std::strerror(errno)));
  }

  if(ret == 0) {
    throw DeviceException("Timeout reached reading data from the video device");
  }

  struct v4l2_buffer buf = {};

  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  if(ioctl(videoDesc, VIDIOC_DQBUF, &buf) == -1) {
    throw DeviceException("Failed to read data (VIDIOC_DQBUF ioctl) from the the video device: " +
                          std::string(std::strerror(errno)));
  }

  // copy buffer to vector
  media_t frame(static_cast<media_t::value_type*>(buffersPool[buf.index].first),
                static_cast<media_t::value_type*>(buffersPool[buf.index].first) +
                  buf.bytesused / sizeof(media_t::value_type));

  if(ioctl(videoDesc, VIDIOC_QBUF, &buf) == -1) {
    throw DeviceException("Failed to queue the buffer (VIDIOC_QBUF).");
  }

  return frame;
}

iface_media::dataVector_type iface_media::read(const unsigned int numberOfFrames) {

  dataVector_type result;

  for(std::size_t i = 0; i < numberOfFrames; i++) {
    result.push_back(read());
  }

  return result;
}
