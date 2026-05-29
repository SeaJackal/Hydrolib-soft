#include "pty_line_device.hpp"

#include <pty.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include <array>
#include <cassert>
#include <cctype>
#include <limits>
#include <random>
#include <vector>

namespace hydrolib::shell::tests {

namespace {
struct PollTimeoutMs {
  int value;
};

int PollInput(int file_descriptor, PollTimeoutMs poll_timeout_ms) {
  pollfd pfd{};
  pfd.fd = file_descriptor;
  pfd.events = POLLIN;

  const auto poll_result = ::poll(&pfd, 1, poll_timeout_ms.value);
  assert(poll_result >= 0 && "poll failed");
  return poll_result;
}

ssize_t ReadBytes(int file_descriptor, void* dest, std::size_t length) {
  const auto read_bytes = ::read(file_descriptor, dest, length);
  assert(read_bytes >= 0 && "read failed");
  return read_bytes;
}
}  // namespace

std::string GenerateRandomString(RandomSeed seed, std::size_t length) {
  std::mt19937_64 gen(seed.value);

  std::vector<char> allowed;
  allowed.reserve(std::numeric_limits<char>::max() + 1);

  constexpr char kBackspace = '\b';

  for (int i = 0; i <= std::numeric_limits<char>::max(); ++i) {
    const auto byte = static_cast<char>(i);
    if ((std::isprint(byte) != 0) || (std::isblank(byte) != 0) ||
        byte == kBackspace) {
      allowed.push_back(byte);
    }
  }

  std::uniform_int_distribution<std::size_t> dist(0, allowed.size() - 1);
  std::string out;
  out.reserve(length);
  for (std::size_t i = 0; i < length; ++i) {
    out.push_back(allowed[dist(gen)]);
  }
  return out;
}

PtyLineDevice::PtyLineDevice() {
  int master = -1;
  int slave = -1;
  const int open_rc = ::openpty(&master, &slave, nullptr, nullptr, nullptr);
  assert(open_rc == 0 && "openpty failed");
  master_fd_ = master;
  slave_fd_ = slave;

  termios tio{};
  const int getattr_rc = ::tcgetattr(slave_fd_, &tio);
  assert(getattr_rc == 0 && "tcgetattr failed");
  tio.c_lflag |= ICANON;
  tio.c_lflag |= ECHO;
  tio.c_cc[VERASE] = '\b';
  const int setattr_rc = ::tcsetattr(slave_fd_, TCSANOW, &tio);
  assert(setattr_rc == 0 && "tcsetattr failed");
}

PtyLineDevice::~PtyLineDevice() {
  if (master_fd_ != -1) {
    const int close_rc = ::close(master_fd_);
    assert(close_rc == 0 && "close master fd failed");
  }
  if (slave_fd_ != -1) {
    const int close_rc = ::close(slave_fd_);
    assert(close_rc == 0 && "close slave fd failed");
  }
}

PtyLineDevice::ProcessResult PtyLineDevice::Process(std::string_view chars) const {
  WriteAll(master_fd_, std::span(chars.data(), chars.size()));

  auto echo_str = ReadEcho(master_fd_);

  constexpr std::string_view kEnter = "\n";
  WriteAll(master_fd_, std::span(kEnter.data(), kEnter.size()));
  ReadEcho(master_fd_);

  ProcessResult result{};
  result.to_program = ReadLine(slave_fd_);
  result.to_echo = std::move(echo_str);
  return result;
}

void PtyLineDevice::WriteAll(int file_descriptor, std::span<const char> bytes) {
  std::size_t offset = 0;
  while (offset < bytes.size()) {
    auto remaining = bytes.subspan(offset);
    const auto written_bytes =
        ::write(file_descriptor, remaining.data(), remaining.size());
    assert(written_bytes >= 0 && "write failed");
    offset += static_cast<std::size_t>(written_bytes);
  }
}

std::string PtyLineDevice::ReadLine(int file_descriptor) {
  constexpr int kPollTimeoutMs = 200;
  constexpr std::size_t kReadBufferSize = 256;

  std::string out;
  pollfd pfd{};
  pfd.fd = file_descriptor;
  pfd.events = POLLIN;
  for (;;) {
    const auto poll_result = ::poll(&pfd, 1, kPollTimeoutMs);
    if (poll_result == 0) {
      assert(false && "timeout reading line");
      return {};
    }

    std::array<char, kReadBufferSize> buf{};
    const ssize_t read_bytes = ::read(file_descriptor, buf.data(), buf.size());
    assert(read_bytes >= 0 && "read failed");
    if (read_bytes == 0) {
      assert(false && "read returned 0 bytes");
      return {};
    }
    out.append(buf.data(), read_bytes - 1);
    return out;
  }
}

std::string PtyLineDevice::ReadEcho(int file_descriptor) {
  constexpr int kPollTimeoutMs = 200;
  constexpr std::size_t kReadBufferSize = 256;

  std::string out;
  const auto first_poll =
      PollInput(file_descriptor, PollTimeoutMs{kPollTimeoutMs});
  if (first_poll == 0) {
    return {};
  }

  for (;;) {
    std::array<char, kReadBufferSize> buf{};
    const ssize_t read_bytes =
        ReadBytes(file_descriptor, buf.data(), buf.size());
    assert(read_bytes != 0 && "read returned 0 bytes");
    out.append(buf.data(), static_cast<std::size_t>(read_bytes));

    const auto next_poll = PollInput(file_descriptor, PollTimeoutMs{0});
    if (next_poll == 0) {
      return out;
    }
  }
}

}  // namespace hydrolib::shell::tests
