#include "hydrolib_fixed_point.hpp"
#include "hydrolib_quaternions.hpp"
#include "hydrolib_vector3d.hpp"

#include <gtest/gtest.h>

using namespace hydrolib::math;

TEST(TestHydrolibMath, QuaternionConstructorWithComponents)
{
    Quaternion<FixedPointBase> q(FixedPointBase(1), FixedPointBase(2),
                                 FixedPointBase(3), FixedPointBase(4));

    EXPECT_EQ(q.x, FixedPointBase(1));
    EXPECT_EQ(q.y, FixedPointBase(2));
    EXPECT_EQ(q.z, FixedPointBase(3));
    EXPECT_EQ(q.w, FixedPointBase(4));
}

TEST(TestHydrolibMath, QuaternionConstructorWithVector3D)
{
    Vector3D<FixedPointBase> v{FixedPointBase(10), FixedPointBase(20),
                               FixedPointBase(30)};
    Quaternion<FixedPointBase> q(v);

    EXPECT_EQ(q.x, FixedPointBase(10));
    EXPECT_EQ(q.y, FixedPointBase(20));
    EXPECT_EQ(q.z, FixedPointBase(30));
    EXPECT_EQ(q.w, FixedPointBase(0));
}

TEST(TestHydrolibMath, QuaternionAddition)
{
    Quaternion<FixedPointBase> q1(FixedPointBase(1), FixedPointBase(2),
                                  FixedPointBase(3), FixedPointBase(4));
    Quaternion<FixedPointBase> q2(FixedPointBase(5), FixedPointBase(6),
                                  FixedPointBase(7), FixedPointBase(8));
    Quaternion<FixedPointBase> result = q1 + q2;

    EXPECT_EQ(result.x, FixedPointBase(6));
    EXPECT_EQ(result.y, FixedPointBase(8));
    EXPECT_EQ(result.z, FixedPointBase(10));
    EXPECT_EQ(result.w, FixedPointBase(12));
}

TEST(TestHydrolibMath, QuaternionAdditionAssignment)
{
    Quaternion<FixedPointBase> q1(FixedPointBase(1), FixedPointBase(2),
                                  FixedPointBase(3), FixedPointBase(4));
    Quaternion<FixedPointBase> q2(FixedPointBase(5), FixedPointBase(6),
                                  FixedPointBase(7), FixedPointBase(8));
    q1 += q2;

    EXPECT_EQ(q1.x, FixedPointBase(6));
    EXPECT_EQ(q1.y, FixedPointBase(8));
    EXPECT_EQ(q1.z, FixedPointBase(10));
    EXPECT_EQ(q1.w, FixedPointBase(12));
}

TEST(TestHydrolibMath, QuaternionSubtraction)
{
    Quaternion<FixedPointBase> q1(FixedPointBase(10), FixedPointBase(20),
                                  FixedPointBase(30), FixedPointBase(40));
    Quaternion<FixedPointBase> q2(FixedPointBase(5), FixedPointBase(6),
                                  FixedPointBase(7), FixedPointBase(8));
    Quaternion<FixedPointBase> result = q1 - q2;

    EXPECT_EQ(result.x, FixedPointBase(5));
    EXPECT_EQ(result.y, FixedPointBase(14));
    EXPECT_EQ(result.z, FixedPointBase(23));
    EXPECT_EQ(result.w, FixedPointBase(32));
}

TEST(TestHydrolibMath, QuaternionUnaryMinus)
{
    Quaternion<FixedPointBase> q(FixedPointBase(1), FixedPointBase(-2),
                                 FixedPointBase(3), FixedPointBase(-4));
    Quaternion<FixedPointBase> result = -q;

    EXPECT_EQ(result.x, FixedPointBase(-1));
    EXPECT_EQ(result.y, FixedPointBase(2));
    EXPECT_EQ(result.z, FixedPointBase(-3));
    EXPECT_EQ(result.w, FixedPointBase(4));
}

