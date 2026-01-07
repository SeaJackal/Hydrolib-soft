#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>

#include <climits>
#include <cmath>
#include <tuple>

#include "hydrolib_fixed_point.hpp"

using namespace hydrolib::math;

TEST(TestHydrolibMath, FixedPointDefaultConstructor) {
  FixedPointBase fixed_point{};

  EXPECT_DOUBLE_EQ(static_cast<double>(fixed_point), 0);
}

class TestFixedPointConstructorInt : public ::testing::Test,
                                     public ::testing::WithParamInterface<int> {
};

INSTANTIATE_TEST_CASE_P(
    Test, TestFixedPointConstructorInt,
    ::testing::Values(0, FixedPointBase::kUpperNotIncludedBound - 1,
                      FixedPointBase::kLowerIncludedBound, 1, -1, 5, 7, 32, -15,
                      -192, 134));

TEST_P(TestFixedPointConstructorInt, Basic) {
  auto value = GetParam();
  FixedPointBase fixed_point(value);

  EXPECT_DOUBLE_EQ(static_cast<double>(fixed_point), value);
}

class TestFixedPointConstructorIntWithDivider
    : public ::testing::Test,
      public ::testing::WithParamInterface<std::tuple<int, int>> {};

INSTANTIATE_TEST_CASE_P(
    Test, TestFixedPointConstructorIntWithDivider,
    ::testing::Values(
        std::tuple<int, int>{0, 2}, std::tuple<int, int>{1, 1},
        std::tuple<int, int>{-1, 1},
        std::tuple<int, int>{INT_MAX, 1 << FixedPointBase::kFractionBits},
        std::tuple<int, int>{INT_MIN, 1 << FixedPointBase::kFractionBits},
        std::tuple<int, int>{11, 5}, std::tuple<int, int>{-17, 8},
        std::tuple<int, int>{12, 3}, std::tuple<int, int>{-196, 3},
        std::tuple<int, int>{346, 9}));

TEST_P(TestFixedPointConstructorIntWithDivider, Basic) {
  auto values = GetParam();
  auto number = std::get<0>(values);
  auto divider = std::get<1>(values);
  FixedPointBase fixed_point(number, divider);

  EXPECT_NEAR(static_cast<double>(fixed_point),
              static_cast<double>(number) / divider,
              FixedPointBase::kLeastBitValue);
}

struct DoubleConstructorTestCase {
  consteval DoubleConstructorTestCase(double value)  // NOLINT
      : value(value), fixed_point(value) {}

  double value = 0;            // NOLINT
  FixedPointBase fixed_point;  // NOLINT
};

constexpr std::array<DoubleConstructorTestCase, 21>
    kDoubleConstructorTestCases = {
        0,
        1,
        -1,
        FixedPointBase::kUpperNotIncludedBound - FixedPointBase::kLeastBitValue,
        FixedPointBase::kLowerIncludedBound,
        FixedPointBase::kLowerIncludedBound + FixedPointBase::kLeastBitValue,
        FixedPointBase::kLeastBitValue,
        -FixedPointBase::kLeastBitValue,
        3.5,
        -3.5,
        1.25,
        -1.25,
        0.75,
        -0.75,
        0.25,
        -0.25,
        0.77,
        -0.77,
        333,
        -123,
        189.013};

class TestFixedPointConstructorDouble
    : public ::testing::Test,
      public ::testing::WithParamInterface<DoubleConstructorTestCase> {};

INSTANTIATE_TEST_CASE_P(Test, TestFixedPointConstructorDouble,
                        ::testing::ValuesIn(kDoubleConstructorTestCases));

TEST_P(TestFixedPointConstructorDouble, Basic) {
  auto test_case = GetParam();
  EXPECT_NEAR(static_cast<double>(test_case.fixed_point), test_case.value,
              FixedPointBase::kLeastBitValue);
}

TEST(TestHydrolibMath, FixedPointBaseConstructorWithLiteral) {
  constexpr auto fixed_point = 3.5_fp;

  EXPECT_DOUBLE_EQ(static_cast<double>(fixed_point), 3.5);
}

