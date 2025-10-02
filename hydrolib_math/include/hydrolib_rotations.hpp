#pragma once

#include "hydrolib_fixed_point.hpp"
#include "hydrolib_quaternions.hpp"
#include "hydrolib_vector3d.hpp"

namespace hydrolib::math
{
Quaternion<FixedPointBase> GetRotation(Vector3D<FixedPointBase> from,
                                     Vector3D<FixedPointBase> to);

Vector3D<FixedPointBase> Rotate(Vector3D<FixedPointBase> source,
                              Quaternion<FixedPointBase> rotation);

Quaternion<FixedPointBase> ExtractZRotation(Quaternion<FixedPointBase> &quaternion);

Quaternion<FixedPointBase> GetMean(const Quaternion<FixedPointBase> &q1,
                                 const Quaternion<FixedPointBase> &q2);

/////////////////////////////////////////////////////////////////////////

inline Quaternion<FixedPointBase> GetRotation(Vector3D<FixedPointBase> from,
                                            Vector3D<FixedPointBase> to)
{
    from.Normalize();
    to.Normalize();
    Quaternion<FixedPointBase> result =
        Quaternion<FixedPointBase>(from.Cross(to), from.Dot(to) + 1);
    result.Normalize();
    return result;
}

inline Vector3D<FixedPointBase> Rotate(Vector3D<FixedPointBase> source,
                                     Quaternion<FixedPointBase> rotation)
{
    auto result = rotation * Quaternion<FixedPointBase>(source) * (!rotation);
    return {.x = result.x, .y = result.y, .z = result.z};
}

inline Quaternion<FixedPointBase>
ExtractZRotation(Quaternion<FixedPointBase> &quaternion)
{
    FixedPointBase new_w =
        sqrt(quaternion.w * quaternion.w + quaternion.z * quaternion.z);
    FixedPointBase sin_yaw = quaternion.z / new_w;
    FixedPointBase cos_yaw = quaternion.w / new_w;
    FixedPointBase new_x = quaternion.x * cos_yaw + quaternion.y * sin_yaw;
    FixedPointBase new_y = quaternion.y * cos_yaw - quaternion.x * sin_yaw;
    quaternion.x = new_x;
    quaternion.y = new_y;
    quaternion.z = 0;
    quaternion.w = new_w;
    return Quaternion<FixedPointBase>(0, 0, sin_yaw, cos_yaw);
}

inline Quaternion<FixedPointBase> GetMean(const Quaternion<FixedPointBase> &q1,
                                        const Quaternion<FixedPointBase> &q2)
{
    if (q1.Dot(q2) < 0)
    {
        auto result = (q1 - q2) / 2;
        result.Normalize();
        return result;
    }
    else
    {
        auto result = (q1 + q2) / 2;
        result.Normalize();
        return result;
    }
}
} // namespace hydrolib::math
