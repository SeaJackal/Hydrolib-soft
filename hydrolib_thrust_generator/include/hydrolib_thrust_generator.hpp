#pragma once

#include <algorithm>
#include <array>

#include "hydrolib_fixed_point.hpp"
#include "hydrolib_linear_equations.hpp"

namespace hydrolib::controlling {

struct Control {
  math::FixedPointBase x_force;
  math::FixedPointBase y_force;
  math::FixedPointBase z_force;

  math::FixedPointBase x_torque;
  math::FixedPointBase y_torque;
  math::FixedPointBase z_torque;
};

template <int THRUSTERS_COUNT, bool ENABLE_SUM_CLAMP = false>
class ThrustGenerator {
 public:
  using ThrustArray = std::array<math::FixedPointBase, THRUSTERS_COUNT>;

  constexpr ThrustGenerator(const ThrustArray& thrust_to_x_rotation,
                            const ThrustArray& thrust_to_y_rotation,
                            const ThrustArray& thrust_to_z_rotation,
                            const ThrustArray& thrust_to_x_linear,
                            const ThrustArray& thrust_to_y_linear,
                            const ThrustArray& thrust_to_z_linear,
                            int single_clamp, int sum_clamp = 0);

  void ProcessWithFeedback(Control& control,
                           std::array<int, THRUSTERS_COUNT>& dest);

 private:
  ThrustArray x_rotation_to_thrust_;
  ThrustArray y_rotation_to_thrust_;
  ThrustArray z_rotation_to_thrust_;

  ThrustArray x_linear_to_thrust_;
  ThrustArray y_linear_to_thrust_;
  ThrustArray z_linear_to_thrust_;

  int single_clamp_;

  int sum_clamp_;
};

template <int THRUSTERS_COUNT, bool ENABLE_SUM_CLAMP>
constexpr ThrustGenerator<THRUSTERS_COUNT, ENABLE_SUM_CLAMP>::ThrustGenerator(
    const ThrustArray& thrust_to_x_rotation,
    const ThrustArray& thrust_to_y_rotation,
    const ThrustArray& thrust_to_z_rotation,
    const ThrustArray& thrust_to_x_linear,
    const ThrustArray& thrust_to_y_linear,
    const ThrustArray& thrust_to_z_linear, int single_clamp, int sum_clamp)
    : single_clamp_(single_clamp), sum_clamp_(sum_clamp) {
  constexpr int kXRotationIndex = 0;
  constexpr int kYRotationIndex = 1;
  constexpr int kZRotationIndex = 2;
  constexpr int kXLinearIndex = 3;
  constexpr int kYLinearIndex = 4;
  constexpr int kZLinearIndex = 5;

  ThrustArray pure_x_rotation = {};
  pure_x_rotation[kXRotationIndex] = 1;
  math::SolveLinear<math::FixedPointBase, THRUSTERS_COUNT>(
      std::array{thrust_to_x_rotation, thrust_to_y_rotation,
                 thrust_to_z_rotation, thrust_to_x_linear, thrust_to_y_linear,
                 thrust_to_z_linear},
      pure_x_rotation, x_rotation_to_thrust_);
  ThrustArray pure_y_rotation = {};
  pure_y_rotation[kYRotationIndex] = 1;
  math::SolveLinear<math::FixedPointBase, THRUSTERS_COUNT>(std::array{thrust_to_x_rotation, thrust_to_y_rotation,
                               thrust_to_z_rotation, thrust_to_x_linear,
                               thrust_to_y_linear, thrust_to_z_linear},
                    pure_y_rotation, y_rotation_to_thrust_);
  ThrustArray pure_z_rotation = {};
  pure_z_rotation[kZRotationIndex] = 1;
  math::SolveLinear<math::FixedPointBase, THRUSTERS_COUNT>(std::array{thrust_to_x_rotation, thrust_to_y_rotation,
                               thrust_to_z_rotation, thrust_to_x_linear,
                               thrust_to_y_linear, thrust_to_z_linear},
                    pure_z_rotation, z_rotation_to_thrust_);
  ThrustArray pure_x_linear = {};
  pure_x_linear[kXLinearIndex] = 1;
  math::SolveLinear<math::FixedPointBase, THRUSTERS_COUNT>(std::array{thrust_to_x_rotation, thrust_to_y_rotation,
                               thrust_to_z_rotation, thrust_to_x_linear,
                               thrust_to_y_linear, thrust_to_z_linear},
                    pure_x_linear, x_linear_to_thrust_);
  ThrustArray pure_y_linear = {};
  pure_y_linear[kYLinearIndex] = 1;
  math::SolveLinear<math::FixedPointBase, THRUSTERS_COUNT>(std::array{thrust_to_x_rotation, thrust_to_y_rotation,
                               thrust_to_z_rotation, thrust_to_x_linear,
                               thrust_to_y_linear, thrust_to_z_linear},
                    pure_y_linear, y_linear_to_thrust_);
  ThrustArray pure_z_linear = {};
  pure_z_linear[kZLinearIndex] = 1;
  math::SolveLinear<math::FixedPointBase, THRUSTERS_COUNT>(
      std::array{thrust_to_x_rotation, thrust_to_y_rotation,
                                 thrust_to_z_rotation, thrust_to_x_linear,
                                 thrust_to_y_linear, thrust_to_z_linear},
      pure_z_linear, z_linear_to_thrust_);
}

template <int THRUSTERS_COUNT, bool ENABLE_SUM_CLAMP>
void ThrustGenerator<THRUSTERS_COUNT, ENABLE_SUM_CLAMP>::ProcessWithFeedback(
    Control& control, std::array<int, THRUSTERS_COUNT>& dest) {
  int max = 0;
  int sum = 0;
  for (int i = 0; i < THRUSTERS_COUNT; i++) {
    dest[i] = static_cast<int>((x_rotation_to_thrust_[i] * control.x_torque) +
                               (x_linear_to_thrust_[i] * control.x_force) +
                               (y_rotation_to_thrust_[i] * control.y_torque) +
                               (y_linear_to_thrust_[i] * control.y_force) +
                               (z_rotation_to_thrust_[i] * control.z_torque) +
                               (z_linear_to_thrust_[i] * control.z_force));
    int dest_abs = dest[i] >= 0 ? dest[i] : -dest[i];

    max = std::max(dest_abs, max);

    if constexpr (ENABLE_SUM_CLAMP) {
      sum += dest_abs;
    }
  }

  int enumerator = 1;
  int denumerator = 1;

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
      dest[i] = dest[i] * enumerator / denumerator;
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
