#pragma once

#include "hydrolib_fixed_point.hpp"
#include "hydrolib_quaternions.hpp"
#include "hydrolib_rotations.hpp"
#include "hydrolib_vector3d.hpp"

namespace hydrolib::sensors {

template <math::ArithmeticConcept Number, double PERIOD_S>
class IMUProcessor {
 public:
  struct AccelG {
    explicit constexpr AccelG(math::Vector3D<Number> value) : value(value) {}

    math::Vector3D<Number> value;  // NOLINT
  };

  struct GyroRadPerS {
    explicit constexpr GyroRadPerS(math::Vector3D<Number> value)
        : value(value) {}

    math::Vector3D<Number> value;  // NOLINT
  };

  constexpr IMUProcessor() = default;

  math::Rotation<Number> Process(AccelG accel_g, GyroRadPerS gyro_rad_per_s);
  //   void Calibrate(math::Vector3D<Number> g_z,
  //                  math::Vector3D<Number> g_opposite_z,
  //                  math::Vector3D<Number> g_x);

 private:
  math::Rotation<Number> orientation_;
  //   math::Vector3D<Number> accel_absolute_error_;
  //   math::Quaternion<Number> orientation_compensation_;
};

template <math::ArithmeticConcept Number, double PERIOD_S>
math::Rotation<Number> IMUProcessor<Number, PERIOD_S>::Process(
    AccelG accel_g, GyroRadPerS gyro_rad_per_s) {
  //   accel_g = accel_g - accel_absolute_error_;
  //   accel_g = math::Rotate(accel_g, orientation_compensation_);
  Number w_rad_per_s = math::Vector3D<Number>::Length(gyro_rad_per_s.value);
  Number fi_rad = w_rad_per_s * Number(PERIOD_S);
  math::Rotation<Number> gyro_rotation(gyro_rad_per_s.value, fi_rad);
  math::Rotation<Number> model_orientation = orientation_ + gyro_rotation;
  math::Vector3D<Number> g_model =
      model_orientation.Rotate({.x = 0, .y = 0, .z = -1});
  math::Rotation<Number> model_orientation_xy({.x = 0, .y = 0, .z = -1},
                                              g_model);
  math::Rotation<Number> model_orientation_z =
      model_orientation - model_orientation_xy;
  math::Rotation<Number> sensor_orientation({.x = 0, .y = 0, .z = -1},
                                            accel_g.value);
  math::Rotation<Number> completed_sensor_orientation =
      sensor_orientation + model_orientation_z;
  completed_sensor_orientation.Normalize();
  // math::Quaternion<Number> mean_orientation =
  //     math::GetMean(model_orientation, completed_sensor_orientation);
  orientation_ = completed_sensor_orientation;
  return orientation_;
}

// template <math::ArithmeticConcept Number, double PERIOD_S>
// void IMUProcessor<Number, PERIOD_S>::Calibrate(
//     math::Vector3D<Number> g_z, math::Vector3D<Number> g_opposite_z,
//     math::Vector3D<Number> g_x) {
//   accel_absolute_error_ = (g_z + g_opposite_z) / Number(2);
//   math::Vector3D<Number> g_z_compensated = g_z - accel_absolute_error_;
//   math::Vector3D<Number> g_x_compensated = g_x - accel_absolute_error_;
//   math::Quaternion<Number> first_rotation =
//       math::GetRotation(g_z_compensated, {0, 0, 1});
//   math::Vector3D<Number> g_x_rotated =
//       math::Rotate(g_x_compensated, first_rotation);
//   math::Quaternion<Number> second_rotation =
//       math::GetRotation(g_x_rotated, {1, 0, 0});
//   orientation_compensation_ = second_rotation * first_rotation;
// }
}  // namespace hydrolib::sensors
