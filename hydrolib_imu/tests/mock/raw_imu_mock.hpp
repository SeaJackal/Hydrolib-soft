#pragma once

#include "hydrolib_fixed_point.hpp"
#include "hydrolib_quaternions.hpp"
#include "hydrolib_rotations.hpp"
#include "hydrolib_vector3d.hpp"

namespace hydrolib::sensors
{

class RawIMUMock
{
public:
    RawIMUMock();

public:
    void SetTarget(math::Vector3D<math::FixedPointBase> axis,
                   math::FixedPointBase angle_rad, int n);

    bool Step();

    math::Vector3D<math::FixedPointBase> GetAcceleration() const;
    math::Vector3D<math::FixedPointBase> GetGyro() const;
    math::Quaternion<math::FixedPointBase> GetOrientation() const;

private:
    int counter_;
    math::Quaternion<math::FixedPointBase> orientation_;
    math::Quaternion<math::FixedPointBase> delta_orientation_;
    math::Vector3D<math::FixedPointBase> w_;
};

inline RawIMUMock::RawIMUMock()
    : counter_(0), orientation_(0, 0, 0, 1), delta_orientation_(0, 0, 0, 1)
{
}

inline void RawIMUMock::SetTarget(math::Vector3D<math::FixedPointBase> axis,
                                  math::FixedPointBase angle_rad, int n)
{
    auto delta_angle_rad = angle_rad / n;
    delta_orientation_ = {axis * math::sin(delta_angle_rad / 2),
                          math::cos(delta_angle_rad / 2)};
    w_ = axis * delta_angle_rad;
    counter_ = n;
}

inline bool RawIMUMock::Step()
{
    if (counter_ == 0)
    {
        w_ = {0, 0, 0};
        return false;
    }
    counter_--;
    orientation_ = orientation_ * delta_orientation_;
    return true;
}

inline math::Vector3D<math::FixedPointBase> RawIMUMock::GetAcceleration() const
{
    math::Vector3D<math::FixedPointBase> g(0, 0, -1);
    auto result = math::Rotate(g, orientation_);
    result.Normalize();
    return result;
}

inline math::Vector3D<math::FixedPointBase> RawIMUMock::GetGyro() const
{
    return w_;
}

inline math::Quaternion<math::FixedPointBase> RawIMUMock::GetOrientation() const
{
    return orientation_;
}

} // namespace hydrolib::sensors