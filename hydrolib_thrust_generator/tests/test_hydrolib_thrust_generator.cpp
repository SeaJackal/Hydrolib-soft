#include <gtest/gtest.h>

#include "hydrolib_thrust_generator.hpp"

constexpr int THRUST_COUNT = 6;
constexpr int DEGREES_OF_FREEDOM_COUNT = 6;
constexpr int THRUST_LIMIT = 5;

using hydrolib::controlling::ThrustGenerator;

class ThrustGeneratorParamTest
    : public ::testing::TestWithParam<hydrolib::controlling::Control> {};

TEST_P(ThrustGeneratorParamTest, GenerationThrust) {
  constexpr ThrustGenerator<THRUST_COUNT>::ThrustArray thrust_to_x_rotation = {
      +0.0983, +0.0000, +0.0983, -0.0983, +0.0000, -0.0983};
  constexpr ThrustGenerator<THRUST_COUNT>::ThrustArray thrust_to_y_rotation = {
      -0.1806, -0.0520, +0.1806, +0.1806, -0.0520, -0.1806};
  constexpr ThrustGenerator<THRUST_COUNT>::ThrustArray thrust_to_z_rotation = {
      -0.1265, -0.1230, -0.1265, +0.1265, +0.1230, +0.1265};
  constexpr ThrustGenerator<THRUST_COUNT>::ThrustArray thrust_to_x_linearss = {
      +0.0000, +1.0000, +0.0000, +0.0000, +1.0000, +0.0000};
  constexpr ThrustGenerator<THRUST_COUNT>::ThrustArray thrust_to_y_linearss = {
      -0.5736, -0.0000, +0.5736, -0.5736, +0.0000, +0.5736};
  constexpr ThrustGenerator<THRUST_COUNT>::ThrustArray thrust_to_z_linearss = {
      +0.8192, +0.0000, +0.8192, +0.8192, +0.0000, +0.8192};

  auto control = GetParam();

  hydrolib::controlling::ThrustGenerator<THRUST_COUNT> generator(
      thrust_to_x_rotation, thrust_to_y_rotation, thrust_to_z_rotation,
      thrust_to_x_linearss, thrust_to_y_linearss, thrust_to_z_linearss,
      THRUST_LIMIT);

  std::array<int, THRUST_COUNT> thrusts{};

  generator.ProcessWithFeedback(control, thrusts);

  for (int i = 0; i < THRUST_COUNT; i++) {
    auto expected =
        static_cast<int>(thrust_to_x_rotation[i] * control.x_torque +
                         thrust_to_y_rotation[i] * control.y_torque +
                         thrust_to_z_rotation[i] * control.z_torque +
                         thrust_to_x_linearss[i] * control.x_force +
                         thrust_to_y_linearss[i] * control.y_force +
                         thrust_to_z_linearss[i] * control.z_force);

    EXPECT_EQ(thrusts[i], expected);
  }
}

INSTANTIATE_TEST_SUITE_P(
    ControlCases, ThrustGeneratorParamTest,
    ::testing::Values(
        hydrolib::controlling::Control{.x_force = 1.000,
                                       .y_force = 0.000,
                                       .z_force = 0.000,
                                       .x_torque = 0.000,
                                       .y_torque = 0.000,
                                       .z_torque = 0.000},
        hydrolib::controlling::Control{.x_force = 0.000,
                                       .y_force = 1.000,
                                       .z_force = 0.000,
                                       .x_torque = 0.000,
                                       .y_torque = 0.000,
                                       .z_torque = 0.000},
        hydrolib::controlling::Control{.x_force = 0.000,
                                       .y_force = 0.000,
                                       .z_force = 1.000,
                                       .x_torque = 0.000,
                                       .y_torque = 0.000,
                                       .z_torque = 0.000},
        hydrolib::controlling::Control{.x_force = 0.000,
                                       .y_force = 0.000,
                                       .z_force = 0.000,
                                       .x_torque = 1.000,
                                       .y_torque = 0.000,
                                       .z_torque = 0.000},
        hydrolib::controlling::Control{.x_force = 0.000,
                                       .y_force = 0.000,
                                       .z_force = 0.000,
                                       .x_torque = 0.000,
                                       .y_torque = 1.000,
                                       .z_torque = 0.000},
        hydrolib::controlling::Control{.x_force = 0.000,
                                       .y_force = 0.000,
                                       .z_force = 0.000,
                                       .x_torque = 0.000,
                                       .y_torque = 0.000,
                                       .z_torque = 1.000},
        hydrolib::controlling::Control{.x_force = 0.500,
                                       .y_force = -0.250,
                                       .z_force = 0.750,
                                       .x_torque = -0.300,
                                       .y_torque = 0.200,
                                       .z_torque = -0.100}));
