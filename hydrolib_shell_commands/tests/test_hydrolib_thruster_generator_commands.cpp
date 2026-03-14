#include <gtest/gtest.h>
#include <unistd.h>

#include <array>
#include <deque>
#include <string>
#include <string_view>
#include <vector>

#include "hydrolib_thruster_generator_commands.hpp"

using hydrolib::device::DeviceManager;
using hydrolib::device::IControlSystem;
using hydrolib::device::ThrustGeneratorDevice;
using hydrolib::shell::cout;
using hydrolib::shell::g_is_running;
using hydrolib::shell::Ostream;
using hydrolib::shell::StreamWrapper;
using hydrolib::shell::ThrusterGeneratorCommands;

constexpr int THRUST_LIMIT = 1000;

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

// Test thruster for the generator
class TestThruster {
 public:
  hydrolib::ReturnCode SetSpeed(int speed) {
    speed_ = speed;
    return hydrolib::ReturnCode::OK;
  }

  int GetSpeed() const { return speed_; }

 private:
  int speed_ = 0;
};

using Generator =
    hydrolib::controlling::ThrustGenerator<TestThruster, THRUST_COUNT>;
using ThrusterRefsArray = std::array<TestThruster, THRUST_COUNT>;

class ThrustGeneratorParamTest
    : public ::testing::TestWithParam<hydrolib::controlling::Control> {};

constinit std::array<TestThruster, THRUST_COUNT> thrusters_storage = {};
constexpr std::array<TestThruster *, THRUST_COUNT> thruster_pointers = {
    &thrusters_storage[0], &thrusters_storage[1], &thrusters_storage[2],
    &thrusters_storage[3], &thrusters_storage[4], &thrusters_storage[5]};

constexpr hydrolib::controlling::ThrustGenerator<
    TestThruster, THRUST_COUNT>::ThrusterParamsArray thrust_to_x_rotation = {
    +0.0983, +0.0000, +0.0983, -0.0983, +0.0000, -0.0983};
constexpr Generator::ThrusterParamsArray thrust_to_y_rotation = {
    -0.1806, -0.0520, +0.1806, +0.1806, -0.0520, -0.1806};
constexpr Generator::ThrusterParamsArray thrust_to_z_rotation = {
    -0.1265, -0.1230, -0.1265, +0.1265, +0.1230, +0.1265};
constexpr Generator::ThrusterParamsArray thrust_to_x_linearss = {
    +0.0000, +1.0000, +0.0000, +0.0000, +1.0000, +0.0000};
constexpr Generator::ThrusterParamsArray thrust_to_y_linearss = {
    -0.5736, -0.0000, +0.5736, -0.5736, +0.0000, +0.5736};
constexpr Generator::ThrusterParamsArray thrust_to_z_linearss = {
    +0.8192, +0.0000, +0.8192, +0.8192, +0.0000, +0.8192};

constinit Generator generator(thrust_to_x_rotation, thrust_to_y_rotation,
                              thrust_to_z_rotation, thrust_to_x_linearss,
                              thrust_to_y_linearss, thrust_to_z_linearss,
                              thruster_pointers, THRUST_LIMIT);
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

// Basic shell tests
TEST(HydrolibThrusterGeneratorShell, PrintsUsageOnHelpAndStops) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  DeviceManager mgr({});

  char arg0[] = "ctrl";
  char arg1[] = "-h";
  char *argv[] = {arg0, arg1, nullptr};

  g_is_running = true;
  int rc = ThrusterGeneratorCommands(2, argv);

  EXPECT_EQ(0, rc);
  std::string output = OutputAsString(cout_stream);
  EXPECT_TRUE(output.find("Usage: ctrl") != std::string::npos);
  EXPECT_FALSE(g_is_running);
}

TEST(HydrolibThrusterGeneratorShell, InvalidOptionSetsError) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  DeviceManager mgr({});

  char arg0[] = "ctrl";
  char arg1[] = "-a";
  char *argv[] = {arg0, arg1, nullptr};

  g_is_running = true;
  int rc = ThrusterGeneratorCommands(2, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("Invalid option: a", OutputAsString(cout_stream));
  EXPECT_FALSE(g_is_running);
}

