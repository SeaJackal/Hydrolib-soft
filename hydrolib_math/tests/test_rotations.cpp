#include <gtest/gtest.h>

#include <array>
#include <tuple>

#include "hydrolib_fixed_point.hpp"
#include "hydrolib_rotations.hpp"
#include "hydrolib_vector3d.hpp"

using hydrolib::math::FixedPointBase;
using hydrolib::math::Rotation;
using hydrolib::math::Vector3D;

constexpr std::array<Vector3D<double>, 10> kVector3DCases{{
    Vector3D<double>{.x = 1, .y = 1, .z = 1},
    Vector3D<double>{.x = 9, .y = -2, .z = 4},
    Vector3D<double>{.x = 1, .y = 0, .z = 0},
    Vector3D<double>{.x = 0, .y = 1, .z = 0},
    Vector3D<double>{.x = 0, .y = 0, .z = 1},
    Vector3D<double>{.x = 1, .y = 2, .z = 3},
    Vector3D<double>{.x = -1, .y = -2, .z = -3},
    Vector3D<double>{.x = 1, .y = -2, .z = 3},
    Vector3D<double>{.x = -1, .y = 2, .z = -3},
    Vector3D<double>{.x = -1, .y = -1, .z = -1},
}};

constexpr std::array<double, 10> kAngleCases{{
    0,
    M_PI / 2,
    M_PI,
    3 * M_PI / 2,
    2 * M_PI,
    M_PI / 4,
    -M_PI / 4,
    M_PI / 3,
    -M_PI / 3,
    M_PI / 6,
}};

class TestRotation : public ::testing::Test,
                     public ::testing::WithParamInterface<
                         std::tuple<Vector3D<double>, Vector3D<double>>> {};

INSTANTIATE_TEST_CASE_P(
    Test, TestRotation,
    ::testing::Combine(::testing::ValuesIn(kVector3DCases),
                       ::testing::ValuesIn(kVector3DCases)),
    [](const testing::TestParamInfo<TestRotation::ParamType>& info) {
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
      return "from_" + sanitize(std::to_string(first.x)) + "_" +
             sanitize(std::to_string(first.y)) + "_" +
             sanitize(std::to_string(first.z)) + "_to_" +
             sanitize(std::to_string(second.x)) + "_" +
             sanitize(std::to_string(second.y)) + "_" +
             sanitize(std::to_string(second.z));
    });

TEST_P(TestRotation, Basic) {
  auto values = GetParam();
  auto from_vec = std::get<0>(values);
  auto to_vec = std::get<1>(values);

  Vector3D<double>::Normalize(to_vec);
  to_vec *= Vector3D<double>::Length(from_vec);

  Rotation<double> rotation(from_vec, to_vec);
  auto result = rotation.Rotate(from_vec);
  EXPECT_NEAR(to_vec.x, result.x, 1e-10);
  EXPECT_NEAR(to_vec.y, result.y, 1e-10);
  EXPECT_NEAR(to_vec.z, result.z, 1e-10);
}

class TestRotationAxisAngle
    : public ::testing::Test,
      public ::testing::WithParamInterface<
          std::tuple<Vector3D<double>, Vector3D<double>, double>> {};

INSTANTIATE_TEST_CASE_P(
    Test, TestRotationAxisAngle,
    ::testing::Combine(::testing::ValuesIn(kVector3DCases),
                       ::testing::ValuesIn(kVector3DCases),
                       ::testing::ValuesIn(kAngleCases)),
    [](const testing::TestParamInfo<TestRotationAxisAngle::ParamType>& info) {
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
      auto third = std::get<2>(values);
      return "vector_" + sanitize(std::to_string(first.x)) + "_" +
             sanitize(std::to_string(first.y)) + "_" +
             sanitize(std::to_string(first.z)) + "axis_" +
             sanitize(std::to_string(second.x)) + "_" +
             sanitize(std::to_string(second.y)) + "_" +
             sanitize(std::to_string(second.z)) + "_angle_" +
             sanitize(std::to_string(third));
    });

TEST_P(TestRotationAxisAngle, Basic) {
  auto values = GetParam();
  auto vector = std::get<0>(values);
  auto axis = std::get<1>(values);
  auto angle = std::get<2>(values);
  Rotation<double> rotation(axis, angle);
  Rotation<double> reverse_rotation(axis, -angle);
  auto result = rotation.Rotate(vector);
  auto reverse_result = reverse_rotation.Rotate(result);
  EXPECT_NEAR(vector.x, reverse_result.x, 1e-10);
  EXPECT_NEAR(vector.y, reverse_result.y, 1e-10);
  EXPECT_NEAR(vector.z, reverse_result.z, 1e-10);
}

