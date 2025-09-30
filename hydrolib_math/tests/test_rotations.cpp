#include "hydrolib_fixed_point.hpp"
#include "hydrolib_quaternions.hpp"
#include "hydrolib_rotations.hpp"
#include "hydrolib_vector3d.hpp"

#include <gtest/gtest.h>

using namespace hydrolib::math;

TEST(TestHydrolibMathRotations, Rotation)
{
    Vector3D<FixedPoint10> a{1, 1, 1};
    Vector3D<FixedPoint10> b{0, 0, 1.732};

    auto q = GetRotation(a, b);
    auto result = Rotate(a, q);
    EXPECT_NEAR(static_cast<double>(b.x), static_cast<double>(result.x), 0.05);
    EXPECT_NEAR(static_cast<double>(b.y), static_cast<double>(result.y), 0.05);
    EXPECT_NEAR(static_cast<double>(b.z), static_cast<double>(result.z), 0.05);
}

TEST(TestHydrolibMathRotations, AxisRotation)
{
    Vector3D<FixedPoint10> x{1, 0, 0};
    Vector3D<FixedPoint10> x_{0, 0, 1};
    Vector3D<FixedPoint10> y{0, 1, 0};
    Vector3D<FixedPoint10> y_{0, 1, 0};
    Vector3D<FixedPoint10> z{0, 0, 1};
    Vector3D<FixedPoint10> z_{-1, 0, 0};

    auto q = GetRotation(x, x_);
    auto result_y = Rotate(y, q);
    auto result_z = Rotate(z, q);
    EXPECT_NEAR(static_cast<double>(y_.x), static_cast<double>(result_y.x),
                0.05);
    EXPECT_NEAR(static_cast<double>(y_.y), static_cast<double>(result_y.y),
                0.05);
    EXPECT_NEAR(static_cast<double>(y_.z), static_cast<double>(result_y.z),
                0.05);
    EXPECT_NEAR(static_cast<double>(z_.x), static_cast<double>(result_z.x),
                0.05);
    EXPECT_NEAR(static_cast<double>(z_.y), static_cast<double>(result_z.y),
                0.05);
    EXPECT_NEAR(static_cast<double>(z_.z), static_cast<double>(result_z.z),
                0.05);
}
