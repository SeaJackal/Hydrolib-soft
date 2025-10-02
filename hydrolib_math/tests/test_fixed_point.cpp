#include "hydrolib_fixed_point.hpp"

#include <gtest/gtest.h>

using namespace hydrolib::math;

TEST(TestHydrolibMath, FixedPoint10Constructor)
{
    FixedPoint10 fp(5.0);

    EXPECT_DOUBLE_EQ(static_cast<double>(fp), 5.0);
}

TEST(TestHydrolibMath, FixedPoint10ConstructorWithDivider)
{
    FixedPoint10 fp(5, 2);

    EXPECT_DOUBLE_EQ(static_cast<double>(fp), 2.5);
}

TEST(TestHydrolibMath, FixedPoint10ConstructorWithLiteral)
{
    auto fp = 3.5_fp;

    EXPECT_DOUBLE_EQ(static_cast<double>(fp), 3.5);
}

TEST(TestHydrolibMath, FixedPoint10Addition)
{
    FixedPoint10 a(10);
    FixedPoint10 b(5);
    FixedPoint10 result = a + b;

    EXPECT_DOUBLE_EQ(static_cast<double>(result), 15.0);
}

TEST(TestHydrolibMath, FixedPoint10AdditionAssignment)
{
    FixedPoint10 a(10);
    FixedPoint10 b(5);
    a += b;

    EXPECT_DOUBLE_EQ(static_cast<double>(a), 15.0);
}

TEST(TestHydrolibMath, FixedPoint10Subtraction)
{
    FixedPoint10 a(10);
    FixedPoint10 b(3);
    FixedPoint10 result = a - b;

    EXPECT_DOUBLE_EQ(static_cast<double>(result), 7.0);
}

TEST(TestHydrolibMath, FixedPoint10SubtractionWithNegative)
{
    FixedPoint10 a(5);
    FixedPoint10 b(-3);
    FixedPoint10 result = a - b;

    EXPECT_DOUBLE_EQ(static_cast<double>(result), 8.0);
}

TEST(TestHydrolibMath, FixedPoint10SubtractionAssignment)
{
    FixedPoint10 a(10);
    FixedPoint10 b(3);
    a -= b;

    EXPECT_DOUBLE_EQ(static_cast<double>(a), 7.0);
}

TEST(TestHydrolibMath, FixedPoint10UnaryMinus)
{
    FixedPoint10 a(10);
    FixedPoint10 b(-5);

    EXPECT_DOUBLE_EQ(static_cast<double>(-a), -10.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(-b), 5.0);
}

TEST(TestHydrolibMath, FixedPoint10Multiplication)
{
    FixedPoint10 a(4);
    FixedPoint10 b(3);
    FixedPoint10 result = a * b;

    EXPECT_DOUBLE_EQ(static_cast<double>(result), 12.0);
}

TEST(TestHydrolibMath, FixedPoint10MultiplicationAssignment)
{
    FixedPoint10 a(6);
    FixedPoint10 b(2);
    a *= b;

    EXPECT_DOUBLE_EQ(static_cast<double>(a), 12.0);
}

TEST(TestHydrolibMath, FixedPoint10Division)
{
    FixedPoint10 a(15);
    FixedPoint10 b(3);
    FixedPoint10 result = a / b;

    EXPECT_DOUBLE_EQ(static_cast<double>(result), 5.0);
}

TEST(TestHydrolibMath, FixedPoint10DivisionAssignment)
{
    FixedPoint10 a(20);
    FixedPoint10 b(4);
    a /= b;

    EXPECT_DOUBLE_EQ(static_cast<double>(a), 5.0);
}

TEST(TestHydrolibMath, FixedPoint10ChainOperations)
{
    FixedPoint10 a(10);
    FixedPoint10 b(5);
    FixedPoint10 c(2);

    FixedPoint10 result = (a + b) * c;
    EXPECT_DOUBLE_EQ(static_cast<double>(result), 30.0);

    FixedPoint10 result2 = a * b - c;
    EXPECT_DOUBLE_EQ(static_cast<double>(result2), 48.0);
}

TEST(TestHydrolibMath, FixedPoint10LargeNumbers)
{
    FixedPoint10 large1(1000);
    FixedPoint10 large2(500);

    EXPECT_DOUBLE_EQ(static_cast<double>(large1 + large2), 1500.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(large1 - large2), 500.0);
    EXPECT_DOUBLE_EQ(static_cast<double>(large1 / large2), 2.0);
}

