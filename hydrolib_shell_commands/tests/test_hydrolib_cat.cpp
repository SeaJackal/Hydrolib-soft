// Intentionally left blank before. Now adding tests for hydrolib_cat.hpp.
#include "hydrolib_cat.hpp"

#include <deque>
#include <string>
#include <string_view>
#include <vector>
#include <sstream>
#include <iostream>
#include <unistd.h>

#include <gtest/gtest.h>

using hydrolib::device::DeviceManager;
using hydrolib::device::StreamDevice;
using hydrolib::shell::Cat;
using hydrolib::shell::g_is_running;

namespace
{

// Simple stream with byte-wise read/write free functions compatible with StreamDevice
struct FakeStream
{
    FakeStream() = default;

    explicit FakeStream(std::string_view initial_data)
    {
        input_.insert(input_.end(), initial_data.begin(), initial_data.end());
    }

    std::deque<char> input_;
    std::vector<char> output_; // unused here, but kept for completeness

    // If set true, the next read will return -1 once and reset to false
    bool fail_next_read_ = false;
};

[[maybe_unused]] int read(FakeStream &stream, void *dest, unsigned length)
{
    if (stream.fail_next_read_)
    {
        stream.fail_next_read_ = false;
        return -1;
    }

    if (length == 0)
    {
        return 0;
    }

    if (stream.input_.empty())
    {
        return 0;
    }

    const unsigned available = static_cast<unsigned>(stream.input_.size());
    const unsigned to_copy = std::min(length, available);

    auto *char_dest = static_cast<char *>(dest);
    for (unsigned i = 0; i < to_copy; ++i)
    {
        char_dest[i] = stream.input_.front();
        stream.input_.pop_front();
    }

    return static_cast<int>(to_copy);
}

[[maybe_unused]] int write(FakeStream &stream, const void *source, unsigned length)
{
    const auto *char_source = static_cast<const char *>(source);
    stream.output_.insert(stream.output_.end(), char_source, char_source + length);
    return static_cast<int>(length);
}

// Utility to run Cat and capture std::cout output.
struct CoutCapture
{
    CoutCapture() : old_buf_(std::cout.rdbuf(oss_.rdbuf())) {}
    ~CoutCapture() { std::cout.rdbuf(old_buf_); }
    std::string str() const { return oss_.str(); }

    std::ostringstream oss_;
    std::streambuf *old_buf_;
};

// Reset POSIX getopt state before each test case that uses getopt
void ResetGetopt()
{
    optind = 1;
    opterr = 0;
}

} // namespace

TEST(HydrolibCat, PrintsUsageOnHelpAndStops)
{
    ResetGetopt();
    CoutCapture capture;

    DeviceManager mgr({});

    char arg0[] = "cat";
    char arg1[] = "-h";
    char *argv[] = {arg0, arg1};

    g_is_running = true;
    int rc = Cat(2, argv);

    EXPECT_EQ(0, rc);
    EXPECT_EQ("Usage: cat [DEVICE_NAME]", capture.str());
    EXPECT_FALSE(g_is_running);
}

TEST(HydrolibCat, InvalidOptionSetsErrorAndPrintsCode)
{
    ResetGetopt();
    CoutCapture capture;

    DeviceManager mgr({});

    char arg0[] = "cat";
    char arg1[] = "-x";
    char *argv[] = {arg0, arg1};

    g_is_running = true;
    int rc = Cat(2, argv);

    EXPECT_EQ(-1, rc);
    EXPECT_EQ("Invalid option: x", capture.str());
    EXPECT_FALSE(g_is_running);
}

TEST(HydrolibCat, DeviceNotFoundSetsError)
{
    ResetGetopt();
    CoutCapture capture;

    DeviceManager mgr({});

    char arg0[] = "cat";
    char arg1[] = "no_such_device";
    char *argv[] = {arg0, arg1, nullptr};

    g_is_running = true;
    int rc = Cat(2, argv);

    EXPECT_EQ(-1, rc);
    EXPECT_EQ("Device not found: no_such_device", capture.str());
    EXPECT_FALSE(g_is_running);
}

TEST(HydrolibCat, ReturnsMinusTwoOnStreamReadError)
{
    ResetGetopt();
    CoutCapture capture;

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