constexpr std::array<FixedPointBase, 10> kFixedPointCases = {
    0,
    1,
    -1,
    FixedPointBase::kUpperNotIncludedBound - FixedPointBase::kLeastBitValue,
    FixedPointBase::kLowerIncludedBound,
    FixedPointBase::kLowerIncludedBound + FixedPointBase::kLeastBitValue,
    FixedPointBase::kLeastBitValue,
    -FixedPointBase::kLeastBitValue,
    2.7568,
    -7.891};

class TestFixedPointBinaryOperations
    : public ::testing::Test,
      public ::testing::WithParamInterface<
          std::tuple<FixedPointBase, FixedPointBase>> {};

INSTANTIATE_TEST_CASE_P(
    Test, TestFixedPointBinaryOperations,
    ::testing::Combine(::testing::ValuesIn(kFixedPointCases),
                       ::testing::ValuesIn(kFixedPointCases)),
    [](const testing::TestParamInfo<TestFixedPointBinaryOperations::ParamType>
           &info) {
      static const auto sanitize = [](const std::string &input) -> std::string {
        std::string out;
        out.reserve(input.size() * 5);
        for (char chr : input) {
          if (chr == '.') {
            out += "point";
          } else if (chr == '-') {
            out += "minus";
          } else {
            out += chr;
          }
        }
        return out;
      };
      auto values = info.param;
      auto first = std::get<0>(values);
      auto second = std::get<1>(values);
      return sanitize(std::to_string(static_cast<double>(first))) + "_" +
             sanitize(std::to_string(static_cast<double>(second)));
    });

TEST_P(TestFixedPointBinaryOperations, Addition) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);

  double expected = static_cast<double>(first) + static_cast<double>(second);
  if (expected < FixedPointBase::kUpperNotIncludedBound &&
      expected >= FixedPointBase::kLowerIncludedBound) {
    FixedPointBase result = first + second;
    EXPECT_DOUBLE_EQ(static_cast<double>(result),
                     static_cast<double>(first) + static_cast<double>(second));
  }
}

TEST_P(TestFixedPointBinaryOperations, AdditionAssignment) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);

  double expected = static_cast<double>(first) + static_cast<double>(second);
  if (expected < FixedPointBase::kUpperNotIncludedBound &&
      expected >= FixedPointBase::kLowerIncludedBound) {
    first += second;
    EXPECT_DOUBLE_EQ(static_cast<double>(first), expected);
  }
}

TEST_P(TestFixedPointBinaryOperations, Subtraction) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);

  double expected = static_cast<double>(first) - static_cast<double>(second);
  if (expected < FixedPointBase::kUpperNotIncludedBound &&
      expected >= FixedPointBase::kLowerIncludedBound) {
    FixedPointBase result = first - second;
    EXPECT_DOUBLE_EQ(static_cast<double>(result), expected);
  }
}

TEST_P(TestFixedPointBinaryOperations, SubtractionAssignment) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);

  double expected = static_cast<double>(first) - static_cast<double>(second);
  if (expected < FixedPointBase::kUpperNotIncludedBound &&
      expected >= FixedPointBase::kLowerIncludedBound) {
    first -= second;
    EXPECT_DOUBLE_EQ(static_cast<double>(first), expected);
  }
}

TEST_P(TestFixedPointBinaryOperations, Multiplication) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);

  double expected = static_cast<double>(first) * static_cast<double>(second);
  if (expected < FixedPointBase::kUpperNotIncludedBound &&
      expected >= FixedPointBase::kLowerIncludedBound) {
    auto result = first * second;
    EXPECT_NEAR(static_cast<double>(result), expected,
                FixedPointBase::kLeastBitValue);
  }
}

TEST_P(TestFixedPointBinaryOperations, MultiplicationAssignment) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);

  double expected = static_cast<double>(first) * static_cast<double>(second);
  if (expected < FixedPointBase::kUpperNotIncludedBound &&
      expected >= FixedPointBase::kLowerIncludedBound) {
    first *= second;
    EXPECT_NEAR(static_cast<double>(first), expected,
                FixedPointBase::kLeastBitValue);
  }
}

