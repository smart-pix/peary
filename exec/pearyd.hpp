/**
 * Caribou Peary Daemon
 */

#ifndef CARIBOU_DAEMON_H
#define CARIBOU_DAEMON_H

#include <array>
#include <vector>

#include "device/DeviceManager.hpp"

// -----------------------------------------------------------------------------
// common header and message handling

static const char* PEARYD_PROTOCOL_VERSION = "1";
enum class Status : uint16_t {
  Ok = 0,
  // message-level errors
  MessageInvalid = 2,
  // command-level errors
  CommandUnknown = 16,
  CommandNotEnoughArguments = 17,
  CommandTooManyArguments = 18,
  CommandInvalidArgument = 19,
  CommandFailure = 20,
};

struct Header {
  std::array<uint16_t, 2> encoded;

  // direct compatibility w/ write_msg(...)
  const uint8_t* data() const { return reinterpret_cast<const uint8_t*>(encoded.data()); }
  size_t size() const { return 4u; }

  uint16_t sequence() const { return ntohs(encoded[0]); }
  Status status() const { return static_cast<Status>(ntohs(encoded[1])); }

  Header() = default;
  Header(const void* buffer) { std::memcpy(encoded.data(), buffer, 4); }
  void set_sequence(uint16_t sequence_) { encoded[0] = htons(sequence_); }
  void set_status(Status status_) { encoded[1] = htons(static_cast<uint16_t>(status_)); }
};

struct ReplyBuffer {
  Header header;
  std::string payload;

  void clear() {
    header.set_sequence(0);
    header.set_status(Status::Ok);
    payload.clear();
  }
  void set_sequence(uint16_t seq) { header.set_sequence(seq); }
  void set_success() { header.set_status(Status::Ok); }
  void set_status(Status status) { header.set_status(status); }
};

void cleanup();
void terminate_success();
void terminate_failure(std::string err);
void terminate_errno();
void termination_handler(int);
void configure_signals();
bool read_fixed_into(int fd, size_t length, void* buffer);
bool read_msg_into(int fd, std::vector<uint8_t>& buffer);
size_t split_once(const char* txt, size_t len, char separator);
std::vector<std::string> split(const char* txt, size_t len, char separator);
bool check_num_args(const std::vector<std::string>& args, size_t expected, ReplyBuffer& reply);
void do_device_list_registers(caribou::Device& device, ReplyBuffer& reply);
void do_device_list_commands(caribou::Device& device, ReplyBuffer& reply);
void do_device_list_components(caribou::Device& device, ReplyBuffer& reply);
void do_device_get_register(caribou::Device& device, const std::vector<std::string>& args, ReplyBuffer& reply);
void do_device_get_registers(caribou::Device& device, ReplyBuffer& reply);
void do_device_get_memory(caribou::Device& device, const std::vector<std::string>& args, ReplyBuffer& reply);
void do_device_set_register(caribou::Device& device, const std::vector<std::string>& args, ReplyBuffer& reply);
void do_device_set_memory(caribou::Device& device, const std::vector<std::string>& args, ReplyBuffer& reply);
void do_device_get_current(caribou::Device& device, const std::vector<std::string>& args, ReplyBuffer& reply);
void do_device_set_current(caribou::Device& device, const std::vector<std::string>& args, ReplyBuffer& reply);
void do_device_get_voltage(caribou::Device& device, const std::vector<std::string>& args, ReplyBuffer& reply);
void do_device_set_voltage(caribou::Device& device, const std::vector<std::string>& args, ReplyBuffer& reply);
void do_device_switch_on(caribou::Device& device, const std::vector<std::string>& args, ReplyBuffer& reply);
void do_device_switch_off(caribou::Device& device, const std::vector<std::string>& args, ReplyBuffer& reply);
void do_device(caribou::DeviceManager& mgr,
               const std::string& cmd,
               const std::vector<std::string>& args,
               ReplyBuffer& reply);
void do_list_devices(caribou::DeviceManager& mgr, ReplyBuffer& reply);
void do_add_device(caribou::DeviceManager& mgr, const std::vector<std::string>& args, ReplyBuffer& reply);
void do_clear_devices(caribou::DeviceManager& mgr, ReplyBuffer& reply);
void do_protocol_version(ReplyBuffer& reply);
void process_request(caribou::DeviceManager& mgr, const std::vector<uint8_t>& request, ReplyBuffer& reply);
void show_help();
void parse_args(int argc, char* argv[], uint16_t& port, std::string& verbosity);

#endif /* CARIBOU_DAEMON_H */