TEST(HydrolibThrusterGeneratorShell, DeviceNotFoundSetsError) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  DeviceManager mgr({});

  char arg0[] = "ctrl";
  char arg1[] = "no_such_device";
  char *argv[] = {arg0, arg1, nullptr};

  g_is_running = true;
  int rc = ThrusterGeneratorCommands(2, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("Device not found: no_such_device", OutputAsString(cout_stream));
  EXPECT_FALSE(g_is_running);
}

TEST(HydrolibThrusterGeneratorShell, DeviceIsNotControlSystemSetsError) {
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
  int rc = ThrusterGeneratorCommands(2, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("Device is not a thruster generator: not_control",
            OutputAsString(cout_stream));
  EXPECT_FALSE(g_is_running);
}

TEST(HydrolibThrusterGeneratorShell, NoDeviceSpecified) {
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
  int rc = ThrusterGeneratorCommands(3, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("No thruster generator device specified",
            OutputAsString(cout_stream));
  EXPECT_FALSE(g_is_running);
}

// Invalid value tests
TEST(HydrolibThrusterGeneratorShell, InvalidForceValue) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  ThrustGeneratorDevice<TestThruster, THRUST_COUNT> generator_dev("gen1",
                                                                  generator);
  DeviceManager mgr({&generator_dev});

  char arg0[] = "ctrl";
  char arg1[] = "gen1";
  char arg2[] = "-x";
  char arg3[] = "abc";
  char *argv[] = {arg0, arg1, arg2, arg3, nullptr};

  g_is_running = true;
  int rc = ThrusterGeneratorCommands(4, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("Invalid force value: abc", OutputAsString(cout_stream));
  EXPECT_FALSE(g_is_running);
}

TEST(HydrolibThrusterGeneratorShell, InvalidTorqueValue) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  ThrustGeneratorDevice<TestThruster, THRUST_COUNT> generator_dev("gen1",
                                                                  generator);
  DeviceManager mgr({&generator_dev});

  char arg0[] = "ctrl";
  char arg1[] = "gen1";
  char arg2[] = "-X";
  char arg3[] = "xyz";
  char *argv[] = {arg0, arg1, arg2, arg3, nullptr};

  g_is_running = true;
  int rc = ThrusterGeneratorCommands(4, argv);

  EXPECT_EQ(-1, rc);
  EXPECT_EQ("Invalid torque value: xyz", OutputAsString(cout_stream));
  EXPECT_FALSE(g_is_running);
}

// Stop command test
TEST(HydrolibThrusterGeneratorShell, StopCommand) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  ThrustGeneratorDevice<TestThruster, THRUST_COUNT> generator_dev("gen1",
                                                                  generator);
  DeviceManager mgr({&generator_dev});

  // First set some values
  char arg0[] = "ctrl";
  char arg1[] = "gen1";
  char arg2[] = "-x";
  char arg3[] = "10";
  char arg4[] = "-X";
  char arg5[] = "5";
  char *argv1[] = {arg0, arg1, arg2, arg3, arg4, arg5, nullptr};

  g_is_running = true;
  int rc = ThrusterGeneratorCommands(6, argv1);
  EXPECT_EQ(0, rc);

  // Clear the output stream
  cout_stream.output_.clear();

  // Then send stop command
  char arg6[] = "stop";
  char *argv2[] = {arg0, arg1, arg6, nullptr};

  g_is_running = true;
  rc = ThrusterGeneratorCommands(3, argv2);

  EXPECT_EQ(0, rc);

  // Verify thrusters are stopped (all values should be 0)
  for (int i = 0; i < THRUST_COUNT; ++i) {
    EXPECT_EQ(0, thrusters_storage[i].GetSpeed());
  }
}

// Parameterized test for control inputs
class ThrusterGeneratorShellParamTest
    : public ::testing::TestWithParam<hydrolib::controlling::Control> {
 protected:
  void SetUp() override {
    ResetGetopt();
    // Reset all thrusters to 0 before each test
    for (auto &thruster : thrusters_storage) {
      thruster.SetSpeed(0);
    }
  }
};