TEST(TestHydrolibMath, QuaternionMultiplication)
{
    Quaternion<FixedPointBase> q1(FixedPointBase(1), FixedPointBase(0),
                                  FixedPointBase(0), FixedPointBase(0));
    Quaternion<FixedPointBase> q2(FixedPointBase(0), FixedPointBase(1),
                                  FixedPointBase(0), FixedPointBase(0));
    Quaternion<FixedPointBase> result = q1 * q2;

    EXPECT_EQ(result.x, FixedPointBase(0));
    EXPECT_EQ(result.y, FixedPointBase(0));
    EXPECT_EQ(result.z, FixedPointBase(1));
    EXPECT_EQ(result.w, FixedPointBase(0));
}

TEST(TestHydrolibMath, QuaternionMultiplicationWithIdentity)
{
    Quaternion<FixedPointBase> q(FixedPointBase(1), FixedPointBase(2),
                                 FixedPointBase(3), FixedPointBase(0));
    Quaternion<FixedPointBase> identity(FixedPointBase(0), FixedPointBase(0),
                                        FixedPointBase(0), FixedPointBase(1));
    Quaternion<FixedPointBase> result = q * identity;

    EXPECT_EQ(result.x, FixedPointBase(1));
    EXPECT_EQ(result.y, FixedPointBase(2));
    EXPECT_EQ(result.z, FixedPointBase(3));
    EXPECT_EQ(result.w, FixedPointBase(0));
}

TEST(TestHydrolibMath, QuaternionConjugate)
{
    Quaternion<FixedPointBase> q(FixedPointBase(1), FixedPointBase(2),
                                 FixedPointBase(3), FixedPointBase(4));
    Quaternion<FixedPointBase> conjugate = !q;

    EXPECT_EQ(conjugate.x, FixedPointBase(-1));
    EXPECT_EQ(conjugate.y, FixedPointBase(-2));
    EXPECT_EQ(conjugate.z, FixedPointBase(-3));
    EXPECT_EQ(conjugate.w, FixedPointBase(4));
}

TEST(TestHydrolibMath, QuaternionScalarMultiplication)
{
    Quaternion<FixedPointBase> q(FixedPointBase(2), FixedPointBase(4),
                                 FixedPointBase(6), FixedPointBase(8));
    Quaternion<FixedPointBase> result = q * FixedPointBase(3);

    EXPECT_EQ(result.x, FixedPointBase(6));
    EXPECT_EQ(result.y, FixedPointBase(12));
    EXPECT_EQ(result.z, FixedPointBase(18));
    EXPECT_EQ(result.w, FixedPointBase(24));
}

TEST(TestHydrolibMath, QuaternionScalarDivision)
{
    Quaternion<FixedPointBase> q(FixedPointBase(6), FixedPointBase(12),
                                 FixedPointBase(18), FixedPointBase(24));
    Quaternion<FixedPointBase> result = q / FixedPointBase(3);

    EXPECT_EQ(result.x, FixedPointBase(2));
    EXPECT_EQ(result.y, FixedPointBase(4));
    EXPECT_EQ(result.z, FixedPointBase(6));
    EXPECT_EQ(result.w, FixedPointBase(8));
}

TEST(TestHydrolibMath, QuaternionNorm)
{
    Quaternion<FixedPointBase> q(FixedPointBase(3), FixedPointBase(4),
                                 FixedPointBase(0), FixedPointBase(0));
    FixedPointBase norm = q.GetNorm();

    EXPECT_EQ(norm, FixedPointBase(5));
}

TEST(TestHydrolibMath, QuaternionNormZero)
{
    Quaternion<FixedPointBase> q(FixedPointBase(0), FixedPointBase(0),
                                 FixedPointBase(0), FixedPointBase(0));
    FixedPointBase norm = q.GetNorm();

    EXPECT_EQ(norm, FixedPointBase(0));
}

TEST(TestHydrolibMath, QuaternionNormalize)
{
    Quaternion<FixedPointBase> q(FixedPointBase(3), FixedPointBase(4),
                                 FixedPointBase(0), FixedPointBase(0));
    q.Normalize();

    auto norm = q.GetNorm();
    EXPECT_EQ(norm, FixedPointBase(1));
}

