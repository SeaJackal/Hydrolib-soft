#include <gtest/gtest.h>

#include <array>
#include <tuple>

#include "hydrolib_quaternions.hpp"

using hydrolib::math::Quaternion;

using BinaryParam =
    std::tuple<Quaternion<double>, Quaternion<double>, Quaternion<double>>;

static constexpr std::array<BinaryParam, 6> kQuaternionAdditionCases{{
    BinaryParam{Quaternion<double>{.x = 1, .y = 2, .z = 3, .w = 4},
                Quaternion<double>{.x = 5, .y = 6, .z = 7, .w = 8},
                Quaternion<double>{.x = 6, .y = 8, .z = 10, .w = 12}},
    BinaryParam{Quaternion<double>{.x = -1, .y = -2, .z = -3, .w = -4},
                Quaternion<double>{.x = 1, .y = 2, .z = 3, .w = 4},
                Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0}},
    BinaryParam{Quaternion<double>{.x = 1, .y = 2, .z = 3, .w = 4},
                Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0},
                Quaternion<double>{.x = 1, .y = 2, .z = 3, .w = 4}},
    BinaryParam{Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0},
                Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0},
                Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0}},
    BinaryParam{Quaternion<double>{.x = 0.5, .y = -0.25, .z = 0.75, .w = -1.5},
                Quaternion<double>{.x = -0.5, .y = 0.25, .z = -0.75, .w = 1.5},
                Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0}},
    BinaryParam{
        Quaternion<double>{.x = 1.25, .y = -2.5, .z = 3.75, .w = -4.125},
        Quaternion<double>{.x = -10.5, .y = 20.25, .z = -30.75, .w = 40.875},
        Quaternion<double>{.x = -9.25, .y = 17.75, .z = -27.0, .w = 36.75}},
}};

static constexpr std::array<BinaryParam, 6> kQuaternionSubtractionCases{{
    BinaryParam{Quaternion<double>{.x = 10, .y = 20, .z = 30, .w = 40},
                Quaternion<double>{.x = 5, .y = 6, .z = 7, .w = 8},
                Quaternion<double>{.x = 5, .y = 14, .z = 23, .w = 32}},
    BinaryParam{Quaternion<double>{.x = 1, .y = 2, .z = 3, .w = 4},
                Quaternion<double>{.x = 1, .y = 1, .z = 1, .w = 1},
                Quaternion<double>{.x = 0, .y = 1, .z = 2, .w = 3}},
    BinaryParam{Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0},
                Quaternion<double>{.x = 1, .y = 2, .z = 3, .w = 4},
                Quaternion<double>{.x = -1, .y = -2, .z = -3, .w = -4}},
    BinaryParam{Quaternion<double>{.x = 1, .y = 2, .z = 3, .w = 4},
                Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0},
                Quaternion<double>{.x = 1, .y = 2, .z = 3, .w = 4}},
    BinaryParam{
        Quaternion<double>{.x = 1.25, .y = -2.5, .z = 3.75, .w = -4.125},
        Quaternion<double>{.x = 1.25, .y = -2.5, .z = 3.75, .w = -4.125},
        Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0}},
    BinaryParam{
        Quaternion<double>{.x = -10.5, .y = 20.25, .z = -30.75, .w = 40.875},
        Quaternion<double>{.x = 1.25, .y = -2.5, .z = 3.75, .w = -4.125},
        Quaternion<double>{.x = -11.75, .y = 22.75, .z = -34.5, .w = 45.0}},
}};