TEST_P(TestRotation, ExtractZComponentSavesRotation) {
  auto values = GetParam();
  auto from_vec = std::get<0>(values);
  auto to_vec = std::get<1>(values);

  Vector3D<double>::Normalize(to_vec);
  to_vec *= Vector3D<double>::Length(from_vec);

  Rotation<double> rotation(from_vec, to_vec);

  auto z_component = rotation.ExtractZComponent();
  auto z_axis = z_component.GetAxis();
  EXPECT_NEAR(z_axis.x, 0, 1e-10);
  EXPECT_NEAR(z_axis.y, 0, 1e-10);
  auto xy_axis = rotation.GetAxis();
  EXPECT_NEAR(xy_axis.z, 0, 1e-10);

  auto result = rotation.Rotate(from_vec);
  result = z_component.Rotate(result);
  EXPECT_NEAR(to_vec.x, result.x, 1e-10);
  EXPECT_NEAR(to_vec.y, result.y, 1e-10);
  EXPECT_NEAR(to_vec.z, result.z, 1e-10);
}

class TestCornerRotation
    : public ::testing::Test,
      public ::testing::WithParamInterface<Vector3D<double>> {};

INSTANTIATE_TEST_CASE_P(
    Test, TestCornerRotation, ::testing::ValuesIn(kVector3DCases),
    [](const testing::TestParamInfo<TestCornerRotation::ParamType>& info) {
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
      return sanitize(std::to_string(value.x)) + "_" +
             sanitize(std::to_string(value.y)) + "_" +
             sanitize(std::to_string(value.z));
    });

constexpr double kNearEqualTolerance = 0.00001;

TEST_P(TestCornerRotation, NearEqual) {
  auto vector = GetParam();
  Vector3D<double>::Normalize(vector);
  auto near_vector = vector;
  near_vector.x += kNearEqualTolerance;
  near_vector.y += kNearEqualTolerance;
  near_vector.z += kNearEqualTolerance;
  Vector3D<double>::Normalize(near_vector);

  Rotation<double> rotation(vector, near_vector);
  auto result = rotation.Rotate(vector);
  EXPECT_NEAR(result.x, near_vector.x, 1e-10);
  EXPECT_NEAR(result.y, near_vector.y, 1e-10);
  EXPECT_NEAR(result.z, near_vector.z, 1e-10);
}

TEST_P(TestCornerRotation, NearOpposite) {
  auto vector = GetParam();
  Vector3D<double>::Normalize(vector);
  Vector3D<double> near_vector{.x = -vector.x, .y = -vector.y, .z = -vector.z};
  near_vector.x += kNearEqualTolerance;
  near_vector.y += kNearEqualTolerance;
  near_vector.z += kNearEqualTolerance;
  Vector3D<double>::Normalize(near_vector);

  Rotation<double> rotation(vector, near_vector);
  auto result = rotation.Rotate(vector);
  EXPECT_NEAR(result.x, near_vector.x, 2e-10);
  EXPECT_NEAR(result.y, near_vector.y, 2e-10);
  EXPECT_NEAR(result.z, near_vector.z, 2e-10);
}

constexpr std::array<Vector3D<FixedPointBase>, 10> kVector3DFixedPointCases{{
    Vector3D<FixedPointBase>{.x = 1, .y = 1, .z = 1},
    Vector3D<FixedPointBase>{.x = 9, .y = -2, .z = 4},
    Vector3D<FixedPointBase>{.x = 1, .y = 0, .z = 0},
    Vector3D<FixedPointBase>{.x = 0, .y = 1, .z = 0},
    Vector3D<FixedPointBase>{.x = 0, .y = 0, .z = 1},
    Vector3D<FixedPointBase>{.x = 1, .y = 2, .z = 3},
    Vector3D<FixedPointBase>{.x = -1, .y = -2, .z = -3},
    Vector3D<FixedPointBase>{.x = 1, .y = -2, .z = 3},
    Vector3D<FixedPointBase>{.x = -1, .y = 2, .z = -3},
    Vector3D<FixedPointBase>{.x = -1, .y = -1, .z = -1},
}};

class TestCornerRotationFixedPoint
    : public ::testing::Test,
      public ::testing::WithParamInterface<Vector3D<FixedPointBase>> {};

INSTANTIATE_TEST_CASE_P(
    Test, TestCornerRotationFixedPoint,
    ::testing::ValuesIn(kVector3DFixedPointCases),
    [](const testing::TestParamInfo<TestCornerRotationFixedPoint::ParamType>&
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
      return sanitize(std::to_string(static_cast<double>(value.x))) + "_" +
             sanitize(std::to_string(static_cast<double>(value.y))) + "_" +
             sanitize(std::to_string(static_cast<double>(value.z)));
    });

