#include "hydrolib_vector3d.hpp"
#include "hydrolib_fixed_point.hpp"

#include <gtest/gtest.h>

using namespace hydrolib::math;

// Basic Vector3D construction and initialization tests
TEST(TestHydrolibMathVector3D, FixedPoint10Construction)
{
    Vector3D<FixedPoint10> vec{FixedPoint10(1), FixedPoint10(2), FixedPoint10(3)};
    
    EXPECT_DOUBLE_EQ(static_cast<double>(vec.x), 1.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(vec.y), 2.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(vec.z), 3.0);
}

// Dot product tests
TEST(TestHydrolibMathVector3D, DotProductOrthogonalVectors)
{
    Vector3D<FixedPoint10> vec1{1.0_fp, 0.0_fp, 0.0_fp};
    Vector3D<FixedPoint10> vec2{0.0_fp, 1.0_fp, 0.0_fp};
    
    FixedPoint10 result = vec1.Dot(vec2);
    
    EXPECT_DOUBLE_EQ(static_cast<double>(result), 0.0);
}

TEST(TestHydrolibMathVector3D, DotProductParallelVectors)
{
    Vector3D<FixedPoint10> vec1{3.0_fp, 4.0_fp, 5.0_fp};
    Vector3D<FixedPoint10> vec2{3.0_fp, 4.0_fp, 5.0_fp};
    
    FixedPoint10 result = vec1.Dot(vec2);
    
    // 3*3 + 4*4 + 5*5 = 9 + 16 + 25 = 50
    EXPECT_DOUBLE_EQ(static_cast<double>(result), 50.0);
}

TEST(TestHydrolibMathVector3D, DotProductGeneralCase)
{
    Vector3D<FixedPoint10> vec1{1.0_fp, 2.0_fp, 3.0_fp};
    Vector3D<FixedPoint10> vec2{4.0_fp, 5.0_fp, 6.0_fp};
    
    FixedPoint10 result = vec1.Dot(vec2);
    
    // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
    EXPECT_DOUBLE_EQ(static_cast<double>(result), 32.0);
}

TEST(TestHydrolibMathVector3D, DotProductWithDecimals)
{
    Vector3D<FixedPoint10> vec1{1.5_fp, 2.5_fp, 3.5_fp};
    Vector3D<FixedPoint10> vec2{0.5_fp, 1.5_fp, 2.5_fp};
    
    FixedPoint10 result = vec1.Dot(vec2);
    
    // 1.5*0.5 + 2.5*1.5 + 3.5*2.5 = 0.75 + 3.75 + 8.75 = 13.25
    EXPECT_DOUBLE_EQ(static_cast<double>(result), 13.25);
}

TEST(TestHydrolibMathVector3D, DotProductWithNegativeValues)
{
    Vector3D<FixedPoint10> vec1{1.0_fp, -2.0_fp, 3.0_fp};
    Vector3D<FixedPoint10> vec2{-1.0_fp, 2.0_fp, 1.0_fp};
    
    FixedPoint10 result = vec1.Dot(vec2);
    
    // 1*(-1) + (-2)*2 + 3*1 = -1 - 4 + 3 = -2
    EXPECT_DOUBLE_EQ(static_cast<double>(result), -2.0);
}

// Cross product tests
TEST(TestHydrolibMathVector3D, CrossProductUnitVectors)
{
    Vector3D<FixedPoint10> vec1{1.0_fp, 0.0_fp, 0.0_fp};  // i
    Vector3D<FixedPoint10> vec2{0.0_fp, 1.0_fp, 0.0_fp};  // j
    
    Vector3D<FixedPoint10> result = vec1.Cross(vec2);
    
    // i × j = k
    EXPECT_DOUBLE_EQ(static_cast<double>(result.x), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.y), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.z), 1.0);
}

TEST(TestHydrolibMathVector3D, CrossProductGeneralCase)
{
    Vector3D<FixedPoint10> vec1{1.0_fp, 2.0_fp, 3.0_fp};
    Vector3D<FixedPoint10> vec2{4.0_fp, 5.0_fp, 6.0_fp};
    
    Vector3D<FixedPoint10> result = vec1.Cross(vec2);
    
    // (2*6 - 3*5, 3*4 - 1*6, 1*5 - 2*4) = (12 - 15, 12 - 6, 5 - 8) = (-3, 6, -3)
    EXPECT_DOUBLE_EQ(static_cast<double>(result.x), -3.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.y), 6.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.z), -3.0);
}

TEST(TestHydrolibMathVector3D, CrossProductParallelVectors)
{
    Vector3D<FixedPoint10> vec1{2.0_fp, 4.0_fp, 6.0_fp};
    Vector3D<FixedPoint10> vec2{1.0_fp, 2.0_fp, 3.0_fp}; // vec2 = vec1 / 2
    
    Vector3D<FixedPoint10> result = vec1.Cross(vec2);
    
    // Cross product of parallel vectors should be zero vector
    EXPECT_DOUBLE_EQ(static_cast<double>(result.x), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.y), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.z), 0.0);
}

