#pragma once

#include <chrono>
#include <concepts>

#include "hydrolib_fixed_point.hpp"

namespace hydrolib::motor {
template <typename T>
concept AngleSensorConcept = requires(T sensor) {
  { sensor.GetPos() } -> std::convertible_to<math::FixedPointBase>;
};

template <typename T>
concept MotorConcept = requires(T motor, math::FixedPointBase voltage) {
  motor.SetAPhaseVoltage(voltage);
  motor.SetBPhaseVoltage(voltage);
  motor.SetCPhaseVoltage(voltage);
};

template <AngleSensorConcept Sensor, MotorConcept Motor>
class MotorController {
 public:
  using RadsPerSecond = math::FixedPointBase;

  MotorController(Sensor& sensor, Motor& motor);

  MotorController(MotorController&) = delete;
  MotorController(MotorController&&) = delete;
  MotorController operator=(MotorController&) = delete;
  MotorController operator=(MotorController&&) = delete;
  ~MotorController() = default;

  void Process();
  void SetSpeed(RadsPerSecond speed);

 private:
  static constexpr math::FixedPointBase kXtoA = 1.0 / 3.0;
  static constexpr math::FixedPointBase kYtoA = std::numbers::sqrt3 / 3.0;
  static constexpr math::FixedPointBase kXtoB = -2.0 / 3.0;
  static constexpr math::FixedPointBase kXtoC = 1.0 / 3.0;
  static constexpr math::FixedPointBase kYtoC = -std::numbers::sqrt3 / 3.0;

  Sensor& sensor_;
  Motor& motor_;

  RadsPerSecond target_speed_ = 0;
  RadsPerSecond current_speed_ = 0;
  math::FixedPointBase last_angle_ = 0;
  std::chrono::steady_clock::time_point last_update_time_;
};

template <AngleSensorConcept Sensor, MotorConcept Motor>
MotorController<Sensor, Motor>::MotorController(Sensor& sensor, Motor& motor)
    : sensor_(sensor),
      motor_(motor),
      last_update_time_(std::chrono::steady_clock::now()) {}

template <AngleSensorConcept Sensor, MotorConcept Motor>
void MotorController<Sensor, Motor>::Process() {
  auto current_time = std::chrono::steady_clock::now();
  auto current_angle = sensor_.GetPos();
  auto target_field_angle =
      current_angle + math::kPi<16> * (target_speed_ < 0 ? -1 : 1) / 2;

  auto a_part =
      (kXtoA * cos(target_field_angle)) + (kYtoA * sin(target_field_angle));
  auto b_part = (kXtoB * cos(target_field_angle));
  auto c_part =
      (kXtoC * cos(target_field_angle)) + (kYtoC * sin(target_field_angle));

  current_speed_ = (current_angle - last_angle_) /
                   math::FixedPointBase(current_time - last_update_time_);

  auto gain = target_speed_ - current_speed_;
  motor_.SetAPhaseVoltage(a_part * gain);
  motor_.SetBPhaseVoltage(b_part * gain);
  motor_.SetCPhaseVoltage(c_part * gain);

  last_angle_ = current_angle;
  last_update_time_ = current_time;
}

template <AngleSensorConcept Sensor, MotorConcept Motor>
void MotorController<Sensor, Motor>::SetSpeed(RadsPerSecond speed) {
  target_speed_ = speed;
}
}  // namespace hydrolib::motor