/**
 * Caribou ipsocket interface class implementation
 */

#include <cerrno>
#include <cstring>
#include <sstream>
#include <string>

// OS socket support
#include <arpa/inet.h>
#include <sys/socket.h>

#include "utils/log.hpp"
#include "utils/utils.hpp"

#include "ipsocket.hpp"

#define BUFFERSIZE 4096

using namespace caribou;

iface_ipsocket::iface_ipsocket(const configuration_type& config) : Interface(config) {

  LOG(TRACE) << "New IP sockets interface requested.";
  auto ip_address = split_ip_address(devicePath());
  LOG(TRACE) << "IP address: " << ip_address.first;
  LOG(TRACE) << "      port: " << ip_address.second;

  // Configure Socket and address
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(ip_address.second);
  inet_aton(ip_address.first.c_str(), &(address.sin_addr));

  mysocket_ = socket(AF_INET, SOCK_STREAM, 0);

  std::stringstream ss;
  ss << inet_ntoa(address.sin_addr);

  LOG(TRACE) << "Connecting to " << ss.str();
  int retval = ::connect(mysocket_, reinterpret_cast<struct sockaddr*>(&address), sizeof(address));

  if(retval == 0) {
    LOG(TRACE) << "Connection to server at " << ss.str() << " established";
  } else {
    LOG(FATAL) << "Connection to server at " << ss.str() << " failed, errno " << errno;
    throw CommunicationError("Connection to server at " + ss.str() + " failed, errno " + std::to_string(errno));
  }
}

std::pair<std::string, uint16_t> iface_ipsocket::split_ip_address(const std::string& address) {

  std::pair<std::string, uint16_t> ip_and_port;

  std::istringstream ss(address);
  std::string token;
  size_t tokens = 0;

  while(std::getline(ss, token, ':')) {
    if(tokens == 0) {
      ip_and_port.first = token;
    } else if(tokens == 1) {
      ip_and_port.second = static_cast<uint16_t>(std::stoi(token));
    }
    tokens++;
  }

  if(tokens != 2) {
    throw ConfigInvalid("Cannot retrieve port and IP address from provided device path: \"" + address + "\"");
  }
  return ip_and_port;
}

std::string iface_ipsocket::trim(const std::string& str, const std::string& delims) {
  size_t b = str.find_first_not_of(delims);
  size_t e = str.find_last_not_of(delims);
  if(b == std::string::npos || e == std::string::npos) {
    return "";
  }
  return std::string(str, b, e - b + 1);
}

std::string iface_ipsocket::cleanCommandString(std::string& str) {

  // str = trim(str);
  LOG(TRACE) << "Trimmed command: " << str;
  // If there are "" then we should take the whole string
  if(!str.empty() && str[0] == '\"') {
    if(str.find('\"', 1) != str.size() - 1) {
      throw std::invalid_argument("remaining data at end");
    }
    return str.substr(1, str.size() - 2);
  }
  // Otherwise read a single string
  return str;
}

iface_ipsocket::~iface_ipsocket() {
  auto ip_address = split_ip_address(devicePath());
  LOG(TRACE) << "Closing IPSocket device. IP address: " << ip_address.first << " port: " << ip_address.second;
  // When finished, close the sockets
  close(mysocket_);
}

ipsocket_t iface_ipsocket::write(const ipsocket_t& payload) {

  return write_nolock(payload);
}

ipsocket_t iface_ipsocket::write_nolock(const ipsocket_t& payload) {
  ipsocket_t data = payload;
  data = cleanCommandString(data);

  if(data.back() != '\n') {
    LOG(TRACE) << "Add carriage return to command.";
    data += '\n';
  }

  LOG(TRACE) << "Sending command \"" << data << "\"";
  auto retval = send(mysocket_, data.c_str(), strlen(data.c_str()), 0);

  if(retval < 0) {
    LOG(ERROR) << "Command returned: " << retval << std::endl;
    throw CommunicationError("Server returned errno " + std::to_string(errno));
  }
  return std::string();
}

std::vector<uint8_t> iface_ipsocket::read_binary(const ipsocket_t& query, const unsigned int length) {
  std::vector<uint8_t> data;

  // Send query command:
  write_nolock(query);

  data.resize(length);
  recv(mysocket_, data.data(), length, MSG_WAITALL);

  return data;
}

iface_ipsocket::dataVector_type iface_ipsocket::read(const ipsocket_t& query, const unsigned int) {
  dataVector_type data;
  ipsocket_t dataword;

  // Send query command:
  write_nolock(query);

  long int msglen = 0;
  char buffer[BUFFERSIZE];
  int msgs = 0;
  while(buffer[msglen - 1] != '\n') {
    LOG(TRACE) << "Receiving buffer from socket...";
    // retrieve response:
    msglen = recv(mysocket_, buffer, BUFFERSIZE, 0);
    LOG(TRACE) << "Received: \"" << std::string(buffer) << "\"";
    // Terminate the string:
    buffer[msglen] = '\0';
    dataword += std::string(buffer);
    // Reset buffer:
    buffer[0] = '\0';
    msgs++;
  }

  data.push_back(dataword);
  LOG(TRACE) << "Received " << msgs << " buffers.";
  return data;
}
