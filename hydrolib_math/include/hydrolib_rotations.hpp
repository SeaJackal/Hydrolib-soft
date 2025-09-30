#pragma once

#include "hydrolib_fixed_point.hpp"
#include "hydrolib_quaternions.hpp"
#include "hydrolib_vector3d.hpp"

namespace hydrolib::math
{
Quaternion<FixedPoint10> GetRotation(Vector3D<FixedPoint10> from,
                                     Vector3D<FixedPoint10> to);

Vector3D<FixedPoint10> Rotate(Vector3D<FixedPoint10> source,
                              Quaternion<FixedPoint10> rotation);

/////////////////////////////////////////////////////////////////////////

inline Quaternion<FixedPoint10> GetRotation(Vector3D<FixedPoint10> from,
                                            Vector3D<FixedPoint10> to)
{
    from.Normalize();
    to.Normalize();
    Quaternion<FixedPoint10> result =
        Quaternion<FixedPoint10>(from.Cross(to), from.Dot(to) + 1);
    result.Normalize();
    return result;
}

inline Vector3D<FixedPoint10> Rotate(Vector3D<FixedPoint10> source,
                                     Quaternion<FixedPoint10> rotation)
{
    auto result = rotation * Quaternion<FixedPoint10>(source) * (!rotation);
    return {.x = result.x, .y = result.y, .z = result.z};
}
} // namespace hydrolib::math
