#include <gtest/gtest.h>

#include <chrono>
#include <cmath>
#include <numbers>
#include <thread>

#include "hydrolib_motor_controller.hpp"

using namespace hydrolib;

class TestSensor {
 public:
  void SetAngle(math::FixedPointBase angle) { angle_ = angle; }
  math::FixedPointBase GetPos() { return angle_; }

 private:
  math::FixedPointBase angle_ = 0;
};

class TestMotor {
 public:
  void SetAPhaseVoltage(math::FixedPointBase voltage) { phase_a_ = voltage; }
  void SetBPhaseVoltage(math::FixedPointBase voltage) { phase_b_ = voltage; }
  void SetCPhaseVoltage(math::FixedPointBase voltage) { phase_c_ = voltage; }

  math::FixedPointBase GetAPhaseVoltage() const { return phase_a_; }
  math::FixedPointBase GetBPhaseVoltage() const { return phase_b_; }
  math::FixedPointBase GetCPhaseVoltage() const { return phase_c_; }

 private:
  math::FixedPointBase phase_a_ = 0;
  math::FixedPointBase phase_b_ = 0;
  math::FixedPointBase phase_c_ = 0;
};

TEST(TestMotorController, ResultantCurrentPerpendicularToAngle) {
  TestSensor sensor;
  TestMotor motor;
  motor::MotorController<TestSensor, TestMotor> controller(sensor, motor);

  constexpr double angle_rad = 0.7;
  constexpr double speed_value = 1.0;

  math::FixedPointBase angle = angle_rad;
  sensor.SetAngle(angle);
  controller.SetSpeed(speed_value);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  controller.Process();

  double phase_a = static_cast<double>(motor.GetAPhaseVoltage());
  double phase_b = static_cast<double>(motor.GetBPhaseVoltage());
  double phase_c = static_cast<double>(motor.GetCPhaseVoltage());

  double result_x = (phase_a * 0.5) - phase_b + (phase_c * 0.5);
  double result_y = (phase_a * std::numbers::sqrt3 / 2) +
                    (phase_c * -std::numbers::sqrt3 / 2);

  double theta = static_cast<double>(angle);
  double target_x = std::cos(theta);
  double target_y = std::sin(theta);

  double dot = (result_x * target_x) + (result_y * target_y);

  EXPECT_NEAR(dot, 0, 1e-3);
}