TEST_P(TestFixedPointBinaryOperations, Division) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);

  if (second != 0) {
    double expected = static_cast<double>(first) / static_cast<double>(second);
    if (expected < FixedPointBase::kUpperNotIncludedBound &&
        expected >= FixedPointBase::kLowerIncludedBound) {
      auto result = first / second;
      EXPECT_NEAR(static_cast<double>(result), expected,
                  FixedPointBase::kLeastBitValue);
    }
  }
}

TEST_P(TestFixedPointBinaryOperations, DivisionAssignment) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);

  if (second != 0) {
    double expected = static_cast<double>(first) / static_cast<double>(second);
    if (expected < FixedPointBase::kUpperNotIncludedBound &&
        expected >= FixedPointBase::kLowerIncludedBound) {
      first /= second;
      EXPECT_NEAR(static_cast<double>(first), expected,
                  FixedPointBase::kLeastBitValue);
    }
  }
}

TEST(TestHydrolibMath, FixedPointBaseChainOperations) {
  FixedPointBase a(10);
  FixedPointBase b(5);
  FixedPointBase c(2);

  FixedPointBase result = (a + b) * c;
  EXPECT_DOUBLE_EQ(static_cast<double>(result), 30.0);

  FixedPointBase result2 = a * b - c;
  EXPECT_DOUBLE_EQ(static_cast<double>(result2), 48.0);
}

TEST_P(TestFixedPointBinaryOperations, Equal) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);
  if (static_cast<double>(first) == static_cast<double>(second)) {
    EXPECT_TRUE(first == second);
  } else {
    EXPECT_FALSE(first == second);
  }
}

TEST_P(TestFixedPointBinaryOperations, NonEqual) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);
  if (static_cast<double>(first) != static_cast<double>(second)) {
    EXPECT_TRUE(first != second);
  } else {
    EXPECT_FALSE(first != second);
  }
}

TEST_P(TestFixedPointBinaryOperations, Less) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);
  if (static_cast<double>(first) < static_cast<double>(second)) {
    EXPECT_TRUE(first < second);
  } else {
    EXPECT_FALSE(first < second);
  }
}

TEST_P(TestFixedPointBinaryOperations, LessOrEqual) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);
  if (static_cast<double>(first) <= static_cast<double>(second)) {
    EXPECT_TRUE(first <= second);
  } else {
    EXPECT_FALSE(first <= second);
  }
}

TEST_P(TestFixedPointBinaryOperations, Greater) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);
  if (static_cast<double>(first) > static_cast<double>(second)) {
    EXPECT_TRUE(first > second);
  } else {
    EXPECT_FALSE(first > second);
  }
}

TEST_P(TestFixedPointBinaryOperations, GreaterOrEqual) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);
  if (static_cast<double>(first) >= static_cast<double>(second)) {
    EXPECT_TRUE(first >= second);
  } else {
    EXPECT_FALSE(first >= second);
  }
}

TEST(TestHydrolibMath, FixedPointBaseSin) {
  constexpr auto rads1 = kPi / 4;
  constexpr auto rads2 = -kPi / 6;
  constexpr auto rads3 = -kPi / 3;
  FixedPointBase result1 = sin(rads1);
  FixedPointBase result2 = sin(rads2);
  FixedPointBase result3 = sin(rads3);

  EXPECT_NEAR(static_cast<double>(result1), sin(static_cast<double>(rads1)),
              0.002);
  EXPECT_NEAR(static_cast<double>(result2), sin(static_cast<double>(rads2)),
              0.002);
  EXPECT_NEAR(static_cast<double>(result3), sin(static_cast<double>(rads3)),
              0.002);
}

TEST(TestHydrolibMath, FixedPointBaseCos) {
  constexpr double rads = 3.14159265358979323846 / 4;
  constexpr double rads2 = 3.14159265358979323846 / 6;
  constexpr double rads3 = 3.14159265358979323846 / 3;
  FixedPointBase a(rads);
  FixedPointBase a2(rads2);
  FixedPointBase a3(rads3);
  FixedPointBase result = cos(a);
  FixedPointBase result2 = cos(a2);
  FixedPointBase result3 = cos(a3);
  EXPECT_NEAR(static_cast<double>(result), cos(rads), 0.002);
  EXPECT_NEAR(static_cast<double>(result2), cos(rads2), 0.002);
  EXPECT_NEAR(static_cast<double>(result3), cos(rads3), 0.002);
}

