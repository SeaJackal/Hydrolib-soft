#include <gtest/gtest.h>

#include <array>
#include <tuple>

#include "hydrolib_vector3d.hpp"

using hydrolib::math::Vector3D;
using Vector = Vector3D<double>;

static constexpr std::array<std::tuple<Vector, Vector, double>, 6> kDotCases{{
    {Vector{1.0, 0.0, 0.0}, Vector{0.0, 1.0, 0.0}, 0.0},
    {Vector{3.0, 4.0, 5.0}, Vector{3.0, 4.0, 5.0}, 50.0},
    {Vector{1.0, 2.0, 3.0}, Vector{4.0, 5.0, 6.0}, 32.0},
    {Vector{1.0, -2.0, 3.0}, Vector{-1.0, 2.0, 1.0}, -2.0},
    {Vector{0.0, 0.0, 0.0}, Vector{5.0, 10.0, 15.0}, 0.0},
    {Vector{3.0, 4.0, 0.0}, Vector{3.0, 4.0, 0.0}, 25.0},
}};

static constexpr std::array<std::tuple<Vector, Vector, Vector>, 5> kCrossCases{{
    {Vector{1.0, 0.0, 0.0}, Vector{0.0, 1.0, 0.0}, Vector{0.0, 0.0, 1.0}},
    {Vector{1.0, 2.0, 3.0}, Vector{4.0, 5.0, 6.0}, Vector{-3.0, 6.0, -3.0}},
    {Vector{2.0, 4.0, 6.0}, Vector{1.0, 2.0, 3.0}, Vector{0.0, 0.0, 0.0}},
    {Vector{0.0, 0.0, 0.0}, Vector{5.0, 10.0, 15.0}, Vector{0.0, 0.0, 0.0}},
    {Vector{1.0, 2.0, 3.0}, Vector{1.0, 2.0, 3.0}, Vector{0.0, 0.0, 0.0}},
}};

static constexpr std::array<std::tuple<Vector, double>, 3> kLengthCases{{
    {Vector{3.0, 4.0, 0.0}, 5.0},
    {Vector{1.0, 2.0, 2.0}, 3.0},
    {Vector{0.0, 0.0, 0.0}, 0.0},
}};

static constexpr std::array<std::tuple<Vector, Vector>, 3> kNormalizeCases{{
    {Vector{3.0, 4.0, 0.0}, Vector{3.0 / 5.0, 4.0 / 5.0, 0.0}},
    {Vector{0.0, 0.0, 0.0}, Vector{0.0, 0.0, 0.0}},
    {Vector{1.0, 2.0, 2.0}, Vector{1.0 / 3.0, 2.0 / 3.0, 2.0 / 3.0}},
}};

static constexpr std::array<std::tuple<Vector, Vector, Vector>, 6>
    kAdditionCases{{
        {Vector{1.0, 2.0, 3.0}, Vector{4.0, 5.0, 6.0}, Vector{5.0, 7.0, 9.0}},
        {Vector{-1.0, -2.0, -3.0}, Vector{1.0, 2.0, 3.0}, Vector{0.0, 0.0, 0.0}},
        {Vector{0.0, 0.0, 0.0}, Vector{0.0, 0.0, 0.0}, Vector{0.0, 0.0, 0.0}},
        {Vector{0.5, -0.25, 0.75}, Vector{-0.5, 0.25, -0.75}, Vector{0.0, 0.0, 0.0}},
        {Vector{10.0, 20.0, 30.0}, Vector{-5.0, 6.0, -7.0}, Vector{5.0, 26.0, 23.0}},
        {Vector{1.25, -2.5, 3.75}, Vector{-10.5, 20.25, -30.75},
         Vector{-9.25, 17.75, -27.0}},
    }};

