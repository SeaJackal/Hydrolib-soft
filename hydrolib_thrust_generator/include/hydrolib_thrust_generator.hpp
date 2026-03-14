#pragma once

#include <algorithm>
#include <array>
#include <concepts>

#include "hydrolib_fixed_point.hpp"
#include "hydrolib_linear_equations.hpp"
#include "hydrolib_return_codes.hpp"

namespace hydrolib::controlling {

template <typename T>
concept ThrusterConcept = requires(T thruster, int speed) {
  { thruster.SetSpeed(speed) } -> std::same_as<ReturnCode>;
};

inline void Error([[maybe_unused]] const char* message) {
  int param = 0;
  [[maybe_unused]] int error = 1 / param;
};

struct Control {
  math::FixedPointBase x_force;
  math::FixedPointBase y_force;
  math::FixedPointBase z_force;

  math::FixedPointBase x_torque;
  math::FixedPointBase y_torque;
  math::FixedPointBase z_torque;
};

template <ThrusterConcept Thruster, int THRUSTERS_COUNT,
          bool ENABLE_SUM_CLAMP = false>
class ThrustGenerator {
 public:
  using ThrusterParamsArray = std::array<double, THRUSTERS_COUNT>;
  using ThrustArray = std::array<math::FixedPointBase, THRUSTERS_COUNT>;

  consteval ThrustGenerator(
      const ThrusterParamsArray& thrust_to_x_rotation,
      const ThrusterParamsArray& thrust_to_y_rotation,
      const ThrusterParamsArray& thrust_to_z_rotation,
      const ThrusterParamsArray& thrust_to_x_linear,
      const ThrusterParamsArray& thrust_to_y_linear,
      const ThrusterParamsArray& thrust_to_z_linear,
      const std::array<Thruster*, THRUSTERS_COUNT>& thrusters,
      math::FixedPointBase single_clamp, math::FixedPointBase sum_clamp = 0);

  void ProcessWithFeedback(Control& control) const;

 private:
  ThrustArray x_rotation_to_thrust_;
  ThrustArray y_rotation_to_thrust_;
  ThrustArray z_rotation_to_thrust_;

  ThrustArray x_linear_to_thrust_;
  ThrustArray y_linear_to_thrust_;
  ThrustArray z_linear_to_thrust_;

  std::array<Thruster*, THRUSTERS_COUNT> thrusters_;

  math::FixedPointBase single_clamp_;