TEST(TestHydrolibMathVector3D, CrossProductAntiCommutative)
{
    Vector3D<FixedPoint10> vec1{1.0_fp, 2.0_fp, 3.0_fp};
    Vector3D<FixedPoint10> vec2{4.0_fp, 5.0_fp, 6.0_fp};
    
    Vector3D<FixedPoint10> result1 = vec1.Cross(vec2);
    Vector3D<FixedPoint10> result2 = vec2.Cross(vec1);
    
    // vec1 × vec2 = -(vec2 × vec1)
    EXPECT_DOUBLE_EQ(static_cast<double>(result1.x), -static_cast<double>(result2.x));
    EXPECT_DOUBLE_EQ(static_cast<double>(result1.y), -static_cast<double>(result2.y));
    EXPECT_DOUBLE_EQ(static_cast<double>(result1.z), -static_cast<double>(result2.z));
}

TEST(TestHydrolibMathVector3D, CrossProductWithDecimals)
{
    Vector3D<FixedPoint10> vec1{1.5_fp, 2.5_fp, 0.5_fp};
    Vector3D<FixedPoint10> vec2{0.5_fp, 1.0_fp, 2.0_fp};
    
    Vector3D<FixedPoint10> result = vec1.Cross(vec2);
    
    // (2.5*2.0 - 0.5*1.0, 0.5*0.5 - 1.5*2.0, 1.5*1.0 - 2.5*0.5) = (5.0 - 0.5, 0.25 - 3.0, 1.5 - 1.25) = (4.5, -2.75, 0.25)
    EXPECT_DOUBLE_EQ(static_cast<double>(result.x), 4.5);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.y), -2.75);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.z), 0.25);
}

// Edge cases and special values
TEST(TestHydrolibMathVector3D, ZeroVectorDotProduct)
{
    Vector3D<FixedPoint10> zero_vec{0.0_fp, 0.0_fp, 0.0_fp};
    Vector3D<FixedPoint10> any_vec{5.0_fp, 10.0_fp, 15.0_fp};
    
    FixedPoint10 result = zero_vec.Dot(any_vec);
    
    EXPECT_DOUBLE_EQ(static_cast<double>(result), 0.0);
}

TEST(TestHydrolibMathVector3D, ZeroVectorCrossProduct)
{
    Vector3D<FixedPoint10> zero_vec{0.0_fp, 0.0_fp, 0.0_fp};
    Vector3D<FixedPoint10> any_vec{5.0_fp, 10.0_fp, 15.0_fp};
    
    Vector3D<FixedPoint10> result = zero_vec.Cross(any_vec);
    
    EXPECT_DOUBLE_EQ(static_cast<double>(result.x), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.y), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.z), 0.0);
}

TEST(TestHydrolibMathVector3D, SelfCrossProduct)
{
    Vector3D<FixedPoint10> vec{1.0_fp, 2.0_fp, 3.0_fp};
    
    Vector3D<FixedPoint10> result = vec.Cross(vec);
    
    // Any vector crossed with itself should be zero vector
    EXPECT_DOUBLE_EQ(static_cast<double>(result.x), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.y), 0.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(result.z), 0.0);
}

TEST(TestHydrolibMathVector3D, SelfDotProduct)
{
    Vector3D<FixedPoint10> vec{3.0_fp, 4.0_fp, 0.0_fp};
    
    FixedPoint10 result = vec.Dot(vec);
    
    // 3^2 + 4^2 + 0^2 = 9 + 16 + 0 = 25
    EXPECT_DOUBLE_EQ(static_cast<double>(result), 25.0);
}

TEST(TestHydrolibMathVector3D, SmallDecimalValues)
{
    Vector3D<FixedPoint10> vec1{0.125_fp, 0.25_fp, 0.5_fp};
    Vector3D<FixedPoint10> vec2{0.5_fp, 0.25_fp, 0.125_fp};
    
    FixedPoint10 dot_result = vec1.Dot(vec2);
    Vector3D<FixedPoint10> cross_result = vec1.Cross(vec2);
    
    // Dot: 0.125*0.5 + 0.25*0.25 + 0.5*0.125 = 0.0625 + 0.0625 + 0.0625 = 0.1875
    EXPECT_DOUBLE_EQ(static_cast<double>(dot_result), 0.1875);
    
    // Cross: (0.25*0.125 - 0.5*0.25, 0.5*0.5 - 0.125*0.125, 0.125*0.25 - 0.25*0.5)
    //      = (0.03125 - 0.125, 0.25 - 0.015625, 0.03125 - 0.125) = (-0.09375, 0.234375, -0.09375)
    EXPECT_DOUBLE_EQ(static_cast<double>(cross_result.x), -0.09375);
    EXPECT_DOUBLE_EQ(static_cast<double>(cross_result.y), 0.234375);
    EXPECT_DOUBLE_EQ(static_cast<double>(cross_result.z), -0.09375);
}
