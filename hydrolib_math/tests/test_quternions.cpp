#include "hydrolib_fixed_point.hpp"
#include "hydrolib_quaternions.hpp"
#include "hydrolib_vector3d.hpp"

#include <gtest/gtest.h>

using namespace hydrolib::math;

TEST(TestHydrolibMath, QuaternionConstructorWithComponents)
{
    Quaternion<FixedPoint10> q(FixedPoint10(1), FixedPoint10(2),
                               FixedPoint10(3), FixedPoint10(4));

    EXPECT_EQ(q.x, FixedPoint10(1));
    EXPECT_EQ(q.y, FixedPoint10(2));
    EXPECT_EQ(q.z, FixedPoint10(3));
    EXPECT_EQ(q.w, FixedPoint10(4));
}

TEST(TestHydrolibMath, QuaternionConstructorWithVector3D)
{
    Vector3D<FixedPoint10> v{FixedPoint10(10), FixedPoint10(20),
                             FixedPoint10(30)};
    Quaternion<FixedPoint10> q(v);

    EXPECT_EQ(q.x, FixedPoint10(10));
    EXPECT_EQ(q.y, FixedPoint10(20));
    EXPECT_EQ(q.z, FixedPoint10(30));
    EXPECT_EQ(q.w, FixedPoint10(0));
}

TEST(TestHydrolibMath, QuaternionAddition)
{
    Quaternion<FixedPoint10> q1(FixedPoint10(1), FixedPoint10(2),
                                FixedPoint10(3), FixedPoint10(4));
    Quaternion<FixedPoint10> q2(FixedPoint10(5), FixedPoint10(6),
                                FixedPoint10(7), FixedPoint10(8));
    Quaternion<FixedPoint10> result = q1 + q2;

    EXPECT_EQ(result.x, FixedPoint10(6));
    EXPECT_EQ(result.y, FixedPoint10(8));
    EXPECT_EQ(result.z, FixedPoint10(10));
    EXPECT_EQ(result.w, FixedPoint10(12));
}

TEST(TestHydrolibMath, QuaternionAdditionAssignment)
{
    Quaternion<FixedPoint10> q1(FixedPoint10(1), FixedPoint10(2),
                                FixedPoint10(3), FixedPoint10(4));
    Quaternion<FixedPoint10> q2(FixedPoint10(5), FixedPoint10(6),
                                FixedPoint10(7), FixedPoint10(8));
    q1 += q2;

    EXPECT_EQ(q1.x, FixedPoint10(6));
    EXPECT_EQ(q1.y, FixedPoint10(8));
    EXPECT_EQ(q1.z, FixedPoint10(10));
    EXPECT_EQ(q1.w, FixedPoint10(12));
}

TEST(TestHydrolibMath, QuaternionSubtraction)
{
    Quaternion<FixedPoint10> q1(FixedPoint10(10), FixedPoint10(20),
                                FixedPoint10(30), FixedPoint10(40));
    Quaternion<FixedPoint10> q2(FixedPoint10(5), FixedPoint10(6),
                                FixedPoint10(7), FixedPoint10(8));
    Quaternion<FixedPoint10> result = q1 - q2;

    EXPECT_EQ(result.x, FixedPoint10(5));
    EXPECT_EQ(result.y, FixedPoint10(14));
    EXPECT_EQ(result.z, FixedPoint10(23));
    EXPECT_EQ(result.w, FixedPoint10(32));
}

TEST(TestHydrolibMath, QuaternionUnaryMinus)
{
    Quaternion<FixedPoint10> q(FixedPoint10(1), FixedPoint10(-2),
                               FixedPoint10(3), FixedPoint10(-4));
    Quaternion<FixedPoint10> result = -q;

    EXPECT_EQ(result.x, FixedPoint10(-1));
    EXPECT_EQ(result.y, FixedPoint10(2));
    EXPECT_EQ(result.z, FixedPoint10(-3));
    EXPECT_EQ(result.w, FixedPoint10(4));
}

