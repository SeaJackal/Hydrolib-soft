// Intentionally left blank before. Now adding tests for hydrolib_cat.hpp.
#include <gtest/gtest.h>
#include <unistd.h>

#include <deque>
#include <string>
#include <string_view>
#include <vector>

#include "hydrolib_cat.hpp"

using hydrolib::device::DeviceManager;
using hydrolib::device::StreamDevice;
using hydrolib::shell::Cat;
using hydrolib::shell::cout;
using hydrolib::shell::g_is_running;
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
  std::vector<char> output_;  // unused here, but kept for completeness

  // If set true, the next read will return -1 once and reset to false
  bool fail_next_read_ = false;
};

[[maybe_unused]] int read(FakeStream &stream, void *dest, unsigned length) {
  if (stream.fail_next_read_) {
    stream.fail_next_read_ = false;
    return -1;
  }

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

TEST(HydrolibCat, PrintsUsageOnHelpAndStops) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  DeviceManager mgr({});

  char arg0[] = "cat";
  char arg1[] = "-h";
  char *argv[] = {arg0, arg1};

  g_is_running = true;
  int rc = Cat(2, argv);

  EXPECT_EQ(0, rc);
  EXPECT_EQ("Usage: cat [DEVICE_NAME]", OutputAsString(cout_stream));
  EXPECT_FALSE(g_is_running);
}

TEST(HydrolibCat, InvalidOptionSetsErrorAndPrintsCode) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  DeviceManager mgr({});

  char arg0[] = "cat";
  char arg1[] = "-x";
  char *argv[] = {arg0, arg1};

  g_is_running = true;
  int rc = Cat(2, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("Invalid option: x", OutputAsString(cout_stream));
  EXPECT_FALSE(g_is_running);
}

TEST(HydrolibCat, DeviceNotFoundSetsError) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  DeviceManager mgr({});

  char arg0[] = "cat";
  char arg1[] = "no_such_device";
  char *argv[] = {arg0, arg1, nullptr};

  g_is_running = true;
  int rc = Cat(2, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("Device not found: no_such_device", OutputAsString(cout_stream));
  EXPECT_FALSE(g_is_running);
}

TEST(HydrolibCat, ReturnsMinusTwoOnStreamReadError) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  FakeStream stream("A");
  StreamDevice<FakeStream> dev("stream1", stream);
  DeviceManager mgr({&dev});

  char arg0[] = "cat";
  char arg1[] = "stream1";
  char *argv[] = {arg0, arg1, nullptr};

  g_is_running = true;
  stream.fail_next_read_ = true;
  int rc = Cat(2, argv);

  EXPECT_EQ(-2, rc);
}
