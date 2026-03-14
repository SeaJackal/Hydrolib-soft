#include <gtest/gtest.h>

#include <utility>

#include "hydrolib_thrust_generator.hpp"

constexpr int THRUST_COUNT = 6;
constexpr int THRUST_LIMIT = 1000;

using hydrolib::controlling::ThrustGenerator;

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

using Generator = ThrustGenerator<TestThruster, THRUST_COUNT>;
using ThrusterRefsArray = std::array<TestThruster, THRUST_COUNT>;

class ThrustGeneratorParamTest
    : public ::testing::TestWithParam<hydrolib::controlling::Control> {};

constinit std::array<TestThruster, THRUST_COUNT> thrusters_storage = {};

constexpr ThrustGenerator<TestThruster, THRUST_COUNT>::ThrusterParamsArray
    thrust_to_x_rotation = {+0.0983, +0.0000, +0.0983,
                            -0.0983, +0.0000, -0.0983};
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
                              thrusters_storage, THRUST_LIMIT);

TEST_P(ThrustGeneratorParamTest, GenerationThrust) {
  auto control = GetParam();

  generator.ProcessWithFeedback(control);

  double expected_x_rotation = 0;
  for (int j = 0; j < THRUST_COUNT; j++) {
    expected_x_rotation += thrust_to_x_rotation[j] *
                           static_cast<double>(thrusters_storage[j].GetSpeed());
  }
  double expected_y_rotation = 0;
  for (int j = 0; j < THRUST_COUNT; j++) {
    expected_y_rotation += thrust_to_y_rotation[j] *
                           static_cast<double>(thrusters_storage[j].GetSpeed());
  }
  double expected_z_rotation = 0;
  for (int j = 0; j < THRUST_COUNT; j++) {
    expected_z_rotation += thrust_to_z_rotation[j] *
                           static_cast<double>(thrusters_storage[j].GetSpeed());
  }
  double expected_x_linear = 0;
  for (int j = 0; j < THRUST_COUNT; j++) {
    expected_x_linear += thrust_to_x_linearss[j] *
                         static_cast<double>(thrusters_storage[j].GetSpeed());
  }
  double expected_y_linear = 0;
  for (int j = 0; j < THRUST_COUNT; j++) {
    expected_y_linear += thrust_to_y_linearss[j] *
                         static_cast<double>(thrusters_storage[j].GetSpeed());
  }
  double expected_z_linear = 0;
  for (int j = 0; j < THRUST_COUNT; j++) {
    expected_z_linear += thrust_to_z_linearss[j] *
                         static_cast<double>(thrusters_storage[j].GetSpeed());
  }
  EXPECT_NEAR(static_cast<double>(control.x_torque),
              static_cast<double>(expected_x_rotation), 1);
  EXPECT_NEAR(static_cast<double>(control.y_torque),
              static_cast<double>(expected_y_rotation), 1);
  EXPECT_NEAR(static_cast<double>(control.z_torque),
              static_cast<double>(expected_z_rotation), 1);
  EXPECT_NEAR(static_cast<double>(control.x_force),
              static_cast<double>(expected_x_linear), 1);
  EXPECT_NEAR(static_cast<double>(control.y_force),
              static_cast<double>(expected_y_linear), 1);
  EXPECT_NEAR(static_cast<double>(control.z_force),
              static_cast<double>(expected_z_linear), 1);
}

INSTANTIATE_TEST_SUITE_P(
    ControlCases, ThrustGeneratorParamTest,
    ::testing::Values(hydrolib::controlling::Control{.x_force = 10,
                                                     .y_force = 0,
                                                     .z_force = 0,
                                                     .x_torque = 0,
                                                     .y_torque = 0,
                                                     .z_torque = 0},
                      hydrolib::controlling::Control{.x_force = 0,
                                                     .y_force = 10,
                                                     .z_force = 0,
                                                     .x_torque = 0,
                                                     .y_torque = 0,
                                                     .z_torque = 0},
                      hydrolib::controlling::Control{.x_force = 0,
                                                     .y_force = 0,
                                                     .z_force = 10,
                                                     .x_torque = 0,
                                                     .y_torque = 0,
                                                     .z_torque = 0},
                      hydrolib::controlling::Control{.x_force = 0,
                                                     .y_force = 0,
                                                     .z_force = 0,
                                                     .x_torque = 10,
                                                     .y_torque = 0,
                                                     .z_torque = 0},
                      hydrolib::controlling::Control{.x_force = 0,
                                                     .y_force = 0,
                                                     .z_force = 0,
                                                     .x_torque = 0,
                                                     .y_torque = 10,
                                                     .z_torque = 0},
                      hydrolib::controlling::Control{.x_force = 0,
                                                     .y_force = 0,
                                                     .z_force = 0,
                                                     .x_torque = 0,
                                                     .y_torque = 0,
                                                     .z_torque = 10}));