TEST(TestHydrolibMath, QuaternionMultiplication)
{
    Quaternion<FixedPoint10> q1(FixedPoint10(1), FixedPoint10(0),
                                FixedPoint10(0), FixedPoint10(0));
    Quaternion<FixedPoint10> q2(FixedPoint10(0), FixedPoint10(1),
                                FixedPoint10(0), FixedPoint10(0));
    Quaternion<FixedPoint10> result = q1 * q2;

    EXPECT_EQ(result.x, FixedPoint10(0));
    EXPECT_EQ(result.y, FixedPoint10(0));
    EXPECT_EQ(result.z, FixedPoint10(1));
    EXPECT_EQ(result.w, FixedPoint10(0));
}

TEST(TestHydrolibMath, QuaternionMultiplicationWithIdentity)
{
    Quaternion<FixedPoint10> q(FixedPoint10(1), FixedPoint10(2),
                               FixedPoint10(3), FixedPoint10(0));
    Quaternion<FixedPoint10> identity(FixedPoint10(0), FixedPoint10(0),
                                      FixedPoint10(0), FixedPoint10(1));
    Quaternion<FixedPoint10> result = q * identity;

    EXPECT_EQ(result.x, FixedPoint10(1));
    EXPECT_EQ(result.y, FixedPoint10(2));
    EXPECT_EQ(result.z, FixedPoint10(3));
    EXPECT_EQ(result.w, FixedPoint10(0));
}

TEST(TestHydrolibMath, QuaternionConjugate)
{
    Quaternion<FixedPoint10> q(FixedPoint10(1), FixedPoint10(2),
                               FixedPoint10(3), FixedPoint10(4));
    Quaternion<FixedPoint10> conjugate = !q;

    EXPECT_EQ(conjugate.x, FixedPoint10(-1));
    EXPECT_EQ(conjugate.y, FixedPoint10(-2));
    EXPECT_EQ(conjugate.z, FixedPoint10(-3));
    EXPECT_EQ(conjugate.w, FixedPoint10(4));
}

TEST(TestHydrolibMath, QuaternionScalarMultiplication)
{
    Quaternion<FixedPoint10> q(FixedPoint10(2), FixedPoint10(4),
                               FixedPoint10(6), FixedPoint10(8));
    Quaternion<FixedPoint10> result = q * FixedPoint10(3);

    EXPECT_EQ(result.x, FixedPoint10(6));
    EXPECT_EQ(result.y, FixedPoint10(12));
    EXPECT_EQ(result.z, FixedPoint10(18));
    EXPECT_EQ(result.w, FixedPoint10(24));
}

TEST(TestHydrolibMath, QuaternionScalarDivision)
{
    Quaternion<FixedPoint10> q(FixedPoint10(6), FixedPoint10(12),
                               FixedPoint10(18), FixedPoint10(24));
    Quaternion<FixedPoint10> result = q / FixedPoint10(3);

    EXPECT_EQ(result.x, FixedPoint10(2));
    EXPECT_EQ(result.y, FixedPoint10(4));
    EXPECT_EQ(result.z, FixedPoint10(6));
    EXPECT_EQ(result.w, FixedPoint10(8));
}

TEST(TestHydrolibMath, QuaternionNorm)
{
    Quaternion<FixedPoint10> q(FixedPoint10(3), FixedPoint10(4),
                               FixedPoint10(0), FixedPoint10(0));
    FixedPoint10 norm = q.GetNorm();

    EXPECT_EQ(norm, FixedPoint10(5));
}

TEST(TestHydrolibMath, QuaternionNormZero)
{
    Quaternion<FixedPoint10> q(FixedPoint10(0), FixedPoint10(0),
                               FixedPoint10(0), FixedPoint10(0));
    FixedPoint10 norm = q.GetNorm();

    EXPECT_EQ(norm, FixedPoint10(0));
}

TEST(TestHydrolibMath, QuaternionNormalize)
{
    Quaternion<FixedPoint10> q(FixedPoint10(3), FixedPoint10(4),
                               FixedPoint10(0), FixedPoint10(0));
    q.Normalize();

    EXPECT_EQ(q.GetNorm(), FixedPoint10(1));
}