static constexpr std::array<std::tuple<Vector, Vector, Vector>, 6>
    kSubtractionCases{{
        {Vector{10.0, 20.0, 30.0}, Vector{5.0, 6.0, 7.0}, Vector{5.0, 14.0, 23.0}},
        {Vector{1.0, 2.0, 3.0}, Vector{1.0, 1.0, 1.0}, Vector{0.0, 1.0, 2.0}},
        {Vector{0.0, 0.0, 0.0}, Vector{1.0, 2.0, 3.0}, Vector{-1.0, -2.0, -3.0}},
        {Vector{1.0, 2.0, 3.0}, Vector{0.0, 0.0, 0.0}, Vector{1.0, 2.0, 3.0}},
        {Vector{1.25, -2.5, 3.75}, Vector{1.25, -2.5, 3.75}, Vector{0.0, 0.0, 0.0}},
        {Vector{-10.5, 20.25, -30.75}, Vector{1.25, -2.5, 3.75},
         Vector{-11.75, 22.75, -34.5}},
    }};

static constexpr std::array<std::tuple<Vector, Vector>, 4> kUnaryMinusCases{{
    {Vector{1.0, -2.0, 3.0}, Vector{-1.0, 2.0, -3.0}},
    {Vector{0.0, 0.0, 0.0}, Vector{0.0, 0.0, 0.0}},
    {Vector{-1.0, 2.0, -3.0}, Vector{1.0, -2.0, 3.0}},
    {Vector{0.5, -0.25, 0.75}, Vector{-0.5, 0.25, -0.75}},
}};

static constexpr std::array<std::tuple<Vector, Vector, Vector>, 3> kPlusAssignCases{{
    {Vector{1.0, 2.0, 3.0}, Vector{4.0, 5.0, 6.0}, Vector{5.0, 7.0, 9.0}},
    {Vector{0.0, 0.0, 0.0}, Vector{1.0, 2.0, 3.0}, Vector{1.0, 2.0, 3.0}},
    {Vector{-1.0, -2.0, -3.0}, Vector{1.0, 2.0, 3.0}, Vector{0.0, 0.0, 0.0}},
}};

static constexpr std::array<std::tuple<Vector, Vector, Vector>, 3>
    kMinusAssignCases{{
        {Vector{10.0, 20.0, 30.0}, Vector{5.0, 6.0, 7.0}, Vector{5.0, 14.0, 23.0}},
        {Vector{0.0, 0.0, 0.0}, Vector{1.0, 2.0, 3.0}, Vector{-1.0, -2.0, -3.0}},
        {Vector{1.0, 2.0, 3.0}, Vector{0.0, 0.0, 0.0}, Vector{1.0, 2.0, 3.0}},
    }};

static constexpr std::array<std::tuple<Vector, double, Vector>, 6>
    kScalarMultiplicationCases{{
        {Vector{1.0, 2.0, 3.0}, 2.0, Vector{2.0, 4.0, 6.0}},
        {Vector{2.0, 4.0, 6.0}, 0.5, Vector{1.0, 2.0, 3.0}},
        {Vector{5.0, 10.0, 15.0}, 0.0, Vector{0.0, 0.0, 0.0}},
        {Vector{1.0, -2.0, 3.0}, -2.0, Vector{-2.0, 4.0, -6.0}},
        {Vector{2.5, 5.0, 7.5}, 4.0, Vector{10.0, 20.0, 30.0}},
        {Vector{1.0, 2.0, 3.0}, 6.0, Vector{6.0, 12.0, 18.0}},
    }};

static constexpr std::array<std::tuple<Vector, double, Vector>, 5>
    kScalarDivisionCases{{
        {Vector{6.0, 8.0, 10.0}, 2.0, Vector{3.0, 4.0, 5.0}},
        {Vector{9.0, 12.0, 15.0}, 3.0, Vector{3.0, 4.0, 5.0}},
        {Vector{1.0, 2.0, 3.0}, 0.5, Vector{2.0, 4.0, 6.0}},
        {Vector{10.0, 20.0, 30.0}, 1.0, Vector{10.0, 20.0, 30.0}},
        {Vector{0.0, 0.0, 0.0}, 3.0, Vector{0.0, 0.0, 0.0}},
    }};

class TestVectorDot : public ::testing::Test,
                      public ::testing::WithParamInterface<
                          std::tuple<Vector, Vector, double>> {};

INSTANTIATE_TEST_CASE_P(Test, TestVectorDot, ::testing::ValuesIn(kDotCases));

TEST_P(TestVectorDot, Basic) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);
  auto expected = std::get<2>(values);
  EXPECT_DOUBLE_EQ(Vector::Dot(first, second), expected);
}

