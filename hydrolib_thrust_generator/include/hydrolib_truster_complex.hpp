#pragma once

namespace hydrolib::controlling {
template <typename Number>
struct Control {
  Number x_force;
  Number y_force;
  Number z_force;

  Number x_torque;
  Number y_torque;
  Number z_torque;
};

template <typename T, typename Number>
concept ThrusterComplexConcept =
    requires(T thruster_complex, Control<Number>& control) {
      { thruster_complex.ProcessWithFeedback(control) };
    };
}  // namespace hydrolib::controlling