TEST(TestHydrolibMath, FixedPointBaseSinNegativeArgument) {
  constexpr double rads = -3.14159265358979323846 / 4.0;
  constexpr double rads2 = -3.14159265358979323846 / 6.0;
  constexpr double rads3 = -3.14159265358979323846 / 3.0;
  FixedPointBase a(rads);
  FixedPointBase result_sin = sin(a);
  FixedPointBase a2(rads2);
  FixedPointBase result_sin2 = sin(a2);
  FixedPointBase a3(rads3);
  FixedPointBase result_sin3 = sin(a3);
  EXPECT_NEAR(static_cast<double>(result_sin), sin(rads), 0.002);
  EXPECT_NEAR(static_cast<double>(result_sin2), sin(rads2), 0.002);
  EXPECT_NEAR(static_cast<double>(result_sin3), sin(rads3), 0.002);
}

TEST(TestHydrolibMath, FixedPointBaseCosNegativeArgument) {
  constexpr double rads = -3.14159265358979323846 / 4.0;
  constexpr double rads2 = -3.14159265358979323846 / 6.0;
  constexpr double rads3 = -3.14159265358979323846 / 3.0;
  FixedPointBase a(rads);
  FixedPointBase result_cos = cos(a);
  FixedPointBase a2(rads2);
  FixedPointBase result_cos2 = cos(a2);
  FixedPointBase a3(rads3);
  FixedPointBase result_cos3 = cos(a3);
  EXPECT_NEAR(static_cast<double>(result_cos), cos(rads), 0.002);
  EXPECT_NEAR(static_cast<double>(result_cos2), cos(rads2), 0.002);
  EXPECT_NEAR(static_cast<double>(result_cos3), cos(rads3), 0.002);
}

TEST(TestHydrolibMath, FixedPointBaseGetAbsIntPart) {
  FixedPointBase fp1(5.75);
  EXPECT_EQ(fp1.GetAbsIntPart(), 5);

  FixedPointBase fp2(10.25);
  EXPECT_EQ(fp2.GetAbsIntPart(), 10);

  FixedPointBase fp3(0.99);
  EXPECT_EQ(fp3.GetAbsIntPart(), 0);

  FixedPointBase fp4(-5.75);
  EXPECT_EQ(fp4.GetAbsIntPart(), 5);

  FixedPointBase fp5(-0.25);
  EXPECT_EQ(fp5.GetAbsIntPart(), 0);

  FixedPointBase fp6(42);
  EXPECT_EQ(fp6.GetAbsIntPart(), 42);

  FixedPointBase fp7(-15);
  EXPECT_EQ(fp7.GetAbsIntPart(), 15);

  FixedPointBase fp8(0);
  EXPECT_EQ(fp8.GetAbsIntPart(), 0);
}

TEST(TestHydrolibMath, FixedPointBaseGetAbsFractionPart) {
  FixedPointBase fp1(5.25);
  EXPECT_EQ(fp1.GetAbsFractionPart(),
            0.25 * (1 << FixedPointBase::kFractionBits));

  FixedPointBase fp2(3.5);
  EXPECT_EQ(fp2.GetAbsFractionPart(),
            0.5 * (1 << FixedPointBase::kFractionBits));

  FixedPointBase fp3(7.75);
  EXPECT_EQ(fp3.GetAbsFractionPart(),
            0.75 * (1 << FixedPointBase::kFractionBits));

  FixedPointBase fp4(10);
  EXPECT_EQ(fp4.GetAbsFractionPart(), 0);

  FixedPointBase fp5(0);
  EXPECT_EQ(fp5.GetAbsFractionPart(), 0);

  FixedPointBase fp6(-4);
  EXPECT_EQ(fp6.GetAbsFractionPart(), 0);

  FixedPointBase fp7(-2.25);
  EXPECT_EQ(fp7.GetAbsFractionPart(),
            0.25 * (1 << FixedPointBase::kFractionBits));
}