TEST(TestHydrolibMath, QuaternionChainOperations)
{
    Quaternion<FixedPoint10> q1(FixedPoint10(1), FixedPoint10(2),
                                FixedPoint10(3), FixedPoint10(4));
    Quaternion<FixedPoint10> q2(FixedPoint10(2), FixedPoint10(3),
                                FixedPoint10(4), FixedPoint10(5));
    Quaternion<FixedPoint10> q3(FixedPoint10(1), FixedPoint10(1),
                                FixedPoint10(1), FixedPoint10(1));

    Quaternion<FixedPoint10> result = (q1 + q2) - q3;

    EXPECT_EQ(result.x, FixedPoint10(2));
    EXPECT_EQ(result.y, FixedPoint10(4));
    EXPECT_EQ(result.z, FixedPoint10(6));
    EXPECT_EQ(result.w, FixedPoint10(8));
}

TEST(TestHydrolibMath, QuaternionNegativeComponents)
{
    Quaternion<FixedPoint10> q(FixedPoint10(-1), FixedPoint10(-2),
                               FixedPoint10(-3), FixedPoint10(-4));

    EXPECT_EQ(q.x, FixedPoint10(-1));
    EXPECT_EQ(q.y, FixedPoint10(-2));
    EXPECT_EQ(q.z, FixedPoint10(-3));
    EXPECT_EQ(q.w, FixedPoint10(-4));

    Quaternion<FixedPoint10> positive = -q;
    EXPECT_EQ(positive.x, FixedPoint10(1));
    EXPECT_EQ(positive.y, FixedPoint10(2));
    EXPECT_EQ(positive.z, FixedPoint10(3));
    EXPECT_EQ(positive.w, FixedPoint10(4));
}

TEST(TestHydrolibMath, QuaternionScalarOperationsWithZero)
{
    Quaternion<FixedPoint10> q(FixedPoint10(5), FixedPoint10(10),
                               FixedPoint10(15), FixedPoint10(20));
    Quaternion<FixedPoint10> result_mult = q * FixedPoint10(0);

    EXPECT_EQ(result_mult.x, FixedPoint10(0));
    EXPECT_EQ(result_mult.y, FixedPoint10(0));
    EXPECT_EQ(result_mult.z, FixedPoint10(0));
    EXPECT_EQ(result_mult.w, FixedPoint10(0));
}

TEST(TestHydrolibMath, QuaternionScalarOperationsWithOne)
{
    Quaternion<FixedPoint10> q(FixedPoint10(5), FixedPoint10(10),
                               FixedPoint10(15), FixedPoint10(20));
    Quaternion<FixedPoint10> result_mult = q * FixedPoint10(1);
    Quaternion<FixedPoint10> result_div = q / FixedPoint10(1);

    EXPECT_EQ(result_mult.x, FixedPoint10(5));
    EXPECT_EQ(result_mult.y, FixedPoint10(10));
    EXPECT_EQ(result_mult.z, FixedPoint10(15));
    EXPECT_EQ(result_mult.w, FixedPoint10(20));

    EXPECT_EQ(result_div.x, FixedPoint10(5));
    EXPECT_EQ(result_div.y, FixedPoint10(10));
    EXPECT_EQ(result_div.z, FixedPoint10(15));
    EXPECT_EQ(result_div.w, FixedPoint10(20));
}

TEST(TestHydrolibMath, QuaternionComplexMultiplication)
{
    Quaternion<FixedPoint10> q1(FixedPoint10(1), FixedPoint10(2),
                                FixedPoint10(3), FixedPoint10(4));
    Quaternion<FixedPoint10> q2(FixedPoint10(5), FixedPoint10(6),
                                FixedPoint10(7), FixedPoint10(8));
    Quaternion<FixedPoint10> result = q1 * q2;

    EXPECT_EQ(result.x, FixedPoint10(24));
    EXPECT_EQ(result.y, FixedPoint10(48));
    EXPECT_EQ(result.z, FixedPoint10(48));
    EXPECT_EQ(result.w, FixedPoint10(-6));
}
