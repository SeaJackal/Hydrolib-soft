#include <gtest/gtest.h>
#include <unistd.h>

#include <array>
#include <deque>
#include <string>
#include <string_view>
#include <vector>

#include "hydrolib_control_system_commands.hpp"
#include "hydrolib_control_system_device.hpp"
#include "hydrolib_device.hpp"

using hydrolib::controlling::Control;
using hydrolib::device::Device;
using hydrolib::device::DeviceManager;
using hydrolib::device::IControlSystem;
using hydrolib::shell::ControlSystemCommands;
using hydrolib::shell::cout;
using hydrolib::shell::g_is_running;
using hydrolib::shell::Ostream;
using hydrolib::shell::StreamWrapper;

namespace {

// Simple stream with byte-wise read/write free functions
struct FakeStream {
  FakeStream() = default;

  explicit FakeStream(std::string_view initial_data) {
    input_.insert(input_.end(), initial_data.begin(), initial_data.end());
  }

  std::deque<char> input_;
  std::vector<char> output_;

  bool fail_next_write_ = false;
};

int read(FakeStream &stream, void *dest, unsigned length) {
  if (length == 0 || stream.input_.empty()) {
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

int write(FakeStream &stream, const void *source, unsigned length) {
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

class MockControlSystem : public IControlSystem {
 public:
  const hydrolib::device::DeviceType kSelfType =
      hydrolib::device::DeviceType::CONTROLSYSTEM;

 public:
  MockControlSystem(std::string_view name) : IControlSystem(name) {}

 public:
  Control control_;
  void ControlProcess(Control &control) override {
    control_.x_force = control.x_force;
    control_.y_force = control.y_force;
    control_.z_force = control.z_force;
    control_.x_torque = control.x_torque;
    control_.y_torque = control.y_torque;
    control_.z_torque = control.z_torque;
  };
};

class NonControlSystemDevice : public hydrolib::device::Device {
 public:
  NonControlSystemDevice()
      : Device("not_control", hydrolib::device::DeviceType::STREAM) {}
};

void ResetGetopt() {
  optind = 1;
  opterr = 0;
}
}  // namespace

TEST(HydrolibControlSystem, PrintsUsageOnHelpAndStops) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  DeviceManager mgr({});

  char arg0[] = "ctrl";
  char arg1[] = "-h";
  char *argv[] = {arg0, arg1, nullptr};

  g_is_running = true;
  int rc = ControlSystemCommands(2, argv);

  EXPECT_EQ(0, rc);
  EXPECT_TRUE(OutputAsString(cout_stream).find("Usage: ctrl") !=
              std::string::npos);
  EXPECT_FALSE(g_is_running);
}

TEST(HydrolibControlSystem, InvalidOptionSetsError) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  DeviceManager mgr({});

  char arg0[] = "ctrl";
  char arg1[] = "-a";
  char *argv[] = {arg0, arg1, nullptr};

  g_is_running = true;
  int rc = ControlSystemCommands(2, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("Invalid option: a", OutputAsString(cout_stream));
  EXPECT_FALSE(g_is_running);
}

TEST(HydrolibControlSystem, DeviceNotFoundSetsError) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  DeviceManager mgr({});

  char arg0[] = "ctrl";
  char arg1[] = "no_such_device";
  char *argv[] = {arg0, arg1, nullptr};

  g_is_running = true;
  int rc = ControlSystemCommands(2, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("Device not found: no_such_device", OutputAsString(cout_stream));
  EXPECT_FALSE(g_is_running);
}

TEST(HydrolibControlSystem, DeviceIsNotControlSystemSetsError) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  NonControlSystemDevice non_control;
  DeviceManager mgr({&non_control});

  char arg0[] = "ctrl";
  char arg1[] = "not_control";
  char *argv[] = {arg0, arg1, nullptr};

  g_is_running = true;
  int rc = ControlSystemCommands(2, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("Device is not a system control: not_control",
            OutputAsString(cout_stream));
  EXPECT_FALSE(g_is_running);
}

TEST(HydrolibControlSystem, NoDeviceSpecified) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  DeviceManager mgr({});

  char arg0[] = "ctrl";
  char arg1[] = "-x";
  char arg2[] = "10";
  char *argv[] = {arg0, arg1, arg2, nullptr};

  g_is_running = true;
  int rc = ControlSystemCommands(3, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("No thruster generator device specified",
            OutputAsString(cout_stream));
  EXPECT_FALSE(g_is_running);
}

TEST(HydrolibControlSystem, InvalidForceValue) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  MockControlSystem mock_dev("ctrl1");
  DeviceManager mgr({&mock_dev});

  char arg0[] = "ctrl";
  char arg1[] = "ctrl1";
  char arg2[] = "-x";
  char arg3[] = "abc";
  char *argv[] = {arg0, arg1, arg2, arg3, nullptr};

  g_is_running = true;
  int rc = ControlSystemCommands(4, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("Invalid force value: abc", OutputAsString(cout_stream));
  EXPECT_FALSE(g_is_running);
}

TEST(HydrolibControlSystem, InvalidTorqueValue) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  MockControlSystem mock_dev("ctrl1");
  DeviceManager mgr({&mock_dev});

  char arg0[] = "ctrl";
  char arg1[] = "ctrl1";
  char arg2[] = "-X";
  char arg3[] = "xyz";
  char *argv[] = {arg0, arg1, arg2, arg3, nullptr};

  g_is_running = true;
  int rc = ControlSystemCommands(4, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("Invalid torque value: xyz", OutputAsString(cout_stream));
  EXPECT_FALSE(g_is_running);
}

TEST(HydrolibControlSystem, SetSingleForce) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  MockControlSystem mock_dev("ctrl1");
  DeviceManager mgr({&mock_dev});

  char arg0[] = "ctrl";
  char arg1[] = "ctrl1";
  char arg2[] = "-x";
  char arg3[] = "42";
  char *argv[] = {arg0, arg1, arg2, arg3, nullptr};

  g_is_running = true;
  int rc = ControlSystemCommands(4, argv);

  EXPECT_EQ(0, rc);

  EXPECT_EQ(42, static_cast<int>(mock_dev.control_.x_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.y_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.z_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.x_torque));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.y_torque));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.z_torque));
  EXPECT_TRUE(g_is_running);
}