static constexpr std::array<BinaryParam, 8> kQuaternionMultiplicationCases{{
    BinaryParam{Quaternion<double>{.x = 1, .y = 0, .z = 0, .w = 0},
                Quaternion<double>{.x = 0, .y = 1, .z = 0, .w = 0},
                Quaternion<double>{.x = 0, .y = 0, .z = 1, .w = 0}},
    BinaryParam{Quaternion<double>{.x = 2, .y = 4, .z = 1, .w = 3},
                Quaternion<double>{.x = 3, .y = 5, .z = 2, .w = 1},
                Quaternion<double>{.x = 14, .y = 18, .z = 5, .w = -25}},
    BinaryParam{Quaternion<double>{.x = 3, .y = 5, .z = 2, .w = 1},
                Quaternion<double>{.x = 2, .y = 4, .z = 1, .w = 3},
                Quaternion<double>{.x = 8, .y = 20, .z = 9, .w = -25}},
    BinaryParam{Quaternion<double>{.x = 2, .y = 4, .z = 6, .w = 8},
                Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 1},
                Quaternion<double>{.x = 2, .y = 4, .z = 6, .w = 8}},
    BinaryParam{Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 1},
                Quaternion<double>{.x = 2, .y = 4, .z = 6, .w = 8},
                Quaternion<double>{.x = 2, .y = 4, .z = 6, .w = 8}},
    BinaryParam{Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0},
                Quaternion<double>{.x = 2, .y = 4, .z = 6, .w = 8},
                Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0}},
    BinaryParam{Quaternion<double>{.x = 2, .y = 4, .z = 6, .w = 8},
                Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0},
                Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0}},
    BinaryParam{Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 1},
                Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 1},
                Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 1}},
}};

using UnaryParam = std::tuple<Quaternion<double>, Quaternion<double>>;

static constexpr std::array<UnaryParam, 4> kQuaternionUnaryMinusCases{{
    UnaryParam{Quaternion<double>{.x = 1, .y = -2, .z = 3, .w = -4},
               Quaternion<double>{.x = -1, .y = 2, .z = -3, .w = 4}},
    UnaryParam{Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0},
               Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0}},
    UnaryParam{Quaternion<double>{.x = -1, .y = 2, .z = -3, .w = 4},
               Quaternion<double>{.x = 1, .y = -2, .z = 3, .w = -4}},
    UnaryParam{Quaternion<double>{.x = 0.5, .y = -0.25, .z = 0.75, .w = -1.5},
               Quaternion<double>{.x = -0.5, .y = 0.25, .z = -0.75, .w = 1.5}},
}};

static constexpr std::array<UnaryParam, 4> kQuaternionConjugateCases{{
    UnaryParam{Quaternion<double>{.x = 1, .y = 2, .z = 3, .w = 4},
               Quaternion<double>{.x = -1, .y = -2, .z = -3, .w = 4}},
    UnaryParam{Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0},
               Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0}},
    UnaryParam{Quaternion<double>{.x = 0.5, .y = -0.25, .z = 0.75, .w = -1.5},
               Quaternion<double>{.x = -0.5, .y = 0.25, .z = -0.75, .w = -1.5}},
    UnaryParam{
        Quaternion<double>{.x = -10.5, .y = 20.25, .z = -30.75, .w = 40.875},
        Quaternion<double>{.x = 10.5, .y = -20.25, .z = 30.75, .w = 40.875}},
}};

using ScalarParam = std::tuple<Quaternion<double>, double, Quaternion<double>>;

static constexpr std::array<ScalarParam, 5>
    kQuaternionScalarMultiplicationCases{{
        ScalarParam{Quaternion<double>{.x = 2, .y = 4, .z = 6, .w = 8}, 3.0,
                    Quaternion<double>{.x = 6, .y = 12, .z = 18, .w = 24}},
        ScalarParam{Quaternion<double>{.x = 2, .y = 4, .z = 6, .w = 8}, 0.0,
                    Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0}},
        ScalarParam{Quaternion<double>{.x = 2, .y = 4, .z = 6, .w = 8}, 1.0,
                    Quaternion<double>{.x = 2, .y = 4, .z = 6, .w = 8}},
        ScalarParam{Quaternion<double>{.x = 2, .y = -4, .z = 6, .w = -8}, -2.0,
                    Quaternion<double>{.x = -4, .y = 8, .z = -12, .w = 16}},
        ScalarParam{
            Quaternion<double>{.x = 0.5, .y = -0.25, .z = 0.75, .w = -1.5}, 2.0,
            Quaternion<double>{.x = 1.0, .y = -0.5, .z = 1.5, .w = -3.0}},
    }};