TEST_P(TestCornerRotationFixedPoint, NearEqual) {
  auto vector = GetParam();
  Vector3D<FixedPointBase>::Normalize(vector);
  auto near_vector = vector;
  near_vector.x += kNearEqualTolerance;
  near_vector.y += kNearEqualTolerance;
  near_vector.z += kNearEqualTolerance;
  Vector3D<FixedPointBase>::Normalize(near_vector);

  Rotation<FixedPointBase> rotation(vector, near_vector);
  auto result = rotation.Rotate(vector);
  EXPECT_NEAR(static_cast<double>(result.x), static_cast<double>(near_vector.x),
              FixedPointBase::kLeastBitValue * 10);
  EXPECT_NEAR(static_cast<double>(result.y), static_cast<double>(near_vector.y),
              FixedPointBase::kLeastBitValue * 10);
  EXPECT_NEAR(static_cast<double>(result.z), static_cast<double>(near_vector.z),
              FixedPointBase::kLeastBitValue * 10);
}

TEST_P(TestCornerRotationFixedPoint, NearOpposite) {
  auto vector = GetParam();
  Vector3D<FixedPointBase>::Normalize(vector);
  Vector3D<FixedPointBase> near_vector{
      .x = -vector.x, .y = -vector.y, .z = -vector.z};
  near_vector.x += kNearEqualTolerance;
  near_vector.y += kNearEqualTolerance;
  near_vector.z += kNearEqualTolerance;
  Vector3D<FixedPointBase>::Normalize(near_vector);

  Rotation<FixedPointBase> rotation(vector, near_vector);
  auto result = rotation.Rotate(vector);
  EXPECT_NEAR(static_cast<double>(result.x), static_cast<double>(near_vector.x),
              FixedPointBase::kLeastBitValue * 10);
  EXPECT_NEAR(static_cast<double>(result.y), static_cast<double>(near_vector.y),
              FixedPointBase::kLeastBitValue * 10);
  EXPECT_NEAR(static_cast<double>(result.z), static_cast<double>(near_vector.z),
              FixedPointBase::kLeastBitValue * 10);
}

class TestRotationOperations
    : public ::testing::Test,
      public ::testing::WithParamInterface<
          std::tuple<Vector3D<double>, double, double>> {};

INSTANTIATE_TEST_CASE_P(
    Test, TestRotationOperations,
    ::testing::Combine(::testing::ValuesIn(kVector3DCases),
                       ::testing::ValuesIn(kAngleCases),
                       ::testing::ValuesIn(kAngleCases)),
    [](const testing::TestParamInfo<TestRotationOperations::ParamType>& info) {
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
      auto third = std::get<2>(values);
      return "axis_" + sanitize(std::to_string(first.x)) + "_" +
             sanitize(std::to_string(first.y)) + "_" +
             sanitize(std::to_string(first.z)) + "_angle_" +
             sanitize(std::to_string(second)) + "_angle_" +
             sanitize(std::to_string(third));
    });

constexpr Vector3D<double> kReferenceVector{.x = 1, .y = 1, .z = 1};

TEST_P(TestRotationOperations, Addition) {
  auto values = GetParam();
  auto axis = std::get<0>(values);
  auto first_angle = std::get<1>(values);
  auto second_angle = std::get<2>(values);
  Rotation<double> first_rotation(axis, first_angle);
  Rotation<double> second_rotation(axis, second_angle);
  auto full_rotation = first_rotation + second_rotation;
  auto result = full_rotation.Rotate(kReferenceVector);
  auto expected_result =
      second_rotation.Rotate(first_rotation.Rotate(kReferenceVector));
  EXPECT_NEAR(result.x, expected_result.x, 1e-10);
  EXPECT_NEAR(result.y, expected_result.y, 1e-10);
  EXPECT_NEAR(result.z, expected_result.z, 1e-10);
}

TEST_P(TestRotationOperations, Subtraction) {
  auto values = GetParam();
  auto axis = std::get<0>(values);
  auto full_angle = std::get<1>(values);
  auto second_angle = std::get<2>(values);
  Rotation<double> full_rotation(axis, full_angle);
  Rotation<double> second_rotation(axis, second_angle);
  auto first_rotation = full_rotation - second_rotation;
  auto expected_result = full_rotation.Rotate(kReferenceVector);
  auto result = second_rotation.Rotate(first_rotation.Rotate(kReferenceVector));
  EXPECT_NEAR(result.x, expected_result.x, 1e-10);
  EXPECT_NEAR(result.y, expected_result.y, 1e-10);
  EXPECT_NEAR(result.z, expected_result.z, 1e-10);
}