TEST_P(ThrusterGeneratorShellParamTest, ControlCommand) {
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  ThrustGeneratorDevice<TestThruster, THRUST_COUNT> generator_dev("gen1",
                                                                  generator);
  DeviceManager mgr({&generator_dev});

  auto control = GetParam();

  // Build command line arguments based on control values
  std::vector<std::string> args;
  args.push_back("ctrl");
  args.push_back("gen1");

  // Используем static_cast<int> для преобразования FixedPoint в int
  if (static_cast<int>(control.x_force) != 0) {
    args.push_back("-x");
    args.push_back(std::to_string(static_cast<int>(control.x_force)));
  }
  if (static_cast<int>(control.y_force) != 0) {
    args.push_back("-y");
    args.push_back(std::to_string(static_cast<int>(control.y_force)));
  }
  if (static_cast<int>(control.z_force) != 0) {
    args.push_back("-z");
    args.push_back(std::to_string(static_cast<int>(control.z_force)));
  }
  if (static_cast<int>(control.x_torque) != 0) {
    args.push_back("-X");
    args.push_back(std::to_string(static_cast<int>(control.x_torque)));
  }
  if (static_cast<int>(control.y_torque) != 0) {
    args.push_back("-Y");
    args.push_back(std::to_string(static_cast<int>(control.y_torque)));
  }
  if (static_cast<int>(control.z_torque) != 0) {
    args.push_back("-Z");
    args.push_back(std::to_string(static_cast<int>(control.z_torque)));
  }

  // Convert to argv format
  std::vector<char *> argv;
  for (auto &arg : args) {
    argv.push_back(const_cast<char *>(arg.c_str()));
  }
  argv.push_back(nullptr);

  g_is_running = true;
  int rc =
      ThrusterGeneratorCommands(static_cast<int>(argv.size() - 1), argv.data());

  EXPECT_EQ(0, rc);

  // Verify the resulting thrusts produce the expected control values
  double expected_x_rotation = 0;
  double expected_y_rotation = 0;
  double expected_z_rotation = 0;
  double expected_x_linear = 0;
  double expected_y_linear = 0;
  double expected_z_linear = 0;

  for (int j = 0; j < THRUST_COUNT; j++) {
    double speed = static_cast<double>(thrusters_storage[j].GetSpeed());
    expected_x_rotation += thrust_to_x_rotation[j] * speed;
    expected_y_rotation += thrust_to_y_rotation[j] * speed;
    expected_z_rotation += thrust_to_z_rotation[j] * speed;
    expected_x_linear += thrust_to_x_linearss[j] * speed;
    expected_y_linear += thrust_to_y_linearss[j] * speed;
    expected_z_linear += thrust_to_z_linearss[j] * speed;
  }

  EXPECT_NEAR(static_cast<double>(static_cast<int>(control.x_torque)),
              expected_x_rotation, 1);
  EXPECT_NEAR(static_cast<double>(static_cast<int>(control.y_torque)),
              expected_y_rotation, 1);
  EXPECT_NEAR(static_cast<double>(static_cast<int>(control.z_torque)),
              expected_z_rotation, 1);
  EXPECT_NEAR(static_cast<double>(static_cast<int>(control.x_force)),
              expected_x_linear, 1);
  EXPECT_NEAR(static_cast<double>(static_cast<int>(control.y_force)),
              expected_y_linear, 1);
  EXPECT_NEAR(static_cast<double>(static_cast<int>(control.z_force)),
              expected_z_linear, 1);
}

