#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>

#include <chrono>
#include <climits>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <tuple>

#include "hydrolib_fixed_point.hpp"
#include "mock_stream.hpp"

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
                      FixedPointBase::kLowerNotIncludedBound + 1, 1, -1, 5, 7,
                      32, -15, -192, 134));

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

constexpr std::array<DoubleConstructorTestCase, 20>
    kDoubleConstructorTestCases = {
        0,
        1,
        -1,
        FixedPointBase::kUpperNotIncludedBound - FixedPointBase::kLeastBitValue,
        FixedPointBase::kLowerNotIncludedBound + FixedPointBase::kLeastBitValue,
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

TEST_P(TestFixedPointConstructorDouble, ToInt) {
  auto test_case = GetParam();
  EXPECT_EQ(static_cast<int>(test_case.fixed_point),
            static_cast<int>(test_case.value));
}

class TestFixedPointConstructorDuration
    : public ::testing::Test,
      public ::testing::WithParamInterface<double> {};

INSTANTIATE_TEST_CASE_P(Test, TestFixedPointConstructorDuration,
                        ::testing::Values(0.0, 1.0, 0.5, 0.25, 0.75, 1.25));

TEST_P(TestFixedPointConstructorDuration, Basic) {
  auto duration_seconds = GetParam();
  auto duration_chrono = std::chrono::duration<double>(duration_seconds);
  FixedPointBase fixed_point(
      std::chrono::duration_cast<std::chrono::steady_clock::duration>(duration_chrono));
  EXPECT_NEAR(static_cast<double>(fixed_point), duration_seconds,
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
    FixedPointBase::kLowerNotIncludedBound + FixedPointBase::kLeastBitValue,
    FixedPointBase::kLeastBitValue,
    -FixedPointBase::kLeastBitValue,
    2.7568,
    -7.891,
    107.11231};

class TestFixedPointBinaryOperations
    : public ::testing::Test,
      public ::testing::WithParamInterface<
          std::tuple<FixedPointBase, FixedPointBase>> {};

INSTANTIATE_TEST_CASE_P(
    Test, TestFixedPointBinaryOperations,
    ::testing::Combine(::testing::ValuesIn(kFixedPointCases),
                       ::testing::ValuesIn(kFixedPointCases)),
    [](const testing::TestParamInfo<TestFixedPointBinaryOperations::ParamType>&
           info) {
      static const auto sanitize = [](const std::string& input) -> std::string {
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
      expected > FixedPointBase::kLowerNotIncludedBound) {
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
      expected > FixedPointBase::kLowerNotIncludedBound) {
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
      expected > FixedPointBase::kLowerNotIncludedBound) {
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
      expected > FixedPointBase::kLowerNotIncludedBound) {
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
      expected > FixedPointBase::kLowerNotIncludedBound) {
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
      expected > FixedPointBase::kLowerNotIncludedBound) {
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
        expected > FixedPointBase::kLowerNotIncludedBound) {
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
        expected > FixedPointBase::kLowerNotIncludedBound) {
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

constexpr std::array<FixedPointBase, 21> kFixedPointTrigonometryCases = {
    0,
    kPi<FixedPointBase::kFractionBits>,
    -kPi<FixedPointBase::kFractionBits>,
    kPi<FixedPointBase::kFractionBits> / 2,
    -kPi<FixedPointBase::kFractionBits> / 2,
    kPi<FixedPointBase::kFractionBits> / 4,
    -kPi<FixedPointBase::kFractionBits> / 4,
    kPi<FixedPointBase::kFractionBits> / 4 * 3,
    -kPi<FixedPointBase::kFractionBits> / 4 * 3,
    kPi<FixedPointBase::kFractionBits> / 3,
    -kPi<FixedPointBase::kFractionBits> / 3,
    kPi<FixedPointBase::kFractionBits> / 6,
    -kPi<FixedPointBase::kFractionBits> / 6,
    kPi<FixedPointBase::kFractionBits> + kPi<FixedPointBase::kFractionBits> / 2,
    -kPi<FixedPointBase::kFractionBits> -
        kPi<FixedPointBase::kFractionBits> / 2,
    kPi<FixedPointBase::kFractionBits> + kPi<FixedPointBase::kFractionBits> / 4,
    -kPi<FixedPointBase::kFractionBits> -
        kPi<FixedPointBase::kFractionBits> / 4,
    FixedPointBase::kUpperNotIncludedBound - FixedPointBase::kLeastBitValue,
    FixedPointBase::kLowerNotIncludedBound + FixedPointBase::kLeastBitValue,
    FixedPointBase::kLeastBitValue,
    -FixedPointBase::kLeastBitValue,
};

class TestFixedPointTrigonometry
    : public ::testing::Test,
      public ::testing::WithParamInterface<FixedPointBase> {};

INSTANTIATE_TEST_CASE_P(
    Test, TestFixedPointTrigonometry,
    ::testing::ValuesIn(kFixedPointTrigonometryCases),
    [](const testing::TestParamInfo<TestFixedPointTrigonometry::ParamType>&
           info) {
      static const auto sanitize = [](const std::string& input) -> std::string {
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
      auto value = info.param;
      return sanitize(std::to_string(static_cast<double>(value)));
    });

TEST_P(TestFixedPointTrigonometry, Sin) {
  auto value = GetParam();
  auto result = sin(value);
  if (value > hydrolib::math::kPi<FixedPointBase::kFractionBits> * 2 ||
      value < -hydrolib::math::kPi<FixedPointBase::kFractionBits> * 2) {
    EXPECT_NEAR(static_cast<double>(result), sin(static_cast<double>(value)),
                0.001);
  } else {
    EXPECT_NEAR(static_cast<double>(result), sin(static_cast<double>(value)),
                2 * FixedPointBase::kLeastBitValue);
  }
}

TEST_P(TestFixedPointTrigonometry, Cos) {
  auto value = GetParam();
  auto result = cos(value);
  if (value > hydrolib::math::kPi<FixedPointBase::kFractionBits> * 2 ||
      value < -hydrolib::math::kPi<FixedPointBase::kFractionBits> * 2) {
    EXPECT_NEAR(static_cast<double>(result), cos(static_cast<double>(value)),
                0.001);
  } else {
    EXPECT_NEAR(static_cast<double>(result), cos(static_cast<double>(value)),
                2 * FixedPointBase::kLeastBitValue);
  }
}

constexpr std::array<FixedPointBase, 10> kFixedPointSqrtCases = {
    0,
    1,
    FixedPointBase::kUpperNotIncludedBound - FixedPointBase::kLeastBitValue,
    FixedPointBase::kLeastBitValue,
    2.7568,
    107.11231,
    0.25,
    7304.4209,
    0.0881,
    4.001};

class TestFixedPointSqrt
    : public ::testing::Test,
      public ::testing::WithParamInterface<FixedPointBase> {};

INSTANTIATE_TEST_CASE_P(
    Test, TestFixedPointSqrt, ::testing::ValuesIn(kFixedPointSqrtCases),
    [](const testing::TestParamInfo<TestFixedPointTrigonometry::ParamType>&
           info) {
      static const auto sanitize = [](const std::string& input) -> std::string {
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
      auto value = info.param;
      return sanitize(std::to_string(static_cast<double>(value)));
    });

TEST_P(TestFixedPointSqrt, Base) {
  auto value = GetParam();
  if (value >= 0) {
    FixedPointBase result = sqrt(value);

    EXPECT_NEAR(static_cast<double>(result), sqrt(static_cast<double>(value)),
                FixedPointBase::kLeastBitValue);
  }
}

class TestFixedPointToBytes
    : public ::testing::Test,
      public ::testing::WithParamInterface<FixedPointBase> {};

INSTANTIATE_TEST_CASE_P(Test, TestFixedPointToBytes,
                        ::testing::ValuesIn(kFixedPointCases));

TEST_P(TestFixedPointToBytes, Base) {
  const auto value = GetParam();
  std::ostringstream output_stream;
  output_stream << std::fixed << std::setprecision(3)
                << static_cast<double>(value);
  const auto expected_string = output_stream.str();
  hydrolib::streams::mock::MockByteStream stream;

  EXPECT_EQ(value.ToBytes(stream), hydrolib::ReturnCode::OK);

  std::string output;
  output.resize(stream.GetSize());
  for (size_t i = 0; i < output.size(); ++i) {
    output[i] = static_cast<char>(stream[i]);
  }

  EXPECT_EQ(output, expected_string);
}
