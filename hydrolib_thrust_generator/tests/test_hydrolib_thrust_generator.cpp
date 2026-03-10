#include <gtest/gtest.h>

#include "hydrolib_thrust_generator.hpp"

constexpr int THRUST_COUNT = 6;
constexpr int DEGREES_OF_FREEDOM_COUNT = 6;
constexpr int THRUST_LIMIT = 5;

using hydrolib::controlling::ThrustGenerator;

class ThrustGeneratorParamTest
    : public ::testing::TestWithParam<hydrolib::controlling::Control> {};

TEST_P(ThrustGeneratorParamTest, GenerationThrust) {
  constexpr ThrustGenerator<THRUST_COUNT>::ThrusterParamsArray
      thrust_to_x_rotation = {+0.0983, +0.0000, +0.0983,
                              -0.0983, +0.0000, -0.0983};
  constexpr ThrustGenerator<THRUST_COUNT>::ThrusterParamsArray
      thrust_to_y_rotation = {-0.1806, -0.0520, +0.1806,
                              +0.1806, -0.0520, -0.1806};
  constexpr ThrustGenerator<THRUST_COUNT>::ThrusterParamsArray
      thrust_to_z_rotation = {-0.1265, -0.1230, -0.1265,
                              +0.1265, +0.1230, +0.1265};
  constexpr ThrustGenerator<THRUST_COUNT>::ThrusterParamsArray
      thrust_to_x_linearss = {+0.0000, +1.0000, +0.0000,
                              +0.0000, +1.0000, +0.0000};
  constexpr ThrustGenerator<THRUST_COUNT>::ThrusterParamsArray
      thrust_to_y_linearss = {-0.5736, -0.0000, +0.5736,
                              -0.5736, +0.0000, +0.5736};
  constexpr ThrustGenerator<THRUST_COUNT>::ThrusterParamsArray
      thrust_to_z_linearss = {+0.8192, +0.0000, +0.8192,
                              +0.8192, +0.0000, +0.8192};

  auto control = GetParam();

  constexpr hydrolib::controlling::ThrustGenerator<THRUST_COUNT> generator(
      thrust_to_x_rotation, thrust_to_y_rotation, thrust_to_z_rotation,
      thrust_to_x_linearss, thrust_to_y_linearss, thrust_to_z_linearss,
      THRUST_LIMIT);

  std::array<hydrolib::math::FixedPointBase, THRUST_COUNT> thrusts{};

  generator.ProcessWithFeedback(control, thrusts);

  double expected_x_rotation = 0;
  for (int j = 0; j < THRUST_COUNT; j++) {
    expected_x_rotation +=
        thrust_to_x_rotation[j] * static_cast<double>(thrusts[j]);
  }
  double expected_y_rotation = 0;
  for (int j = 0; j < THRUST_COUNT; j++) {
    expected_y_rotation +=
        thrust_to_y_rotation[j] * static_cast<double>(thrusts[j]);
  }
  double expected_z_rotation = 0;
  for (int j = 0; j < THRUST_COUNT; j++) {
    expected_z_rotation +=
        thrust_to_z_rotation[j] * static_cast<double>(thrusts[j]);
  }
  double expected_x_linear = 0;
  for (int j = 0; j < THRUST_COUNT; j++) {
    expected_x_linear +=
        thrust_to_x_linearss[j] * static_cast<double>(thrusts[j]);
  }
  double expected_y_linear = 0;
  for (int j = 0; j < THRUST_COUNT; j++) {
    expected_y_linear +=
        thrust_to_y_linearss[j] * static_cast<double>(thrusts[j]);
  }
  double expected_z_linear = 0;
  for (int j = 0; j < THRUST_COUNT; j++) {
    expected_z_linear +=
        thrust_to_z_linearss[j] * static_cast<double>(thrusts[j]);
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
    ::testing::Values(hydrolib::controlling::Control{.x_force = 10.000,
                                                     .y_force = 0.000,
                                                     .z_force = 0.000,
                                                     .x_torque = 0.000,
                                                     .y_torque = 0.000,
                                                     .z_torque = 0.000},
                      hydrolib::controlling::Control{.x_force = 0.000,
                                                     .y_force = 10.000,
                                                     .z_force = 0.000,
                                                     .x_torque = 0.000,
                                                     .y_torque = 0.000,
                                                     .z_torque = 0.000},
                      hydrolib::controlling::Control{.x_force = 0.000,
                                                     .y_force = 0.000,
                                                     .z_force = 10.000,
                                                     .x_torque = 0.000,
                                                     .y_torque = 0.000,
                                                     .z_torque = 0.000},
                      hydrolib::controlling::Control{.x_force = 0.000,
                                                     .y_force = 0.000,
                                                     .z_force = 0.000,
                                                     .x_torque = 10.000,
                                                     .y_torque = 0.000,
                                                     .z_torque = 0.000},
                      hydrolib::controlling::Control{.x_force = 0.000,
                                                     .y_force = 0.000,
                                                     .z_force = 0.000,
                                                     .x_torque = 0.000,
                                                     .y_torque = 10.000,
                                                     .z_torque = 0.000},
                      hydrolib::controlling::Control{.x_force = 0.000,
                                                     .y_force = 0.000,
                                                     .z_force = 0.000,
                                                     .x_torque = 0.000,
                                                     .y_torque = 0.000,
                                                     .z_torque = 10.000}));