TEST(HydrolibControlSystem, SetSingleForceY) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  MockControlSystem mock_dev("ctrl1");
  DeviceManager mgr({&mock_dev});

  char arg0[] = "ctrl";
  char arg1[] = "ctrl1";
  char arg2[] = "-y";
  char arg3[] = "123";
  char *argv[] = {arg0, arg1, arg2, arg3, nullptr};

  g_is_running = true;
  int rc = ControlSystemCommands(4, argv);

  EXPECT_EQ(0, rc);

  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.x_force));
  EXPECT_EQ(123, static_cast<int>(mock_dev.control_.y_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.z_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.x_torque));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.y_torque));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.z_torque));
  EXPECT_TRUE(g_is_running);
}

TEST(HydrolibControlSystem, SetSingleForceZ) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  MockControlSystem mock_dev("ctrl1");
  DeviceManager mgr({&mock_dev});

  char arg0[] = "ctrl";
  char arg1[] = "ctrl1";
  char arg2[] = "-z";
  char arg3[] = "77";
  char *argv[] = {arg0, arg1, arg2, arg3, nullptr};

  g_is_running = true;
  int rc = ControlSystemCommands(4, argv);

  EXPECT_EQ(0, rc);

  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.x_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.y_force));
  EXPECT_EQ(77, static_cast<int>(mock_dev.control_.z_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.x_torque));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.y_torque));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.z_torque));
  EXPECT_TRUE(g_is_running);
}

TEST(HydrolibControlSystem, SetSingleTorqueX) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  MockControlSystem mock_dev("ctrl1");
  DeviceManager mgr({&mock_dev});

  char arg0[] = "ctrl";
  char arg1[] = "ctrl1";
  char arg2[] = "-X";
  char arg3[] = "5";
  char *argv[] = {arg0, arg1, arg2, arg3, nullptr};

  g_is_running = true;
  int rc = ControlSystemCommands(4, argv);

  EXPECT_EQ(0, rc);

  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.x_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.y_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.z_force));
  EXPECT_EQ(5, static_cast<int>(mock_dev.control_.x_torque));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.y_torque));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.z_torque));
  EXPECT_TRUE(g_is_running);
}

TEST(HydrolibControlSystem, SetSingleTorqueY) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  MockControlSystem mock_dev("ctrl1");
  DeviceManager mgr({&mock_dev});

  char arg0[] = "ctrl";
  char arg1[] = "ctrl1";
  char arg2[] = "-Y";
  char arg3[] = "8";
  char *argv[] = {arg0, arg1, arg2, arg3, nullptr};

  g_is_running = true;
  int rc = ControlSystemCommands(4, argv);

  EXPECT_EQ(0, rc);

  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.x_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.y_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.z_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.x_torque));
  EXPECT_EQ(8, static_cast<int>(mock_dev.control_.y_torque));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.z_torque));
  EXPECT_TRUE(g_is_running);
}

TEST(HydrolibControlSystem, SetSingleTorqueZ) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  MockControlSystem mock_dev("ctrl1");
  DeviceManager mgr({&mock_dev});

  char arg0[] = "ctrl";
  char arg1[] = "ctrl1";
  char arg2[] = "-Z";
  char arg3[] = "3";
  char *argv[] = {arg0, arg1, arg2, arg3, nullptr};

  g_is_running = true;
  int rc = ControlSystemCommands(4, argv);

  EXPECT_EQ(0, rc);

  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.x_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.y_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.z_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.x_torque));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.y_torque));
  EXPECT_EQ(3, static_cast<int>(mock_dev.control_.z_torque));
  EXPECT_TRUE(g_is_running);
}

