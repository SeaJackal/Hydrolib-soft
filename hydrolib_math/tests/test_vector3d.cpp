#include "hydrolib_fixed_point.hpp"
#include "hydrolib_vector3d.hpp"

#include <gtest/gtest.h>

using namespace hydrolib::math;

// Basic Vector3D construction and initialization tests
TEST(TestHydrolibMathVector3D, FixedPointBaseConstruction)
{
    Vector3D<FixedPointBase> vec{FixedPointBase(1), FixedPointBase(2),
                               FixedPointBase(3)};

    EXPECT_DOUBLE_EQ(static_cast<double>(vec.x), 1.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(vec.y), 2.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(vec.z), 3.0);
}

// Dot product tests
TEST(TestHydrolibMathVector3D, DotProductOrthogonalVectors)
{
    Vector3D<FixedPointBase> vec1{1.0_fp, 0.0_fp, 0.0_fp};
    Vector3D<FixedPointBase> vec2{0.0_fp, 1.0_fp, 0.0_fp};

    FixedPointBase result = vec1.Dot(vec2);

    EXPECT_DOUBLE_EQ(static_cast<double>(result), 0.0);
}

TEST(TestHydrolibMathVector3D, DotProductParallelVectors)
{
    Vector3D<FixedPointBase> vec1{3.0_fp, 4.0_fp, 5.0_fp};
    Vector3D<FixedPointBase> vec2{3.0_fp, 4.0_fp, 5.0_fp};

    FixedPointBase result = vec1.Dot(vec2);

    // 3*3 + 4*4 + 5*5 = 9 + 16 + 25 = 50
    EXPECT_DOUBLE_EQ(static_cast<double>(result), 50.0);
}

TEST(TestHydrolibMathVector3D, DotProductGeneralCase)
{
    Vector3D<FixedPointBase> vec1{1.0_fp, 2.0_fp, 3.0_fp};
    Vector3D<FixedPointBase> vec2{4.0_fp, 5.0_fp, 6.0_fp};

    FixedPointBase result = vec1.Dot(vec2);

    // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
    EXPECT_DOUBLE_EQ(static_cast<double>(result), 32.0);
}

TEST(TestHydrolibMathVector3D, DotProductWithNegativeValues)
{
    Vector3D<FixedPointBase> vec1{1.0_fp, -2.0_fp, 3.0_fp};
    Vector3D<FixedPointBase> vec2{-1.0_fp, 2.0_fp, 1.0_fp};

    FixedPointBase result = vec1.Dot(vec2);

    // 1*(-1) + (-2)*2 + 3*1 = -1 - 4 + 3 = -2
    EXPECT_DOUBLE_EQ(static_cast<double>(result), -2.0);
}

// Cross product tests
TEST(TestHydrolibMathVector3D, CrossProductUnitVectors)
{
    Vector3D<FixedPointBase> vec1{1.0_fp, 0.0_fp, 0.0_fp}; // i
    Vector3D<FixedPointBase> vec2{0.0_fp, 1.0_fp, 0.0_fp}; // j

    Vector3D<FixedPointBase> result = vec1.Cross(vec2);

    // i × j = k
    EXPECT_DOUBLE_EQ(static_cast<double>(result.x), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.y), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.z), 1.0);
}

TEST(TestHydrolibMathVector3D, CrossProductGeneralCase)
{
    Vector3D<FixedPointBase> vec1{1.0_fp, 2.0_fp, 3.0_fp};
    Vector3D<FixedPointBase> vec2{4.0_fp, 5.0_fp, 6.0_fp};

    Vector3D<FixedPointBase> result = vec1.Cross(vec2);

    // (2*6 - 3*5, 3*4 - 1*6, 1*5 - 2*4) = (12 - 15, 12 - 6, 5 - 8) = (-3, 6,
    // -3)
    EXPECT_DOUBLE_EQ(static_cast<double>(result.x), -3.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.y), 6.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.z), -3.0);
}

TEST(TestHydrolibMathVector3D, CrossProductParallelVectors)
{
    Vector3D<FixedPointBase> vec1{2.0_fp, 4.0_fp, 6.0_fp};
    Vector3D<FixedPointBase> vec2{1.0_fp, 2.0_fp, 3.0_fp}; // vec2 = vec1 / 2

    Vector3D<FixedPointBase> result = vec1.Cross(vec2);

    // Cross product of parallel vectors should be zero vector
    EXPECT_DOUBLE_EQ(static_cast<double>(result.x), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.y), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.z), 0.0);
}

TEST(TestHydrolibMathVector3D, LengthCalculation)
{
    Vector3D<FixedPointBase> vec{3.0_fp, 4.0_fp, 0.0_fp};

    FixedPointBase length = vec.Length();

    EXPECT_DOUBLE_EQ(static_cast<double>(length), 5.0);
}

TEST(TestHydrolibMathVector3D, Length3DVector)
{
    Vector3D<FixedPointBase> vec{1.0_fp, 2.0_fp, 2.0_fp};

    FixedPointBase length = vec.Length();

    EXPECT_DOUBLE_EQ(static_cast<double>(length), 3.0);
}

TEST(TestHydrolibMathVector3D, ScalarMultiplicationRight)
{
    Vector3D<FixedPointBase> vec{1.0_fp, 2.0_fp, 3.0_fp};
    FixedPointBase scalar(2);

    Vector3D<FixedPointBase> result = vec * scalar;

    EXPECT_DOUBLE_EQ(static_cast<double>(result.x), 2.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.y), 4.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.z), 6.0);
}

