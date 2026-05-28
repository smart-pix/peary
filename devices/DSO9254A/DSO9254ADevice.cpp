/**
 * Caribou Dso9254a Device implementation
 */

#include "DSO9254ADevice.hpp"
#include "utils/log.hpp"

#include <chrono>
#include <fstream>
#include <sstream>
#include <string>

using namespace caribou;

DSO9254ADevice::DSO9254ADevice(const caribou::Configuration config)
    : AuxiliaryDevice(config, InterfaceConfiguration(config.Get("devicepath", std::string(DEFAULT_DEVICEPATH)))) {

  _dispatcher.add("configure", &DSO9254ADevice::configure, this);
  _dispatcher.add("query", &DSO9254ADevice::query, this);
  _dispatcher.add("send", &DSO9254ADevice::send, this);
  _dispatcher.add("waitForTrigger", &DSO9254ADevice::waitForTrigger, this);
  _dispatcher.add("getBinaryData", &DSO9254ADevice::getBinaryData, this);
  _dispatcher.add("printErrors", &DSO9254ADevice::printErrors, this);

  LOG(DEBUG) << "New scope interface, trying to connect...";
  std::string query = "*IDN?";
  std::string data = AuxiliaryDevice<iface_ipsocket>::receive(query).front();

  LOG(DEBUG) << "Connected successfully to "
             << InterfaceManager::getInterface<iface_ipsocket>(_interfaceConfig).devicePath();
  LOG(DEBUG) << "Scope identifier: " << data;
}

DSO9254ADevice::~DSO9254ADevice() {
  LOG(INFO) << "Shutdown, delete device.";
}

void DSO9254ADevice::send(std::string command) {
  LOG(DEBUG) << "Command: " << command;
  AuxiliaryDevice<iface_ipsocket>::send(command);
}

std::string DSO9254ADevice::query(const std::string query) {
  std::string s = AuxiliaryDevice<iface_ipsocket>::receive(query).front();
  LOG(DEBUG) << s;
  return s;
}

void DSO9254ADevice::configure() {
  std::string path = _config.Get("config", "");

  LOG(INFO) << "Configuring using: " << path;

  if(path.empty()) {
    LOG(ERROR) << "File not existing";
    return;
  }

  std::ifstream file(path);
  std::string line;

  while(std::getline(file, line)) {
    if(!line.empty() && line[0] != '#') {
      send(line);
    }
  }

  // Enable channels
  channels_.clear();
  for(int i = 1; i <= 4; i++) {
    std::string cmd = ":CHANNEL" + std::to_string(i) + ":DISPLAY?";
    if(std::stoi(receive(cmd).front()) == 1) {
      channels_.push_back(i);
    }
  }
}

int DSO9254ADevice::waitForTrigger() {

  LOG(DEBUG) << "Waiting for triggers...";

  std::string query = ":ter?";

  int trigger = 0;
  while(trigger == 0) {
    mDelay(10);
    std::string s = AuxiliaryDevice<iface_ipsocket>::receive(query).front();
    trigger = std::stoi(s);
  }

  LOG(DEBUG) << "Received trigger. Done.";

  return trigger;
}

pearydata DSO9254ADevice::getData() {

  std::vector<double> values;

  LOG(INFO) << "Retrieving data from scope...";
  std::string cmd = "wav:data?";
  auto data = AuxiliaryDevice<iface_ipsocket>::receive(cmd);

  LOG(INFO) << "Crunching numbers...";

  if(data.empty()) {
    return pearydata();
  }

  std::istringstream ss(data.front());
  std::string token;

  while(std::getline(ss, token, ',')) {
    values.push_back(std::stod(token));
  }
  // return values;
  return pearydata();
}

pearydataVector DSO9254ADevice::getData(const unsigned int) {
  LOG(FATAL) << "Data readback not implemented for this device";
  throw caribou::DeviceImplException("Data readback not implemented for this device");
}