TEST(HydrolibControlSystem, SetAllParameters) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  MockControlSystem mock_dev("ctrl1");
  DeviceManager mgr({&mock_dev});

  char arg0[] = "ctrl";
  char arg1[] = "ctrl1";
  char arg2[] = "-x";
  char arg3[] = "10";
  char arg4[] = "-y";
  char arg5[] = "20";
  char arg6[] = "-z";
  char arg7[] = "30";
  char arg8[] = "-X";
  char arg9[] = "1";
  char arg10[] = "-Y";
  char arg11[] = "2";
  char arg12[] = "-Z";
  char arg13[] = "3";
  char *argv[] = {arg0, arg1, arg2,  arg3,  arg4,  arg5,  arg6,   arg7,
                  arg8, arg9, arg10, arg11, arg12, arg13, nullptr};

  g_is_running = true;
  int rc = ControlSystemCommands(14, argv);

  EXPECT_EQ(0, rc);

  EXPECT_EQ(10, static_cast<int>(mock_dev.control_.x_force));
  EXPECT_EQ(20, static_cast<int>(mock_dev.control_.y_force));
  EXPECT_EQ(30, static_cast<int>(mock_dev.control_.z_force));
  EXPECT_EQ(1, static_cast<int>(mock_dev.control_.x_torque));
  EXPECT_EQ(2, static_cast<int>(mock_dev.control_.y_torque));
  EXPECT_EQ(3, static_cast<int>(mock_dev.control_.z_torque));
  EXPECT_TRUE(g_is_running);
}

TEST(HydrolibControlSystem, NegativeValues) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  MockControlSystem mock_dev("ctrl1");
  DeviceManager mgr({&mock_dev});

  char arg0[] = "ctrl";
  char arg1[] = "ctrl1";
  char arg2[] = "-x";
  char arg3[] = "-15";
  char arg4[] = "-X";
  char arg5[] = "-7";
  char *argv[] = {arg0, arg1, arg2, arg3, arg4, arg5, nullptr};

  g_is_running = true;
  int rc = ControlSystemCommands(6, argv);

  EXPECT_EQ(0, rc);

  EXPECT_EQ(-15, static_cast<int>(mock_dev.control_.x_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.y_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.z_force));
  EXPECT_EQ(-7, static_cast<int>(mock_dev.control_.x_torque));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.y_torque));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.z_torque));
  EXPECT_TRUE(g_is_running);
}

TEST(HydrolibControlSystem, ZeroValues) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  MockControlSystem mock_dev("ctrl1");
  DeviceManager mgr({&mock_dev});

  char arg0[] = "ctrl";
  char arg1[] = "ctrl1";
  char arg2[] = "-x";
  char arg3[] = "0";
  char arg4[] = "-X";
  char arg5[] = "0";
  char *argv[] = {arg0, arg1, arg2, arg3, arg4, arg5, nullptr};

  g_is_running = true;
  int rc = ControlSystemCommands(6, argv);

  EXPECT_EQ(0, rc);

  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.x_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.x_torque));
  EXPECT_TRUE(g_is_running);
}

TEST(HydrolibControlSystem, OptionsOrderDoesNotMatter) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  MockControlSystem mock_dev("ctrl1");
  DeviceManager mgr({&mock_dev});

  char arg0[] = "ctrl";
  char arg1[] = "-X";
  char arg2[] = "5";
  char arg3[] = "ctrl1";
  char arg4[] = "-x";
  char arg5[] = "10";
  char *argv[] = {arg0, arg1, arg2, arg3, arg4, arg5, nullptr};

  g_is_running = true;
  int rc = ControlSystemCommands(6, argv);

  EXPECT_EQ(0, rc);

  EXPECT_EQ(10, static_cast<int>(mock_dev.control_.x_force));
  EXPECT_EQ(5, static_cast<int>(mock_dev.control_.x_torque));
  EXPECT_TRUE(g_is_running);
}

TEST(HydrolibControlSystem, StopCommand) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  MockControlSystem mock_dev("ctrl1");
  DeviceManager mgr({&mock_dev});

  char arg0[] = "ctrl";
  char arg1[] = "ctrl1";
  char arg2[] = "-x";
  char arg3[] = "10";
  char arg4[] = "-X";
  char arg5[] = "5";
  char *argv_set[] = {arg0, arg1, arg2, arg3, arg4, arg5, nullptr};

  g_is_running = true;
  int rc = ControlSystemCommands(6, argv_set);
  EXPECT_EQ(0, rc);

  EXPECT_EQ(10, static_cast<int>(mock_dev.control_.x_force));
  EXPECT_EQ(5, static_cast<int>(mock_dev.control_.x_torque));

  cout_stream.output_.clear();

  char arg_stop[] = "stop";
  char *argv_stop[] = {arg0, arg1, arg_stop, nullptr};

  g_is_running = true;
  rc = ControlSystemCommands(3, argv_stop);
  EXPECT_EQ(0, rc);

  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.x_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.y_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.z_force));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.x_torque));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.y_torque));
  EXPECT_EQ(0, static_cast<int>(mock_dev.control_.z_torque));
  EXPECT_TRUE(g_is_running);
}