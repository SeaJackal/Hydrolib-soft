#pragma once

#include <concepts>

#include "hydrolib_fixed_point.hpp"
#include "hydrolib_rotations.hpp"

namespace hydrolib::sensors {

template <typename T, typename Number>
concept IMUConcept = requires(T imu) {
  { imu.GetOrientation() } -> std::same_as<math::Rotation<Number>>;
};

}  // namespace hydrolib::sensors