void DSO9254ADevice::getBinaryData(std::string out) {
  auto p = AuxiliaryDevice<iface_ipsocket>::receive(":WAVeform:PREamble?");

  std::ofstream file(out + ".txt");
  file << p.front();

  LOG(DEBUG) << "Retrieving data from scope...";
  LOG(DEBUG) << "Preamble: " << p.front();

  std::istringstream s(p.front());
  std::string str;
  std::vector<std::string> preamble;

  while(std::getline(s, str, ',')) {
    preamble.push_back(str);
  }

  LOG(DEBUG) << "Points";
  size_t points = std::stoul(preamble[2]);
  LOG(DEBUG) << "segments";
  size_t segments = preamble.size() == 25 ? std::stoul(preamble[24]) : 1;

  size_t size = 2;

  //+3 bytes for streaming header and end character
  auto data = InterfaceManager::getInterface<iface_ipsocket>(_interfaceConfig)
                .read_binary(":WAVEFORM:DATA?", static_cast<unsigned int>(points * size * segments + 3));

  std::ofstream file2(out + ".dat", std::ofstream::binary);
  file2.write(reinterpret_cast<char*>(&data[2]), static_cast<long int>(points * size * segments));
}

pearyRawData DSO9254ADevice::fetch_channel_data(int channel) {

  LOG(DEBUG) << "Retrieving data from scope...";

  std::string cmd = ":WAVeform:SOURce CHANnel" + std::to_string(channel);
  send(cmd);
  LOG(DEBUG) << "Selected channel " << channel;

  // Receive preamble from scope:
  auto p = receive(":WAVeform:PREamble?").front();
  LOG(DEBUG) << "Preamble: " << p;
  pearyRawData preamble(p.begin(), p.end());

  // Read preamble
  size_t points = std::stoul(receive(":WAVeform:POINts?").front());
  LOG(DEBUG) << "Points: " << points;
  size_t segments = std::stoul(receive(":WAVeform:SEGMented:COUNt?").front());
  LOG(DEBUG) << "Segments: " << segments;
  if(segments == 0) {
    LOG(INFO) << "Found zero segments...?";
    throw NoDataAvailable();
  }

  // We have 16bit per point (?) so request two packets of uint8_t?
  size_t size = 2;

  // Receive waveform data:
  // points x segments x 2 +3 bytes for streaming header and end character
  auto ifdata = InterfaceManager::getInterface<iface_ipsocket>(_interfaceConfig)
                  .read_binary(":WAVEFORM:DATA?", static_cast<unsigned int>(points * size * segments + 3));
  LOG(DEBUG) << "Retrieved " << ifdata.size() << " words from interface";

  // Allocate new vector of correct data type and length - drop three words of header & trailer:
  auto datasize = ifdata.size() - 3;
  uintptr_t a;
  LOG(DEBUG) << "Calculated data size of " << datasize << ", in: " << sizeof(ifdata[0]) << " out: " << sizeof(a);
  pearyRawData data(datasize * sizeof(ifdata[0]) / sizeof(uintptr_t));
  LOG(DEBUG) << "Data vector allocated, length " << data.size();
  std::memcpy(&data[0], ifdata.data() + 2, datasize);
  LOG(DEBUG) << "Succeeded in memcpy bit-magic, constructing return vector";

  // Build data vector:
  pearyRawData channel_data;
  // 0th word:   total channel data length
  channel_data.push_back(2 + preamble.size() + data.size());
  // 1st word:   preamble length (n)
  channel_data.push_back(preamble.size());
  // n words:    preamble
  channel_data.insert(channel_data.end(), preamble.begin(), preamble.end());
  // n+1st word: data length (m)
  channel_data.push_back(data.size());
  // m words:    data
  channel_data.insert(channel_data.end(), data.begin(), data.end());

  LOG(DEBUG) << "Returning channel data with " << channel_data.size() << " words";
  return channel_data;
}

void DSO9254ADevice::daqStart(void) {
  // ensure only one daq thread is running
  if(daqRunning) {
    LOG(WARNING) << "Data aquisition is already running";
    return;
  }

  if(_daqThread.joinable()) {
    LOG(WARNING) << "DAQ thread is already running";
    return;
  }

  _daqContinue.test_and_set();

  // reset trigger counter
  triggers_seen_ = 0;

  // Clear acquisition flag:
  receive(":ADER?");

  // Start
  send(":RUN");
  LOG(DEBUG) << "DAQ running";
  daqRunning = true;
}

