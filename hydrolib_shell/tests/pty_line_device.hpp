#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

namespace hydrolib::shell::tests {

struct RandomSeed {
  int value;
};

std::string GenerateRandomString(RandomSeed seed, std::size_t length);

class PtyLineDevice {
 public:
  struct ProcessResult {
    std::string to_program;
    std::string to_echo;
  };

  PtyLineDevice();

  PtyLineDevice(const PtyLineDevice&) = delete;
  PtyLineDevice(PtyLineDevice&&) = delete;
  PtyLineDevice& operator=(const PtyLineDevice&) = delete;
  PtyLineDevice& operator=(PtyLineDevice&&) = delete;

  ~PtyLineDevice();

  [[nodiscard]] ProcessResult Process(std::string_view chars) const;

 private:
  static void WriteAll(int file_descriptor, std::span<const char> bytes);
  static std::string ReadLine(int file_descriptor);
  static std::string ReadEcho(int file_descriptor);

  int master_fd_ = -1;
  int slave_fd_ = -1;
};

}  // namespace hydrolib::shell::tests