INSTANTIATE_TEST_SUITE_P(
    ControlCases, ThrusterGeneratorShellParamTest,
    ::testing::Values(
        hydrolib::controlling::Control{
            .x_force = hydrolib::math::FixedPointBase(10),
            .y_force = hydrolib::math::FixedPointBase(0),
            .z_force = hydrolib::math::FixedPointBase(0),
            .x_torque = hydrolib::math::FixedPointBase(0),
            .y_torque = hydrolib::math::FixedPointBase(0),
            .z_torque = hydrolib::math::FixedPointBase(0)},
        hydrolib::controlling::Control{
            .x_force = hydrolib::math::FixedPointBase(0),
            .y_force = hydrolib::math::FixedPointBase(10),
            .z_force = hydrolib::math::FixedPointBase(0),
            .x_torque = hydrolib::math::FixedPointBase(0),
            .y_torque = hydrolib::math::FixedPointBase(0),
            .z_torque = hydrolib::math::FixedPointBase(0)},
        hydrolib::controlling::Control{
            .x_force = hydrolib::math::FixedPointBase(0),
            .y_force = hydrolib::math::FixedPointBase(0),
            .z_force = hydrolib::math::FixedPointBase(10),
            .x_torque = hydrolib::math::FixedPointBase(0),
            .y_torque = hydrolib::math::FixedPointBase(0),
            .z_torque = hydrolib::math::FixedPointBase(0)},
        hydrolib::controlling::Control{
            .x_force = hydrolib::math::FixedPointBase(0),
            .y_force = hydrolib::math::FixedPointBase(0),
            .z_force = hydrolib::math::FixedPointBase(0),
            .x_torque = hydrolib::math::FixedPointBase(10),
            .y_torque = hydrolib::math::FixedPointBase(0),
            .z_torque = hydrolib::math::FixedPointBase(0)},
        hydrolib::controlling::Control{
            .x_force = hydrolib::math::FixedPointBase(0),
            .y_force = hydrolib::math::FixedPointBase(0),
            .z_force = hydrolib::math::FixedPointBase(0),
            .x_torque = hydrolib::math::FixedPointBase(0),
            .y_torque = hydrolib::math::FixedPointBase(10),
            .z_torque = hydrolib::math::FixedPointBase(0)},
        hydrolib::controlling::Control{
            .x_force = hydrolib::math::FixedPointBase(0),
            .y_force = hydrolib::math::FixedPointBase(0),
            .z_force = hydrolib::math::FixedPointBase(0),
            .x_torque = hydrolib::math::FixedPointBase(0),
            .y_torque = hydrolib::math::FixedPointBase(0),
            .z_torque = hydrolib::math::FixedPointBase(10)}));

// Combined control test
TEST(HydrolibThrusterGeneratorShell, CombinedControl) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  ThrustGeneratorDevice<TestThruster, THRUST_COUNT> generator_dev("gen1",
                                                                  generator);
  DeviceManager mgr({&generator_dev});

  // Reset thrusters
  for (auto &thruster : thrusters_storage) {
    thruster.SetSpeed(0);
  }

  char arg0[] = "ctrl";
  char arg1[] = "gen1";
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
  int rc = ThrusterGeneratorCommands(14, argv);

  EXPECT_EQ(0, rc);

  // Verify all thrusters got some non-zero values (combined control should
  // activate all)
  bool all_zero = true;
  for (int i = 0; i < THRUST_COUNT; ++i) {
    if (thrusters_storage[i].GetSpeed() != 0) {
      all_zero = false;
      break;
    }
  }
  EXPECT_FALSE(all_zero);
}

TEST(HydrolibThrusterGeneratorShell, OptionsOrderDoesNotMatter) {
  ResetGetopt();
  FakeStream cout_stream;
  StreamWrapper<FakeStream> cout_wrapper(cout_stream);
  cout = Ostream(cout_wrapper);

  ThrustGeneratorDevice<TestThruster, THRUST_COUNT> generator_dev("gen1",
                                                                  generator);
  DeviceManager mgr({&generator_dev});

  // Reset thrusters
  for (auto &thruster : thrusters_storage) {
    thruster.SetSpeed(0);
  }

  char arg0[] = "ctrl";
  char arg1[] = "-X";
  char arg2[] = "5";
  char arg3[] = "gen1";  // device in the middle
  char arg4[] = "-x";
  char arg5[] = "10";
  char *argv[] = {arg0, arg1, arg2, arg3, arg4, arg5, nullptr};

  g_is_running = true;
  int rc = ThrusterGeneratorCommands(6, argv);

  EXPECT_EQ(0, rc);

  // Calculate expected results
  double expected_x_rotation = 0;
  double expected_x_linear = 0;

  for (int j = 0; j < THRUST_COUNT; j++) {
    double speed = static_cast<double>(thrusters_storage[j].GetSpeed());
    expected_x_rotation += thrust_to_x_rotation[j] * speed;
    expected_x_linear += thrust_to_x_linearss[j] * speed;
  }

  EXPECT_NEAR(10.0, expected_x_linear, 1);
  EXPECT_NEAR(5.0, expected_x_rotation, 1);
}