TEST(TestHydrolibMath, FixedPoint10SmallDecimals)
{
    auto small1 = 0.125_fp;
    auto small2 = 0.25_fp;

    EXPECT_DOUBLE_EQ(static_cast<double>(small1 + small1), 0.25);
    EXPECT_DOUBLE_EQ(static_cast<double>(small2 - small1), 0.125);
    EXPECT_DOUBLE_EQ(static_cast<double>(small1 * 8.0_fp), 1.0);
}

TEST(TestHydrolibMath, FixedPoint10Sqrt)
{
    FixedPoint10 a(16);
    FixedPoint10 result = sqrt(a);

    EXPECT_DOUBLE_EQ(static_cast<double>(result), 4.0);

    FixedPoint10 b(9);
    FixedPoint10 result2 = sqrt(b);

    EXPECT_DOUBLE_EQ(static_cast<double>(result2), 3.0);

    auto c = 6.25_fp;
    FixedPoint10 result3 = sqrt(c);

    EXPECT_DOUBLE_EQ(static_cast<double>(result3), 2.5);

    auto d = 0.25_fp;
    FixedPoint10 result4 = sqrt(d);

    EXPECT_DOUBLE_EQ(static_cast<double>(result4), 0.5);
}

TEST(TestHydrolibMath, FixedPoint10Sin)
{
    constexpr double rads = 1.5708;
    FixedPoint10 a(rads);
    FixedPoint10 result = sin(a);

    EXPECT_DOUBLE_EQ(static_cast<double>(result), sin(rads));
}

TEST(TestHydrolibMath, FixedPoint10EdgeCases)
{
    auto tiny = 0.001_fp;
    auto large = 1023.0_fp;

    EXPECT_EQ(tiny + tiny, 0.002_fp);
    EXPECT_EQ(large / 2.0_fp, 511.5_fp);
}

TEST(TestHydrolibMath, FixedPoint10GetFractionBits)
{
    FixedPoint10 fp(5.5);
    EXPECT_EQ(FixedPoint10::GetFractionBits(), 10);

    FixedPoint10 fp2(0.0);
    EXPECT_EQ(FixedPoint10::GetFractionBits(), 10);

    FixedPoint10 fp3(-3.14);
    EXPECT_EQ(FixedPoint10::GetFractionBits(), 10);
}

TEST(TestHydrolibMath, FixedPoint10GetIntPart)
{
    FixedPoint10 fp1(5.75);
    EXPECT_EQ(fp1.GetIntPart(), 5);

    FixedPoint10 fp2(10.25);
    EXPECT_EQ(fp2.GetIntPart(), 10);

    FixedPoint10 fp3(0.99);
    EXPECT_EQ(fp3.GetIntPart(), 0);

    FixedPoint10 fp4(-5.75);
    EXPECT_EQ(fp4.GetIntPart(), -6);

    FixedPoint10 fp5(-0.25);
    EXPECT_EQ(fp5.GetIntPart(), -1);

    FixedPoint10 fp6(42);
    EXPECT_EQ(fp6.GetIntPart(), 42);

    FixedPoint10 fp7(-15);
    EXPECT_EQ(fp7.GetIntPart(), -15);

    FixedPoint10 fp8(0);
    EXPECT_EQ(fp8.GetIntPart(), 0);
}

TEST(TestHydrolibMath, FixedPoint10GetFractionPart)
{
    FixedPoint10 fp1(5.25);
    EXPECT_EQ(fp1.GetFractionPart(),
              0.25 * (1 << FixedPoint10::GetFractionBits()));

    FixedPoint10 fp2(3.5);
    EXPECT_EQ(fp2.GetFractionPart(),
              0.5 * (1 << FixedPoint10::GetFractionBits()));

    FixedPoint10 fp3(7.75);
    EXPECT_EQ(fp3.GetFractionPart(),
              0.75 * (1 << FixedPoint10::GetFractionBits()));

    FixedPoint10 fp4(10);
    EXPECT_EQ(fp4.GetFractionPart(), 0);

    FixedPoint10 fp5(0);
    EXPECT_EQ(fp5.GetFractionPart(), 0);
}

TEST(TestHydrolibMath, FixedPoint10GetFractionPartNegative)
{
    FixedPoint10 fp6(-2.25);
    EXPECT_EQ(fp6.GetFractionPart(),
              0.25 * (1 << FixedPoint10::GetFractionBits()));
}
