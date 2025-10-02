#pragma once

#include "hydrolib_imu.hpp"

#include "hydrolib_fixed_point.hpp"
#include "hydrolib_quaternions.hpp"
#include "hydrolib_rotations.hpp"
#include "hydrolib_vector3d.hpp"

namespace hydrolib::sensors
{

template <double PERIOD_S>
class IMUProcessor
{
public:
    IMUProcessor();

public:
    math::Quaternion<math::FixedPointBase>
    Process(math::Vector3D<math::FixedPointBase> accel_g,
            math::Vector3D<math::FixedPointBase> gyro_deg_per_s);

private:
    math::Quaternion<math::FixedPointBase> orientation_;
};

template <double PERIOD_S>
IMUProcessor<PERIOD_S>::IMUProcessor() : orientation_(0, 0, 0, 1)
{
}

template <double PERIOD_S>
math::Quaternion<math::FixedPointBase> IMUProcessor<PERIOD_S>::Process(
    math::Vector3D<math::FixedPointBase> accel_g,
    math::Vector3D<math::FixedPointBase> gyro_rad_per_s)
{
    math::FixedPointBase w_rad_per_s = gyro_rad_per_s.Length();
    math::FixedPointBase fi_rad = w_rad_per_s * math::FixedPointBase(PERIOD_S);
    gyro_rad_per_s.Normalize();
    math::Quaternion<math::FixedPointBase> gyro_rotation =
        math::Quaternion<math::FixedPointBase>(
            gyro_rad_per_s * math::sin(fi_rad / 2), math::cos(fi_rad / 2));
    math::Quaternion<math::FixedPointBase> model_orientation =
        orientation_ * gyro_rotation;
    math::Quaternion<math::FixedPointBase> model_orientation_xy =
        model_orientation;
    math::Quaternion<math::FixedPointBase> model_orientation_z =
        math::ExtractZRotation(model_orientation_xy);
    math::Quaternion<math::FixedPointBase> sensor_orientation =
        math::GetRotation({0, 0, -1}, accel_g);
    math::Quaternion<math::FixedPointBase> completed_sensor_orientation =
        model_orientation_z * sensor_orientation;
    math::Quaternion<math::FixedPointBase> mean_orientation =
        math::GetMean(model_orientation, completed_sensor_orientation);
    orientation_ = model_orientation;
    return orientation_;
}
} // namespace hydrolib::sensors