static constexpr std::array<ScalarParam, 4> kQuaternionScalarDivisionCases{{
    ScalarParam{Quaternion<double>{.x = 6, .y = 12, .z = 18, .w = 24}, 3.0,
                Quaternion<double>{.x = 2, .y = 4, .z = 6, .w = 8}},
    ScalarParam{Quaternion<double>{.x = 6, .y = 12, .z = 18, .w = 24}, 1.0,
                Quaternion<double>{.x = 6, .y = 12, .z = 18, .w = 24}},
    ScalarParam{Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0}, 3.0,
                Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0}},
    ScalarParam{Quaternion<double>{.x = 1.0, .y = -0.5, .z = 1.5, .w = -3.0},
                2.0,
                Quaternion<double>{.x = 0.5, .y = -0.25, .z = 0.75, .w = -1.5}},
}};

using DotParam = std::tuple<Quaternion<double>, Quaternion<double>, double>;

static constexpr std::array<DotParam, 4> kQuaternionDotCases{{
    DotParam{Quaternion<double>{.x = 1, .y = 2, .z = 3, .w = 4},
             Quaternion<double>{.x = 5, .y = 6, .z = 7, .w = 8}, 70.0},
    DotParam{Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0},
             Quaternion<double>{.x = 5, .y = 6, .z = 7, .w = 8}, 0.0},
    DotParam{Quaternion<double>{.x = 1, .y = 2, .z = 3, .w = 4},
             Quaternion<double>{.x = 1, .y = 2, .z = 3, .w = 4}, 30.0},
    DotParam{Quaternion<double>{.x = 0.5, .y = -0.25, .z = 0.75, .w = -1.5},
             Quaternion<double>{.x = -0.5, .y = 0.25, .z = -0.75, .w = 1.5},
             -3.125},
}};

using NormParam = std::tuple<Quaternion<double>, double>;

static constexpr std::array<NormParam, 4> kQuaternionNormCases{{
    NormParam{Quaternion<double>{.x = 3, .y = 4, .z = 0, .w = 0}, 5.0},
    NormParam{Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0}, 0.0},
    NormParam{Quaternion<double>{.x = 1, .y = 2, .z = 3, .w = 4},
              5.477225575051661},
    NormParam{Quaternion<double>{.x = 0.5, .y = -0.25, .z = 0.75, .w = -1.5},
              1.7677669529663689},
}};

using NormalizeParam = std::tuple<Quaternion<double>, Quaternion<double>>;

static constexpr std::array<NormalizeParam, 3> kQuaternionNormalizeCases{{
    NormalizeParam{Quaternion<double>{.x = 3, .y = 4, .z = 0, .w = 0},
                   Quaternion<double>{.x = 0.6, .y = 0.8, .z = 0, .w = 0}},
    NormalizeParam{Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0},
                   Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = 0}},
    NormalizeParam{Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = -2},
                   Quaternion<double>{.x = 0, .y = 0, .z = 0, .w = -1}},
}};

TEST(TestHydrolibMath, QuaternionConstructorWithComponents) {
  Quaternion<double> quaternion{.x = 1.0, .y = 2.0, .z = 3.0, .w = 4.0};

  EXPECT_DOUBLE_EQ(quaternion.x, 1.0);
  EXPECT_DOUBLE_EQ(quaternion.y, 2.0);
  EXPECT_DOUBLE_EQ(quaternion.z, 3.0);
  EXPECT_DOUBLE_EQ(quaternion.w, 4.0);
}

class TestQuaternionBinaryOperations
    : public ::testing::Test,
      public ::testing::WithParamInterface<std::tuple<
          Quaternion<double>, Quaternion<double>, Quaternion<double>>> {};

class TestQuaternionAddition : public TestQuaternionBinaryOperations {};

