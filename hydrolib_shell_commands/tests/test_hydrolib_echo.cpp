#include <gtest/gtest.h>
#include <unistd.h>

#include <deque>
#include <string>
#include <string_view>
#include <vector>

#include "hydrolib_echo.hpp"

using hydrolib::device::DeviceManager;
using hydrolib::device::StreamDevice;
using hydrolib::shell::cout;
using hydrolib::shell::Echo;
using hydrolib::shell::IsRunning();
using hydrolib::shell::Ostream;
using hydrolib::shell::StreamWrapper;

namespace {

// Simple stream with byte-wise read/write free functions compatible with
// StreamDevice
struct FakeStream {
  FakeStream() = default;

  explicit FakeStream(std::string_view initial_data) {
    input_.insert(input_.end(), initial_data.begin(), initial_data.end());
  }

  std::deque<char> input_;
  std::vector<char> output_;

  // If set true, the next write will return -1 once and reset to false
  bool fail_next_write_ = false;
};

[[maybe_unused]] int read(FakeStream &stream, void *dest, unsigned length) {
  if (length == 0) {
    return 0;
  }

  if (stream.input_.empty()) {
    return 0;
  }

  const unsigned available = static_cast<unsigned>(stream.input_.size());
  const unsigned to_copy = std::min(length, available);

  auto *char_dest = static_cast<char *>(dest);
  for (unsigned i = 0; i < to_copy; ++i) {
    char_dest[i] = stream.input_.front();
    stream.input_.pop_front();
  }

  return static_cast<int>(to_copy);
}

[[maybe_unused]] int write(FakeStream &stream, const void *source,
                           unsigned length) {
  if (stream.fail_next_write_) {
    stream.fail_next_write_ = false;
    return -1;
  }
  const auto *char_source = static_cast<const char *>(source);
  stream.output_.insert(stream.output_.end(), char_source,
                        char_source + length);
  return static_cast<int>(length);
}

static std::string OutputAsString(const FakeStream &stream) {
  return std::string(stream.output_.begin(), stream.output_.end());
}

// Reset POSIX getopt state before each test case that uses getopt
void ResetGetopt() {
  optind = 1;
  opterr = 0;
}

}  // namespace

TEST(HydrolibEcho, PrintsUsageOnHelpAndStops) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  DeviceManager mgr({});

  char arg0[] = "echo";
  char arg1[] = "-h";
  char *argv[] = {arg0, arg1};

  SetRunningTrue();
  int rc = Echo(2, argv);

  EXPECT_EQ(0, rc);
  EXPECT_EQ("Usage: echo [DEVICE_NAME] [STRING]", OutputAsString(cout_stream));
  EXPECT_FALSE(IsRunning());
}

TEST(HydrolibEcho, InvalidOptionSetsErrorAndPrintsCode) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  DeviceManager mgr({});

  char arg0[] = "echo";
  char arg1[] = "-x";
  char *argv[] = {arg0, arg1};

  SetRunningTrue();
  int rc = Echo(2, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("Invalid option: x", OutputAsString(cout_stream));
  EXPECT_FALSE(IsRunning());
}

TEST(HydrolibEcho, DeviceNotFoundSetsError) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  DeviceManager mgr({});

  char arg0[] = "echo";
  char arg1[] = "no_such_device";
  char arg2[] = "hello";
  char *argv[] = {arg0, arg1, arg2, nullptr};

  SetRunningTrue();
  int rc = Echo(3, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("Device not found: no_such_device", OutputAsString(cout_stream));
  EXPECT_FALSE(IsRunning());
}

TEST(HydrolibEcho, NoDeviceSpecified) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  DeviceManager mgr({});

  char arg0[] = "echo";
  char *argv[] = {arg0, nullptr};

  SetRunningTrue();
  int rc = Echo(1, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("No device specified", OutputAsString(cout_stream));
  EXPECT_FALSE(IsRunning());
}

TEST(HydrolibEcho, NoDataSpecified) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  FakeStream stream;
  StreamDevice<FakeStream> dev("stream1", stream);
  DeviceManager mgr({&dev});

  char arg0[] = "echo";
  char arg1[] = "stream1";
  char *argv[] = {arg0, arg1, nullptr};

  SetRunningTrue();
  int rc = Echo(2, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("No data specified", OutputAsString(cout_stream));
  EXPECT_FALSE(IsRunning());
}

TEST(HydrolibEcho, TooManyArguments) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  FakeStream stream;
  StreamDevice<FakeStream> dev("stream1", stream);
  DeviceManager mgr({&dev});

  char arg0[] = "echo";
  char arg1[] = "stream1";
  char arg2[] = "hello";
  char arg3[] = "extra";
  char *argv[] = {arg0, arg1, arg2, arg3, nullptr};

  SetRunningTrue();
  int rc = Echo(4, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("Too many arguments", OutputAsString(cout_stream));
  EXPECT_FALSE(IsRunning());
}

TEST(HydrolibEcho, WritesAllDataToDevice) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  FakeStream stream;
  StreamDevice<FakeStream> dev("stream1", stream);
  DeviceManager mgr({&dev});

  char arg0[] = "echo";
  char arg1[] = "stream1";
  char arg2[] = "hello_world";
  char *argv[] = {arg0, arg1, arg2, nullptr};

  SetRunningTrue();
  int rc = Echo(3, argv);

  EXPECT_EQ(0, rc);
  EXPECT_EQ("hello_world", OutputAsString(stream));
}

TEST(HydrolibEcho, ReturnsMinusTwoOnStreamWriteError) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  FakeStream stream;
  stream.fail_next_write_ = true;
  StreamDevice<FakeStream> dev("stream1", stream);
  DeviceManager mgr({&dev});

  char arg0[] = "echo";
  char arg1[] = "stream1";
  char arg2[] = "data";
  char *argv[] = {arg0, arg1, arg2, nullptr};

  SetRunningTrue();
  int rc = Echo(3, argv);

  EXPECT_EQ(-2, rc);
}