void DSO9254ADevice::daqStop(void) {
  if(_daqThread.joinable()) {
    // signal to daq thread that we want to stop and wait until it does
    _daqContinue.clear();
    _daqThread.join();
  }
  _daqContinue.clear();

  // stop acquisition
  send(":STOP");
  mDelay(500);

  // report trigger statistics
  LOG(STATUS) << "Seen " << triggers_seen_ << " plus " << std::stoul(receive("WAVeform:SEGMented:COUNt?").front())
              << " lost in scope memory";

  daqRunning = false;
}

pearyRawData DSO9254ADevice::getRawData() { // following example from manual

  // acquisition should loop via caribouProducer::runLoop, so that gracefull stop is possible
  if(!scope_acq_running) { // avoid excessive on/off
    send(":STOP");         // stop
    receive(":PDER?");     // progressing done event register
    receive(":ADER?");     // clear acquistion done event register
    // CAUTON these^ MUST be RECEIVE, otherwise it is piling up and messes up later readings!
    send(":SINGle");
    scope_acq_running = true;
    scope_acq_start = std::chrono::system_clock::now();
  }

  // check if we got triggered
  auto now = std::chrono::system_clock::now();
  std::chrono::duration<double> running = now - scope_acq_start;
  LOG_PROGRESS(DEBUG, "sec") << "Acquiring segments. Waiting for [s]: " << running.count();
  if(std::stoi(receive(":ADER?").front()) == 0) { // not enough segments
    // without timeout
    mDelay(50);
    throw NoDataAvailable();
  }

  // prefere to use this; "WAVeform:SEGMented:COUNt?" can be delayed.
  triggers_seen_ += std::stoul(receive("ACQuire:SEGMented:COUNt?").front());

  scope_acq_running = false;
  mDelay(250); // 200 seems to be mandatory to prevent empty readings

  // reading part
  pearyRawData data;
  for(auto ch : channels_) {
    auto channel_data = fetch_channel_data(ch);
    data.insert(data.end(), channel_data.begin(), channel_data.end());
  }

  LOG(DEBUG) << "Triggers seen: " << triggers_seen_ << ".";
  return data;
}

void DSO9254ADevice::runDaq(void) {
  size_t count = 0;
  std::vector<int> channels;
  bool flag;

  // enable channels
  for(int i = 1; i <= 4; i++) {
    std::string cmd = ":CHANNEL" + std::to_string(i) + ":DISPLAY?";
    if(std::stoi(AuxiliaryDevice<iface_ipsocket>::receive(cmd).front()) == 1) {
      channels.push_back(i);
    }
  }

  AuxiliaryDevice<iface_ipsocket>::receive(":ADER?"); // clear flag

  for(;;) {
    AuxiliaryDevice<iface_ipsocket>::send(":SINGLE");

    LOG(DEBUG) << "Count = " << count;

    while(std::stoi(AuxiliaryDevice<iface_ipsocket>::receive(":ADER?").front()) == 0) {

      // check if buffer full
      flag = _daqContinue.test_and_set();
      if(!flag) {
        AuxiliaryDevice<iface_ipsocket>::send(":STOP");
        break;
      }

      mDelay(1000);
    }

    LOG(DEBUG) << "Reading block " << count;

    for(auto ch : channels) {
      std::string file = "data_" + std::to_string(count) + "_ch" + std::to_string(ch);
      std::string cmd = ":WAVeform:SOURce CHANnel" + std::to_string(ch);
      AuxiliaryDevice<iface_ipsocket>::send(cmd);
      getBinaryData(file);

      // clear waveform memory to avoid errors
      std::string clr = ":WMEMory" + std::to_string(ch) + ":CLEar";
      AuxiliaryDevice<iface_ipsocket>::send(clr);
    }

    if(!flag) {
      return;
    }

    count++;
  }
}

void DSO9254ADevice::printErrors() {

  for(int i = 0; i < 31; i++) {
    std::string query = ":SYSTem:ERRor? STRing";
    std::string ers = receive(query).front();
    LOG(INFO) << ers;
  }

  return;
}