INSTANTIATE_TEST_CASE_P(Test, TestQuaternionAddition,
                        ::testing::ValuesIn(kQuaternionAdditionCases));

TEST_P(TestQuaternionAddition, Basic) {
  auto values = GetParam();
  auto quaternion_1 = std::get<0>(values);
  auto quaternion_2 = std::get<1>(values);
  auto expected = std::get<2>(values);

  auto result = quaternion_1 + quaternion_2;

  EXPECT_DOUBLE_EQ(result.x, expected.x);
  EXPECT_DOUBLE_EQ(result.y, expected.y);
  EXPECT_DOUBLE_EQ(result.z, expected.z);
  EXPECT_DOUBLE_EQ(result.w, expected.w);
}

class TestQuaternionSubtraction : public TestQuaternionBinaryOperations {};

INSTANTIATE_TEST_CASE_P(Test, TestQuaternionSubtraction,
                        ::testing::ValuesIn(kQuaternionSubtractionCases));

TEST_P(TestQuaternionSubtraction, Basic) {
  auto values = GetParam();
  auto quaternion_1 = std::get<0>(values);
  auto quaternion_2 = std::get<1>(values);
  auto expected = std::get<2>(values);

  auto result = quaternion_1 - quaternion_2;

  EXPECT_DOUBLE_EQ(result.x, expected.x);
  EXPECT_DOUBLE_EQ(result.y, expected.y);
  EXPECT_DOUBLE_EQ(result.z, expected.z);
  EXPECT_DOUBLE_EQ(result.w, expected.w);
}

class TestQuaternionMultiplication : public TestQuaternionBinaryOperations {};

INSTANTIATE_TEST_CASE_P(Test, TestQuaternionMultiplication,
                        ::testing::ValuesIn(kQuaternionMultiplicationCases));

TEST_P(TestQuaternionMultiplication, Basic) {
  auto values = GetParam();
  auto quaternion_1 = std::get<0>(values);
  auto quaternion_2 = std::get<1>(values);
  auto expected = std::get<2>(values);

  auto result = quaternion_1 * quaternion_2;

  EXPECT_DOUBLE_EQ(result.x, expected.x);
  EXPECT_DOUBLE_EQ(result.y, expected.y);
  EXPECT_DOUBLE_EQ(result.z, expected.z);
  EXPECT_DOUBLE_EQ(result.w, expected.w);
}

class TestQuaternionUnaryOperations
    : public ::testing::Test,
      public ::testing::WithParamInterface<UnaryParam> {};

class TestQuaternionUnaryMinus : public TestQuaternionUnaryOperations {};

INSTANTIATE_TEST_CASE_P(Test, TestQuaternionUnaryMinus,
                        ::testing::ValuesIn(kQuaternionUnaryMinusCases));

TEST_P(TestQuaternionUnaryMinus, Basic) {
  auto values = GetParam();
  auto quaternion = std::get<0>(values);
  auto expected = std::get<1>(values);
  auto result = -quaternion;
  EXPECT_DOUBLE_EQ(result.x, expected.x);
  EXPECT_DOUBLE_EQ(result.y, expected.y);
  EXPECT_DOUBLE_EQ(result.z, expected.z);
  EXPECT_DOUBLE_EQ(result.w, expected.w);
}

class TestQuaternionConjugate : public TestQuaternionUnaryOperations {};

INSTANTIATE_TEST_CASE_P(Test, TestQuaternionConjugate,
                        ::testing::ValuesIn(kQuaternionConjugateCases));

TEST_P(TestQuaternionConjugate, Basic) {
  auto values = GetParam();
  auto quaternion = std::get<0>(values);
  auto expected = std::get<1>(values);
  auto result = !quaternion;
  EXPECT_DOUBLE_EQ(result.x, expected.x);
  EXPECT_DOUBLE_EQ(result.y, expected.y);
  EXPECT_DOUBLE_EQ(result.z, expected.z);
  EXPECT_DOUBLE_EQ(result.w, expected.w);
}