TEST(TestHydrolibMath, QuaternionChainOperations)
{
    Quaternion<FixedPointBase> q1(FixedPointBase(1), FixedPointBase(2),
                                  FixedPointBase(3), FixedPointBase(4));
    Quaternion<FixedPointBase> q2(FixedPointBase(2), FixedPointBase(3),
                                  FixedPointBase(4), FixedPointBase(5));
    Quaternion<FixedPointBase> q3(FixedPointBase(1), FixedPointBase(1),
                                  FixedPointBase(1), FixedPointBase(1));

    Quaternion<FixedPointBase> result = (q1 + q2) - q3;

    EXPECT_EQ(result.x, FixedPointBase(2));
    EXPECT_EQ(result.y, FixedPointBase(4));
    EXPECT_EQ(result.z, FixedPointBase(6));
    EXPECT_EQ(result.w, FixedPointBase(8));
}

TEST(TestHydrolibMath, QuaternionNegativeComponents)
{
    Quaternion<FixedPointBase> q(FixedPointBase(-1), FixedPointBase(-2),
                                 FixedPointBase(-3), FixedPointBase(-4));

    EXPECT_EQ(q.x, FixedPointBase(-1));
    EXPECT_EQ(q.y, FixedPointBase(-2));
    EXPECT_EQ(q.z, FixedPointBase(-3));
    EXPECT_EQ(q.w, FixedPointBase(-4));

    Quaternion<FixedPointBase> positive = -q;
    EXPECT_EQ(positive.x, FixedPointBase(1));
    EXPECT_EQ(positive.y, FixedPointBase(2));
    EXPECT_EQ(positive.z, FixedPointBase(3));
    EXPECT_EQ(positive.w, FixedPointBase(4));
}

TEST(TestHydrolibMath, QuaternionScalarOperationsWithZero)
{
    Quaternion<FixedPointBase> q(FixedPointBase(5), FixedPointBase(10),
                                 FixedPointBase(15), FixedPointBase(20));
    Quaternion<FixedPointBase> result_mult = q * FixedPointBase(0);

    EXPECT_EQ(result_mult.x, FixedPointBase(0));
    EXPECT_EQ(result_mult.y, FixedPointBase(0));
    EXPECT_EQ(result_mult.z, FixedPointBase(0));
    EXPECT_EQ(result_mult.w, FixedPointBase(0));
}

TEST(TestHydrolibMath, QuaternionScalarOperationsWithOne)
{
    Quaternion<FixedPointBase> q(FixedPointBase(5), FixedPointBase(10),
                                 FixedPointBase(15), FixedPointBase(20));
    Quaternion<FixedPointBase> result_mult = q * FixedPointBase(1);
    Quaternion<FixedPointBase> result_div = q / FixedPointBase(1);

    EXPECT_EQ(result_mult.x, FixedPointBase(5));
    EXPECT_EQ(result_mult.y, FixedPointBase(10));
    EXPECT_EQ(result_mult.z, FixedPointBase(15));
    EXPECT_EQ(result_mult.w, FixedPointBase(20));

    EXPECT_EQ(result_div.x, FixedPointBase(5));
    EXPECT_EQ(result_div.y, FixedPointBase(10));
    EXPECT_EQ(result_div.z, FixedPointBase(15));
    EXPECT_EQ(result_div.w, FixedPointBase(20));
}

TEST(TestHydrolibMath, QuaternionComplexMultiplication)
{
    Quaternion<FixedPointBase> q1(FixedPointBase(1), FixedPointBase(2),
                                  FixedPointBase(3), FixedPointBase(4));
    Quaternion<FixedPointBase> q2(FixedPointBase(5), FixedPointBase(6),
                                  FixedPointBase(7), FixedPointBase(8));
    Quaternion<FixedPointBase> result = q1 * q2;

    EXPECT_EQ(result.x, FixedPointBase(24));
    EXPECT_EQ(result.y, FixedPointBase(48));
    EXPECT_EQ(result.z, FixedPointBase(48));
    EXPECT_EQ(result.w, FixedPointBase(-6));
}