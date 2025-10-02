#include "hydrolib_fixed_point.hpp"
#include "hydrolib_quaternions.hpp"
#include "hydrolib_rotations.hpp"
#include "hydrolib_vector3d.hpp"

#include <gtest/gtest.h>

using namespace hydrolib::math;

TEST(TestHydrolibMathRotations, Rotation)
{
    Vector3D<FixedPointBase> a{1, 1, 1};
    Vector3D<FixedPointBase> b{0, 0, 1.732};

    auto q = GetRotation(a, b);
    auto result = Rotate(a, q);
    EXPECT_NEAR(static_cast<double>(b.x), static_cast<double>(result.x), 0.05);
    EXPECT_NEAR(static_cast<double>(b.y), static_cast<double>(result.y), 0.05);
    EXPECT_NEAR(static_cast<double>(b.z), static_cast<double>(result.z), 0.05);
}

TEST(TestHydrolibMathRotations, AxisRotation)
{
    Vector3D<FixedPointBase> x{1, 0, 0};
    Vector3D<FixedPointBase> x_{0, 0, 1};
    Vector3D<FixedPointBase> y{0, 1, 0};
    Vector3D<FixedPointBase> y_{0, 1, 0};
    Vector3D<FixedPointBase> z{0, 0, 1};
    Vector3D<FixedPointBase> z_{-1, 0, 0};

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

TEST(TestHydrolibMath, QuaternionExtractZRotation)
{
    Quaternion<FixedPointBase> q_z(0, 0, FixedPointBase(1), FixedPointBase(1));
    q_z.Normalize();
    Quaternion<FixedPointBase> q_xy(FixedPointBase(1), FixedPointBase(1), 0,
                                    FixedPointBase(1));
    q_xy.Normalize();
    Quaternion<FixedPointBase> q = q_z * q_xy;

    Quaternion<FixedPointBase> result = ExtractZRotation(q);
    EXPECT_NEAR(static_cast<double>(result.x), static_cast<double>(q_z.x),
                0.003);
    EXPECT_NEAR(static_cast<double>(result.y), static_cast<double>(q_z.y),
                0.003);
    EXPECT_NEAR(static_cast<double>(result.z), static_cast<double>(q_z.z),
                0.003);
    EXPECT_NEAR(static_cast<double>(result.w), static_cast<double>(q_z.w),
                0.003);
    EXPECT_NEAR(static_cast<double>(q.x), static_cast<double>(q_xy.x), 0.003);
    EXPECT_NEAR(static_cast<double>(q.y), static_cast<double>(q_xy.y), 0.003);
    EXPECT_NEAR(static_cast<double>(q.z), static_cast<double>(q_xy.z), 0.003);
    EXPECT_NEAR(static_cast<double>(q.w), static_cast<double>(q_xy.w), 0.003);
}

TEST(TestHydrolibMath, QuaternionExtractZRotationReversed)
{
    Quaternion<FixedPointBase> q(FixedPointBase(1), FixedPointBase(1),
                                 FixedPointBase(1), FixedPointBase(1));
    q.Normalize();

    Quaternion<FixedPointBase> q_xy = q;

    Quaternion<FixedPointBase> q_z = ExtractZRotation(q_xy);
    auto result = q_z * q_xy;
    EXPECT_NEAR(static_cast<double>(q.x), static_cast<double>(result.x), 0.002);
    EXPECT_NEAR(static_cast<double>(q.y), static_cast<double>(result.y), 0.002);
    EXPECT_NEAR(static_cast<double>(q.z), static_cast<double>(result.z), 0.002);
    EXPECT_NEAR(static_cast<double>(q.w), static_cast<double>(result.w), 0.002);
}