class TestVectorCross : public ::testing::Test,
                        public ::testing::WithParamInterface<
                            std::tuple<Vector, Vector, Vector>> {};

INSTANTIATE_TEST_CASE_P(Test, TestVectorCross, ::testing::ValuesIn(kCrossCases));

TEST_P(TestVectorCross, Basic) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);
  auto expected = std::get<2>(values);
  auto result = Vector::Cross(first, second);
  EXPECT_DOUBLE_EQ(result.x, expected.x);
  EXPECT_DOUBLE_EQ(result.y, expected.y);
  EXPECT_DOUBLE_EQ(result.z, expected.z);
}

class TestVectorLength : public ::testing::Test,
                         public ::testing::WithParamInterface<
                             std::tuple<Vector, double>> {};

INSTANTIATE_TEST_CASE_P(Test, TestVectorLength, ::testing::ValuesIn(kLengthCases));

TEST_P(TestVectorLength, Basic) {
  auto values = GetParam();
  auto vector = std::get<0>(values);
  auto expected = std::get<1>(values);
  EXPECT_DOUBLE_EQ(Vector::Length(vector), expected);
}

class TestVectorNormalize : public ::testing::Test,
                            public ::testing::WithParamInterface<
                                std::tuple<Vector, Vector>> {};

INSTANTIATE_TEST_CASE_P(Test, TestVectorNormalize,
                        ::testing::ValuesIn(kNormalizeCases));

TEST_P(TestVectorNormalize, Basic) {
  auto values = GetParam();
  auto vector = std::get<0>(values);
  auto expected = std::get<1>(values);
  Vector::Normalize(vector);
  EXPECT_DOUBLE_EQ(vector.x, expected.x);
  EXPECT_DOUBLE_EQ(vector.y, expected.y);
  EXPECT_DOUBLE_EQ(vector.z, expected.z);
}

class TestVectorAddition : public ::testing::Test,
                           public ::testing::WithParamInterface<
                               std::tuple<Vector, Vector, Vector>> {};

INSTANTIATE_TEST_CASE_P(Test, TestVectorAddition,
                        ::testing::ValuesIn(kAdditionCases));

TEST_P(TestVectorAddition, Basic) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);
  auto expected = std::get<2>(values);
  auto result = first + second;
  EXPECT_DOUBLE_EQ(result.x, expected.x);
  EXPECT_DOUBLE_EQ(result.y, expected.y);
  EXPECT_DOUBLE_EQ(result.z, expected.z);
}

class TestVectorSubtraction : public ::testing::Test,
                              public ::testing::WithParamInterface<
                                  std::tuple<Vector, Vector, Vector>> {};

INSTANTIATE_TEST_CASE_P(Test, TestVectorSubtraction,
                        ::testing::ValuesIn(kSubtractionCases));

TEST_P(TestVectorSubtraction, Basic) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);
  auto expected = std::get<2>(values);
  auto result = first - second;
  EXPECT_DOUBLE_EQ(result.x, expected.x);
  EXPECT_DOUBLE_EQ(result.y, expected.y);
  EXPECT_DOUBLE_EQ(result.z, expected.z);
}

class TestVectorUnaryMinus : public ::testing::Test,
                             public ::testing::WithParamInterface<
                                 std::tuple<Vector, Vector>> {};

INSTANTIATE_TEST_CASE_P(Test, TestVectorUnaryMinus,
                        ::testing::ValuesIn(kUnaryMinusCases));

TEST_P(TestVectorUnaryMinus, Basic) {
  auto values = GetParam();
  auto vector = std::get<0>(values);
  auto expected = std::get<1>(values);
  auto result = -vector;
  EXPECT_DOUBLE_EQ(result.x, expected.x);
  EXPECT_DOUBLE_EQ(result.y, expected.y);
  EXPECT_DOUBLE_EQ(result.z, expected.z);
}

class TestVectorPlusAssign : public ::testing::Test,
                             public ::testing::WithParamInterface<
                                 std::tuple<Vector, Vector, Vector>> {};

INSTANTIATE_TEST_CASE_P(Test, TestVectorPlusAssign,
                        ::testing::ValuesIn(kPlusAssignCases));