class TestQuaternionScalarMultiplication
    : public ::testing::Test,
      public ::testing::WithParamInterface<ScalarParam> {};

INSTANTIATE_TEST_CASE_P(
    Test, TestQuaternionScalarMultiplication,
    ::testing::ValuesIn(kQuaternionScalarMultiplicationCases));

TEST_P(TestQuaternionScalarMultiplication, Basic) {
  auto values = GetParam();
  auto quaternion = std::get<0>(values);
  auto multiplier = std::get<1>(values);
  auto expected = std::get<2>(values);
  auto left = quaternion * multiplier;
  auto right = multiplier * quaternion;
  EXPECT_DOUBLE_EQ(left.x, expected.x);
  EXPECT_DOUBLE_EQ(left.y, expected.y);
  EXPECT_DOUBLE_EQ(left.z, expected.z);
  EXPECT_DOUBLE_EQ(left.w, expected.w);
  EXPECT_DOUBLE_EQ(right.x, expected.x);
  EXPECT_DOUBLE_EQ(right.y, expected.y);
  EXPECT_DOUBLE_EQ(right.z, expected.z);
  EXPECT_DOUBLE_EQ(right.w, expected.w);
}

class TestQuaternionScalarDivision
    : public ::testing::Test,
      public ::testing::WithParamInterface<ScalarParam> {};

INSTANTIATE_TEST_CASE_P(Test, TestQuaternionScalarDivision,
                        ::testing::ValuesIn(kQuaternionScalarDivisionCases));

TEST_P(TestQuaternionScalarDivision, Basic) {
  auto values = GetParam();
  auto quaternion = std::get<0>(values);
  auto divider = std::get<1>(values);
  auto expected = std::get<2>(values);
  auto result = quaternion / divider;
  EXPECT_DOUBLE_EQ(result.x, expected.x);
  EXPECT_DOUBLE_EQ(result.y, expected.y);
  EXPECT_DOUBLE_EQ(result.z, expected.z);
  EXPECT_DOUBLE_EQ(result.w, expected.w);
}

class TestQuaternionDot : public ::testing::Test,
                          public ::testing::WithParamInterface<DotParam> {};

INSTANTIATE_TEST_CASE_P(Test, TestQuaternionDot,
                        ::testing::ValuesIn(kQuaternionDotCases));

TEST_P(TestQuaternionDot, Basic) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);
  auto expected = std::get<2>(values);
  EXPECT_DOUBLE_EQ(Quaternion<double>::Dot(first, second), expected);
}

class TestQuaternionNorm : public ::testing::Test,
                           public ::testing::WithParamInterface<NormParam> {};

INSTANTIATE_TEST_CASE_P(Test, TestQuaternionNorm,
                        ::testing::ValuesIn(kQuaternionNormCases));

TEST_P(TestQuaternionNorm, Basic) {
  auto values = GetParam();
  auto quaternion = std::get<0>(values);
  auto expected = std::get<1>(values);
  EXPECT_DOUBLE_EQ(Quaternion<double>::GetNorm(quaternion), expected);
}

class TestQuaternionNormalize
    : public ::testing::Test,
      public ::testing::WithParamInterface<NormalizeParam> {};

INSTANTIATE_TEST_CASE_P(Test, TestQuaternionNormalize,
                        ::testing::ValuesIn(kQuaternionNormalizeCases));

TEST_P(TestQuaternionNormalize, Basic) {
  auto values = GetParam();
  auto quaternion = std::get<0>(values);
  auto expected = std::get<1>(values);
  Quaternion<double>::Normalize(quaternion);
  EXPECT_DOUBLE_EQ(quaternion.x, expected.x);
  EXPECT_DOUBLE_EQ(quaternion.y, expected.y);
  EXPECT_DOUBLE_EQ(quaternion.z, expected.z);
  EXPECT_DOUBLE_EQ(quaternion.w, expected.w);
}