  math::FixedPointBase sum_clamp_;
};

template <ThrusterConcept Thruster, int THRUSTERS_COUNT, bool ENABLE_SUM_CLAMP>
consteval ThrustGenerator<Thruster, THRUSTERS_COUNT, ENABLE_SUM_CLAMP>::
    ThrustGenerator(const ThrusterParamsArray& thrust_to_x_rotation,
                    const ThrusterParamsArray& thrust_to_y_rotation,
                    const ThrusterParamsArray& thrust_to_z_rotation,
                    const ThrusterParamsArray& thrust_to_x_linear,
                    const ThrusterParamsArray& thrust_to_y_linear,
                    const ThrusterParamsArray& thrust_to_z_linear,
                    const std::array<Thruster*, THRUSTERS_COUNT>& thrusters,
                    math::FixedPointBase single_clamp,
                    math::FixedPointBase sum_clamp)
    : thrusters_(thrusters),
      single_clamp_(single_clamp),
      sum_clamp_(sum_clamp) {
  constexpr int kXRotationIndex = 0;
  constexpr int kYRotationIndex = 1;
  constexpr int kZRotationIndex = 2;
  constexpr int kXLinearIndex = 3;
  constexpr int kYLinearIndex = 4;
  constexpr int kZLinearIndex = 5;

  bool error_flag = true;

  ThrusterParamsArray pure_x_rotation = {};
  ThrusterParamsArray x_rotation_to_thrust = {};
  pure_x_rotation[kXRotationIndex] = 1;
  math::SolveLinear<double, THRUSTERS_COUNT>(
      std::array{thrust_to_x_rotation, thrust_to_y_rotation,
                 thrust_to_z_rotation, thrust_to_x_linear, thrust_to_y_linear,
                 thrust_to_z_linear},
      pure_x_rotation, x_rotation_to_thrust);
  for (int i = 0; i < THRUSTERS_COUNT; i++) {
    if (x_rotation_to_thrust[i] != 0) {
      error_flag = false;
    }
    x_rotation_to_thrust_[i] = x_rotation_to_thrust[i];
  }
  if (error_flag) {
    Error("No solution");
  }
  error_flag = true;

  ThrusterParamsArray pure_y_rotation = {};
  ThrusterParamsArray y_rotation_to_thrust = {};
  pure_y_rotation[kYRotationIndex] = 1;
  math::SolveLinear<double, THRUSTERS_COUNT>(
      std::array{thrust_to_x_rotation, thrust_to_y_rotation,
                 thrust_to_z_rotation, thrust_to_x_linear, thrust_to_y_linear,
                 thrust_to_z_linear},
      pure_y_rotation, y_rotation_to_thrust);
  for (int i = 0; i < THRUSTERS_COUNT; i++) {
    if (y_rotation_to_thrust[i] != 0) {
      error_flag = false;
    }
    y_rotation_to_thrust_[i] = y_rotation_to_thrust[i];
  }
  if (error_flag) {
    Error("No solution");
  }
  error_flag = true;

  ThrusterParamsArray pure_z_rotation = {};
  ThrusterParamsArray z_rotation_to_thrust = {};
  pure_z_rotation[kZRotationIndex] = 1;
  math::SolveLinear<double, THRUSTERS_COUNT>(
      std::array{thrust_to_x_rotation, thrust_to_y_rotation,
                 thrust_to_z_rotation, thrust_to_x_linear, thrust_to_y_linear,
                 thrust_to_z_linear},
      pure_z_rotation, z_rotation_to_thrust);
  for (int i = 0; i < THRUSTERS_COUNT; i++) {
    if (z_rotation_to_thrust[i] != 0) {
      error_flag = false;
    }
    z_rotation_to_thrust_[i] = z_rotation_to_thrust[i];
  }
  if (error_flag) {
    Error("No solution");
  }
  error_flag = true;

  ThrusterParamsArray pure_x_linear = {};
  ThrusterParamsArray x_linear_to_thrust = {};
  pure_x_linear[kXLinearIndex] = 1;
  math::SolveLinear<double, THRUSTERS_COUNT>(
      std::array{thrust_to_x_rotation, thrust_to_y_rotation,
                 thrust_to_z_rotation, thrust_to_x_linear, thrust_to_y_linear,
                 thrust_to_z_linear},
      pure_x_linear, x_linear_to_thrust);
  for (int i = 0; i < THRUSTERS_COUNT; i++) {
    if (x_linear_to_thrust[i] != 0) {
      error_flag = false;
    }
    x_linear_to_thrust_[i] = x_linear_to_thrust[i];
  }
  if (error_flag) {
    Error("No solution");
  }
  error_flag = true;

  ThrusterParamsArray pure_y_linear = {};
  ThrusterParamsArray y_linear_to_thrust = {};
  pure_y_linear[kYLinearIndex] = 1;
  math::SolveLinear<double, THRUSTERS_COUNT>(
      std::array{thrust_to_x_rotation, thrust_to_y_rotation,
                 thrust_to_z_rotation, thrust_to_x_linear, thrust_to_y_linear,
                 thrust_to_z_linear},
      pure_y_linear, y_linear_to_thrust);
  for (int i = 0; i < THRUSTERS_COUNT; i++) {
    if (y_linear_to_thrust[i] != 0) {
      error_flag = false;
    }
    y_linear_to_thrust_[i] = y_linear_to_thrust[i];
  }
  if (error_flag) {
    Error("No solution");
  }
  error_flag = true;

  ThrusterParamsArray pure_z_linear = {};
  ThrusterParamsArray z_linear_to_thrust = {};
  pure_z_linear[kZLinearIndex] = 1;
  math::SolveLinear<double, THRUSTERS_COUNT>(
      std::array{thrust_to_x_rotation, thrust_to_y_rotation,
                 thrust_to_z_rotation, thrust_to_x_linear, thrust_to_y_linear,
                 thrust_to_z_linear},
      pure_z_linear, z_linear_to_thrust);
  for (int i = 0; i < THRUSTERS_COUNT; i++) {
    if (z_linear_to_thrust[i] != 0) {
      error_flag = false;
    }
    z_linear_to_thrust_[i] = z_linear_to_thrust[i];
  }
  if (error_flag) {
    Error("No solution");
  }
  error_flag = true;
}

template <ThrusterConcept Thruster, int THRUSTERS_COUNT, bool ENABLE_SUM_CLAMP>
void ThrustGenerator<Thruster, THRUSTERS_COUNT,
                     ENABLE_SUM_CLAMP>::ProcessWithFeedback(Control& control)
    const {
  ThrustArray dest;
  math::FixedPointBase max = 0;
  math::FixedPointBase sum = 0;
  for (int i = 0; i < THRUSTERS_COUNT; i++) {
    dest[i] = static_cast<int>((x_rotation_to_thrust_[i] * control.x_torque) +
                               (x_linear_to_thrust_[i] * control.x_force) +
                               (y_rotation_to_thrust_[i] * control.y_torque) +
                               (y_linear_to_thrust_[i] * control.y_force) +
                               (z_rotation_to_thrust_[i] * control.z_torque) +
                               (z_linear_to_thrust_[i] * control.z_force));
    math::FixedPointBase dest_abs =
        (dest[i] >= static_cast<math::FixedPointBase>(0)) ? dest[i] : -dest[i];

    max = std::max(dest_abs, max);

    if constexpr (ENABLE_SUM_CLAMP) {
      sum += dest_abs;
    }
  }

  math::FixedPointBase enumerator = 1;
  math::FixedPointBase denumerator = 1;

  if (max > single_clamp_) {
    enumerator = single_clamp_;
    denumerator = max;
  }

  if constexpr (ENABLE_SUM_CLAMP) {
    if (sum > sum_clamp_) {
      if (sum_clamp_ * denumerator < sum * enumerator) {
        enumerator = sum_clamp_;
        denumerator = sum;
      }
    }
  }

  if (enumerator == denumerator) {
    for (int i = 0; i < THRUSTERS_COUNT; i++) {
      thrusters_[i]->SetSpeed(
          static_cast<int>(dest[i] * enumerator / denumerator));
    }
    control.x_torque = control.x_torque * enumerator / denumerator;
    control.y_torque = control.y_torque * enumerator / denumerator;
    control.z_torque = control.z_torque * enumerator / denumerator;
    control.x_force = control.x_force * enumerator / denumerator;
    control.y_force = control.y_force * enumerator / denumerator;
    control.z_force = control.z_force * enumerator / denumerator;
  }
}
}  // namespace hydrolib::controlling
