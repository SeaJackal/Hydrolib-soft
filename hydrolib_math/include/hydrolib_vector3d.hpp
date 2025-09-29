#pragma once

#include "hydrolib_fixed_point.hpp"

namespace hydrolib::math
{
template <ArithmeticConcept Number>
struct Vector3D
{
public:
    Number Dot(Vector3D &other) const;
    Vector3D Cross(Vector3D &other) const;

public:
    Number x;
    Number y;
    Number z;
};

template <ArithmeticConcept Number>
inline Number Vector3D<Number>::Dot(Vector3D<Number> &other) const
{
    return (x * other.x) + (y * other.y) + (z * other.z);
}

template <ArithmeticConcept Number>
inline Vector3D<Number> Vector3D<Number>::Cross(Vector3D<Number> &other) const
{
    return Vector3D<Number>(y * other.z - z * other.y,
                            z * other.x - x * other.z,
                            x * other.y - y * other.x);
}
} // namespace hydrolib::math