TEST_P(TestVectorPlusAssign, Basic) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);
  auto expected = std::get<2>(values);
  first += second;
  EXPECT_DOUBLE_EQ(first.x, expected.x);
  EXPECT_DOUBLE_EQ(first.y, expected.y);
  EXPECT_DOUBLE_EQ(first.z, expected.z);
}

class TestVectorMinusAssign : public ::testing::Test,
                              public ::testing::WithParamInterface<
                                  std::tuple<Vector, Vector, Vector>> {};

INSTANTIATE_TEST_CASE_P(Test, TestVectorMinusAssign,
                        ::testing::ValuesIn(kMinusAssignCases));

TEST_P(TestVectorMinusAssign, Basic) {
  auto values = GetParam();
  auto first = std::get<0>(values);
  auto second = std::get<1>(values);
  auto expected = std::get<2>(values);
  first -= second;
  EXPECT_DOUBLE_EQ(first.x, expected.x);
  EXPECT_DOUBLE_EQ(first.y, expected.y);
  EXPECT_DOUBLE_EQ(first.z, expected.z);
}

class TestVectorScalarMultiplication
    : public ::testing::Test,
      public ::testing::WithParamInterface<std::tuple<Vector, double, Vector>> {};

INSTANTIATE_TEST_CASE_P(Test, TestVectorScalarMultiplication,
                        ::testing::ValuesIn(kScalarMultiplicationCases));

TEST_P(TestVectorScalarMultiplication, Basic) {
  auto values = GetParam();
  auto vector = std::get<0>(values);
  auto scalar = std::get<1>(values);
  auto expected = std::get<2>(values);
  auto result = vector * scalar;
  EXPECT_DOUBLE_EQ(result.x, expected.x);
  EXPECT_DOUBLE_EQ(result.y, expected.y);
  EXPECT_DOUBLE_EQ(result.z, expected.z);
}

class TestVectorScalarMultiplicationAssign
    : public ::testing::Test,
      public ::testing::WithParamInterface<std::tuple<Vector, double, Vector>> {
};

INSTANTIATE_TEST_CASE_P(Test, TestVectorScalarMultiplicationAssign,
                        ::testing::ValuesIn(kScalarMultiplicationCases));

TEST_P(TestVectorScalarMultiplicationAssign, Basic) {
  auto values = GetParam();
  auto vector = std::get<0>(values);
  auto scalar = std::get<1>(values);
  auto expected = std::get<2>(values);
  vector *= scalar;
  EXPECT_DOUBLE_EQ(vector.x, expected.x);
  EXPECT_DOUBLE_EQ(vector.y, expected.y);
  EXPECT_DOUBLE_EQ(vector.z, expected.z);
}

class TestVectorScalarDivision
    : public ::testing::Test,
      public ::testing::WithParamInterface<std::tuple<Vector, double, Vector>> {};

INSTANTIATE_TEST_CASE_P(Test, TestVectorScalarDivision,
                        ::testing::ValuesIn(kScalarDivisionCases));

TEST_P(TestVectorScalarDivision, Basic) {
  auto values = GetParam();
  auto vector = std::get<0>(values);
  auto divisor = std::get<1>(values);
  auto expected = std::get<2>(values);
  auto result = vector / divisor;
  EXPECT_DOUBLE_EQ(result.x, expected.x);
  EXPECT_DOUBLE_EQ(result.y, expected.y);
  EXPECT_DOUBLE_EQ(result.z, expected.z);
}

class TestVectorScalarDivisionAssign
    : public ::testing::Test,
      public ::testing::WithParamInterface<std::tuple<Vector, double, Vector>> {};

INSTANTIATE_TEST_CASE_P(Test, TestVectorScalarDivisionAssign,
                        ::testing::ValuesIn(kScalarDivisionCases));

TEST_P(TestVectorScalarDivisionAssign, Basic) {
  auto values = GetParam();
  auto vector = std::get<0>(values);
  auto divisor = std::get<1>(values);
  auto expected = std::get<2>(values);
  vector /= divisor;
  EXPECT_DOUBLE_EQ(vector.x, expected.x);
  EXPECT_DOUBLE_EQ(vector.y, expected.y);
  EXPECT_DOUBLE_EQ(vector.z, expected.z);
}