TEST(TestHydrolibMathVector3D, ScalarMultiplicationAssignment)
{
    Vector3D<FixedPointBase> vec{2.0_fp, 4.0_fp, 6.0_fp};
    FixedPointBase scalar(0.5);

    vec *= scalar;

    EXPECT_DOUBLE_EQ(static_cast<double>(vec.x), 1.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(vec.y), 2.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(vec.z), 3.0);
}

TEST(TestHydrolibMathVector3D, ScalarMultiplicationZero)
{
    Vector3D<FixedPointBase> vec{5.0_fp, 10.0_fp, 15.0_fp};
    FixedPointBase zero(0);

    Vector3D<FixedPointBase> result = vec * zero;

    EXPECT_DOUBLE_EQ(static_cast<double>(result.x), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.y), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.z), 0.0);
}

TEST(TestHydrolibMathVector3D, ScalarMultiplicationNegative)
{
    Vector3D<FixedPointBase> vec{1.0_fp, -2.0_fp, 3.0_fp};
    FixedPointBase scalar(-2);

    Vector3D<FixedPointBase> result = vec * scalar;

    EXPECT_DOUBLE_EQ(static_cast<double>(result.x), -2.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.y), 4.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.z), -6.0);
}

TEST(TestHydrolibMathVector3D, ScalarDivision)
{
    Vector3D<FixedPointBase> vec{6.0_fp, 8.0_fp, 10.0_fp};
    FixedPointBase divisor(2);

    Vector3D<FixedPointBase> result = vec / divisor;

    EXPECT_DOUBLE_EQ(static_cast<double>(result.x), 3.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.y), 4.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.z), 5.0);
}

TEST(TestHydrolibMathVector3D, ScalarDivisionAssignment)
{
    Vector3D<FixedPointBase> vec{9.0_fp, 12.0_fp, 15.0_fp};
    FixedPointBase divisor(3);

    vec /= divisor;

    EXPECT_DOUBLE_EQ(static_cast<double>(vec.x), 3.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(vec.y), 4.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(vec.z), 5.0);
}

TEST(TestHydrolibMathVector3D, ScalarDivisionDecimal)
{
    Vector3D<FixedPointBase> vec{1.0_fp, 2.0_fp, 3.0_fp};
    FixedPointBase divisor(0.5);

    Vector3D<FixedPointBase> result = vec / divisor;

    EXPECT_DOUBLE_EQ(static_cast<double>(result.x), 2.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.y), 4.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.z), 6.0);
}

TEST(TestHydrolibMathVector3D, ScalarOperationsChaining)
{
    Vector3D<FixedPointBase> vec{1.0_fp, 2.0_fp, 3.0_fp};
    FixedPointBase scalar1(2);
    FixedPointBase scalar2(3);

    Vector3D<FixedPointBase> result = (vec * scalar1) * scalar2;

    EXPECT_DOUBLE_EQ(static_cast<double>(result.x), 6.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.y), 12.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.z), 18.0);
}

TEST(TestHydrolibMathVector3D, ScalarMultiplicationDivisionInverse)
{
    Vector3D<FixedPointBase> original{2.5_fp, 5.0_fp, 7.5_fp};
    FixedPointBase scalar(4);

    Vector3D<FixedPointBase> multiplied = original * scalar;
    Vector3D<FixedPointBase> result = multiplied / scalar;

    EXPECT_NEAR(static_cast<double>(result.x), static_cast<double>(original.x),
                0.01);
    EXPECT_NEAR(static_cast<double>(result.y), static_cast<double>(original.y),
                0.01);
    EXPECT_NEAR(static_cast<double>(result.z), static_cast<double>(original.z),
                0.01);
}

TEST(TestHydrolibMathVector3D, ZeroVectorDotProduct)
{
    Vector3D<FixedPointBase> zero_vec{0.0_fp, 0.0_fp, 0.0_fp};
    Vector3D<FixedPointBase> any_vec{5.0_fp, 10.0_fp, 15.0_fp};

    FixedPointBase result = zero_vec.Dot(any_vec);

    EXPECT_DOUBLE_EQ(static_cast<double>(result), 0.0);
}

TEST(TestHydrolibMathVector3D, ZeroVectorCrossProduct)
{
    Vector3D<FixedPointBase> zero_vec{0.0_fp, 0.0_fp, 0.0_fp};
    Vector3D<FixedPointBase> any_vec{5.0_fp, 10.0_fp, 15.0_fp};

    Vector3D<FixedPointBase> result = zero_vec.Cross(any_vec);

    EXPECT_DOUBLE_EQ(static_cast<double>(result.x), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.y), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.z), 0.0);
}

TEST(TestHydrolibMathVector3D, SelfCrossProduct)
{
    Vector3D<FixedPointBase> vec{1.0_fp, 2.0_fp, 3.0_fp};

    Vector3D<FixedPointBase> result = vec.Cross(vec);

    // Any vector crossed with itself should be zero vector
    EXPECT_DOUBLE_EQ(static_cast<double>(result.x), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.y), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.z), 0.0);
}

TEST(TestHydrolibMathVector3D, SelfDotProduct)
{
    Vector3D<FixedPointBase> vec{3.0_fp, 4.0_fp, 0.0_fp};

    FixedPointBase result = vec.Dot(vec);

    // 3^2 + 4^2 + 0^2 = 9 + 16 + 0 = 25
    EXPECT_DOUBLE_EQ(static_cast<double>(result), 25.0);
}