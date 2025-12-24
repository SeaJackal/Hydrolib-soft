#include "hydrolib_thruster_commands.hpp"

#include <deque>
#include <string>
#include <string_view>
#include <unistd.h>
#include <vector>

#include <gtest/gtest.h>

using hydrolib::device::DeviceManager;
using hydrolib::device::IThruster;
using hydrolib::device::ThrusterDevice;
using hydrolib::shell::cout;
using hydrolib::shell::g_is_running;
using hydrolib::shell::Ostream;
using hydrolib::shell::StreamWrapper;
using hydrolib::shell::ThrusterCommands;

namespace
{

// Simple stream with byte-wise read/write free functions compatible with
// StreamDevice
struct FakeStream
{
    FakeStream() = default;

    explicit FakeStream(std::string_view initial_data)
    {
        input_.insert(input_.end(), initial_data.begin(), initial_data.end());
    }

    std::deque<char> input_;
    std::vector<char> output_;

    // If set true, the next write will return -1 once and reset to false
    bool fail_next_write_ = false;
};

int read(FakeStream &stream, void *dest, unsigned length)
{
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

int write(FakeStream &stream, const void *source, unsigned length)
{
    if (stream.fail_next_write_)
    {
        stream.fail_next_write_ = false;
        return -1;
    }
    const auto *char_source = static_cast<const char *>(source);
    stream.output_.insert(stream.output_.end(), char_source,
                          char_source + length);
    return static_cast<int>(length);
}

static std::string OutputAsString(const FakeStream &stream)
{
    return std::string(stream.output_.begin(), stream.output_.end());
}

struct SimpleThrusterDriver
{
    SimpleThrusterDriver() = default;

    hydrolib::ReturnCode SetSpeed(int speed)
    {
        last_speed = speed;
        return hydrolib::ReturnCode::OK;
    }

    int GetSpeed() const { return last_speed; }

    int last_speed = 0;
};

class NonThrusterDevice : public hydrolib::device::Device
{
public:
    NonThrusterDevice()
        : Device("not_thruster", hydrolib::device::DeviceType::STREAM)
    {
    }
};

void ResetGetopt()
{
    optind = 1;
    opterr = 0;
}

} // namespace

TEST(HydrolibThruster, PrintsUsageOnHelpAndStops)
{
    ResetGetopt();
    FakeStream cout_stream;
    StreamWrapper<FakeStream> cout_wrapper(cout_stream);
    cout = Ostream(cout_wrapper);

    DeviceManager mgr({});

    char arg0[] = "thr";
    char arg1[] = "-h";
    char *argv[] = {arg0, arg1, nullptr};

    g_is_running = true;
    int rc = ThrusterCommands(2, argv);

    EXPECT_EQ(0, rc);
    EXPECT_TRUE(OutputAsString(cout_stream).find("Usage: thr") !=
                std::string::npos);
    EXPECT_FALSE(g_is_running);
}

TEST(HydrolibThruster, InvalidOptionSetsErrorAndPrintsCode)
{
    ResetGetopt();
    FakeStream cout_stream;
    StreamWrapper<FakeStream> cout_wrapper(cout_stream);
    cout = Ostream(cout_wrapper);

    DeviceManager mgr({});

    char arg0[] = "thr";
    char arg1[] = "-x";
    char *argv[] = {arg0, arg1, nullptr};

    g_is_running = true;
    int rc = ThrusterCommands(2, argv);

    EXPECT_EQ(-1, rc);
    EXPECT_EQ("Invalid option: x", OutputAsString(cout_stream));
    EXPECT_FALSE(g_is_running);
}

TEST(HydrolibThruster, InvalidCommandSetsError)
{
    ResetGetopt();
    FakeStream cout_stream;
    StreamWrapper<FakeStream> cout_wrapper(cout_stream);
    cout = Ostream(cout_wrapper);

    DeviceManager mgr({});

    char arg0[] = "thr";
    char arg1[] = "invalidcmd";
    char *argv[] = {arg0, arg1, nullptr};
    g_is_running = true;
    int rc = ThrusterCommands(2, argv);

    std::string output = OutputAsString(cout_stream);
    EXPECT_TRUE(output.find("Invalid command") != std::string::npos);
    EXPECT_EQ(-1, rc);
    EXPECT_FALSE(g_is_running);
}

TEST(HydrolibThruster, DeviceNotFoundSetsError)
{
    ResetGetopt();
    FakeStream cout_stream;
    StreamWrapper<FakeStream> cout_wrapper(cout_stream);
    cout = Ostream(cout_wrapper);

    DeviceManager mgr({});

    char arg0[] = "thr";
    char arg1[] = "setsp";
    char arg2[] = "no_such_device";
    char arg3[] = "100";
    char *argv[] = {arg0, arg1, arg2, arg3, nullptr};

    g_is_running = true;
    int rc = ThrusterCommands(4, argv);

    EXPECT_EQ(-1, rc);
    EXPECT_EQ("Device not found: no_such_device", OutputAsString(cout_stream));
    EXPECT_FALSE(g_is_running);
}

TEST(HydrolibThruster, DeviceIsNotAThrusterSetsError)
{
    ResetGetopt();
    FakeStream cout_stream;
    StreamWrapper<FakeStream> cout_wrapper(cout_stream);
    cout = Ostream(cout_wrapper);

    NonThrusterDevice non_thruster;
    DeviceManager mgr({&non_thruster});

    char arg0[] = "thr";
    char arg1[] = "setsp";
    char arg2[] = "not_thruster";
    char arg3[] = "100";
    char *argv[] = {arg0, arg1, arg2, arg3, nullptr};

    g_is_running = true;
    int rc = ThrusterCommands(4, argv);

    EXPECT_EQ(-1, rc);
    EXPECT_EQ("Device is not a thruster: not_thruster",
              OutputAsString(cout_stream));
    EXPECT_FALSE(g_is_running);
}

TEST(HydrolibThruster, NoSpeedValueSpecifiedForSetsp)
{
    ResetGetopt();
    FakeStream cout_stream;
    StreamWrapper<FakeStream> cout_wrapper(cout_stream);
    cout = Ostream(cout_wrapper);

    SimpleThrusterDriver driver;
    ThrusterDevice<SimpleThrusterDriver> thruster_dev("thruster1", driver);
    DeviceManager mgr({&thruster_dev});

    char arg0[] = "thr";
    char arg1[] = "setsp";
    char arg2[] = "thruster1";
    char *argv[] = {arg0, arg1, arg2, nullptr};

    g_is_running = true;
    int rc = ThrusterCommands(3, argv);

    EXPECT_EQ(-1, rc);
    EXPECT_EQ("No speed value specified", OutputAsString(cout_stream));
    EXPECT_FALSE(g_is_running);
}

TEST(HydrolibThruster, InvalidSpeedValue)
{
    ResetGetopt();
    FakeStream cout_stream;
    StreamWrapper<FakeStream> cout_wrapper(cout_stream);
    cout = Ostream(cout_wrapper);

    SimpleThrusterDriver driver;
    ThrusterDevice<SimpleThrusterDriver> thruster_dev("thruster1", driver);
    DeviceManager mgr({&thruster_dev});

    char arg0[] = "thr";
    char arg1[] = "setsp";
    char arg2[] = "thruster1";
    char arg3[] = "abc";
    char *argv[] = {arg0, arg1, arg2, arg3, nullptr};

    g_is_running = true;
    int rc = ThrusterCommands(4, argv);

    EXPECT_EQ(-1, rc);
    EXPECT_EQ("Invalid speed: abc", OutputAsString(cout_stream));
    EXPECT_FALSE(g_is_running);
}

TEST(HydrolibThruster, TooManyArgumentsForGetsp)
{
    ResetGetopt();
    FakeStream cout_stream;
    StreamWrapper<FakeStream> cout_wrapper(cout_stream);
    cout = Ostream(cout_wrapper);

    SimpleThrusterDriver driver;
    ThrusterDevice<SimpleThrusterDriver> thruster_dev("thruster1", driver);
    DeviceManager mgr({&thruster_dev});

    char arg0[] = "thr";
    char arg1[] = "getsp";
    char arg2[] = "thruster1";
    char arg3[] = "extra";
    char *argv[] = {arg0, arg1, arg2, arg3, nullptr};

    g_is_running = true;
    int rc = ThrusterCommands(4, argv);

    EXPECT_EQ(-1, rc);
    EXPECT_TRUE(OutputAsString(cout_stream).find("Invalid option") !=
                    std::string::npos ||
                OutputAsString(cout_stream).find("Too many arguments") !=
                    std::string::npos);
    EXPECT_FALSE(g_is_running);
}

TEST(HydrolibThruster, SetsPositiveSpeedSuccessfully)
{
    ResetGetopt();
    FakeStream cout_stream;
    StreamWrapper<FakeStream> cout_wrapper(cout_stream);
    cout = Ostream(cout_wrapper);

    SimpleThrusterDriver driver;
    ThrusterDevice<SimpleThrusterDriver> thruster_dev("thruster1", driver);
    DeviceManager mgr({&thruster_dev});

    char arg0[] = "thr";
    char arg1[] = "setsp";
    char arg2[] = "thruster1";
    char arg3[] = "100";
    char *argv[] = {arg0, arg1, arg2, arg3, nullptr};

    g_is_running = true;
    int rc = ThrusterCommands(4, argv);

    EXPECT_EQ(0, rc);
    EXPECT_EQ(100, driver.last_speed);
    EXPECT_TRUE(OutputAsString(cout_stream).find("Thruster set speed: 100") !=
                std::string::npos);
    EXPECT_TRUE(g_is_running);
}

TEST(HydrolibThruster, SetsNegativeSpeedSuccessfully)
{
    ResetGetopt();
    FakeStream cout_stream;
    StreamWrapper<FakeStream> cout_wrapper(cout_stream);
    cout = Ostream(cout_wrapper);

    SimpleThrusterDriver driver;
    ThrusterDevice<SimpleThrusterDriver> thruster_dev("thruster1", driver);
    DeviceManager mgr({&thruster_dev});

    char arg0[] = "thr";
    char arg1[] = "setsp";
    char arg2[] = "thruster1";
    char arg3[] = "-n";
    char arg4[] = "50";
    char *argv[] = {arg0, arg1, arg2, arg3, arg4, nullptr};

    g_is_running = true;
    int rc = ThrusterCommands(5, argv);

    EXPECT_EQ(0, rc);
    EXPECT_EQ(-50, driver.last_speed);
    EXPECT_TRUE(OutputAsString(cout_stream).find("Thruster set speed: -50") !=
                std::string::npos);
    EXPECT_TRUE(g_is_running);
}

TEST(HydrolibThruster, GetsSpeedSuccessfully)
{
    ResetGetopt();
    FakeStream cout_stream;
    StreamWrapper<FakeStream> cout_wrapper(cout_stream);
    cout = Ostream(cout_wrapper);

    SimpleThrusterDriver driver;
    driver.last_speed = 75;
    ThrusterDevice<SimpleThrusterDriver> thruster_dev("thruster1", driver);
    DeviceManager mgr({&thruster_dev});

    char arg0[] = "thr";
    char arg1[] = "getsp";
    char arg2[] = "thruster1";
    char *argv[] = {arg0, arg1, arg2, nullptr};

    g_is_running = true;
    int rc = ThrusterCommands(3, argv);

    EXPECT_EQ(0, rc);
    EXPECT_TRUE(OutputAsString(cout_stream).find("Thruster speed: 75") !=
                std::string::npos);
    EXPECT_TRUE(g_is_running);
}

TEST(HydrolibThruster, OptionNWithoutSetspFails)
{
    ResetGetopt();
    FakeStream cout_stream;
    StreamWrapper<FakeStream> cout_wrapper(cout_stream);
    cout = Ostream(cout_wrapper);

    SimpleThrusterDriver driver;
    ThrusterDevice<SimpleThrusterDriver> thruster_dev("thruster1", driver);
    DeviceManager mgr({&thruster_dev});

    char arg0[] = "thr";
    char arg1[] = "getsp";
    char arg2[] = "thruster1";
    char arg3[] = "-n";
    char *argv[] = {arg0, arg1, arg2, arg3, nullptr};

    g_is_running = true;
    int rc = ThrusterCommands(4, argv);

    EXPECT_EQ(-1, rc);
    EXPECT_TRUE(OutputAsString(cout_stream).find("Invalid option") !=
                std::string::npos);
    EXPECT_FALSE(g_is_running);
}

TEST(HydrolibThruster, ZeroSpeedSuccessfully)
{
    ResetGetopt();
    FakeStream cout_stream;
    StreamWrapper<FakeStream> cout_wrapper(cout_stream);
    cout = Ostream(cout_wrapper);

    SimpleThrusterDriver driver;
    ThrusterDevice<SimpleThrusterDriver> thruster_dev("thruster1", driver);
    DeviceManager mgr({&thruster_dev});

    char arg0[] = "thr";
    char arg1[] = "setsp";
    char arg2[] = "thruster1";
    char arg3[] = "0";
    char *argv[] = {arg0, arg1, arg2, arg3, nullptr};

    g_is_running = true;
    int rc = ThrusterCommands(4, argv);

    EXPECT_EQ(0, rc);
    EXPECT_EQ(0, driver.last_speed);
    EXPECT_TRUE(OutputAsString(cout_stream).find("Thruster set speed: 0") !=
                std::string::npos);
    EXPECT_TRUE(g_is_running);
}