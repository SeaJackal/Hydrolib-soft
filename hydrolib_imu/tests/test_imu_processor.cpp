#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <tuple>

#include "hydrolib_fixed_point.hpp"
#include "hydrolib_imu_processor.hpp"
#include "hydrolib_rotations.hpp"
#include "hydrolib_vector3d.hpp"
#include "mock/raw_imu_mock.hpp"

using hydrolib::math::FixedPointBase;
using hydrolib::math::Rotation;
using hydrolib::math::Vector3D;
using hydrolib::sensors::IMUProcessor;
using hydrolib::sensors::RawIMUMock;

template <typename Number>
class TestEnvironment {
 public:
  static constexpr std::array kAxisCases{
      Vector3D<Number>{.x = 1, .y = 1, .z = 1},
      Vector3D<Number>{.x = 9, .y = -2, .z = 4},
      Vector3D<Number>{.x = 1, .y = 0, .z = 0},
      Vector3D<Number>{.x = 0, .y = 1, .z = 0},
      Vector3D<Number>{.x = 0, .y = 0, .z = 1},
      Vector3D<Number>{.x = 1, .y = 2, .z = 3},
      Vector3D<Number>{.x = -1, .y = -2, .z = -3},
      Vector3D<Number>{.x = 1, .y = -2, .z = 3},
      Vector3D<Number>{.x = -1, .y = 2, .z = -3},
      Vector3D<Number>{.x = -1, .y = -1, .z = -1},
  };

  static constexpr double GetTolerance();

  static void OneStep(Vector3D<Number> axis);
  static void ManySteps(Vector3D<Number> axis, int steps);
};

template <>
constexpr double TestEnvironment<double>::GetTolerance() {
  return 1e-10;
}

template <>
constexpr double TestEnvironment<FixedPointBase>::GetTolerance() {
  return 0.05;
}

template <typename Number>
void TestEnvironment<Number>::OneStep(Vector3D<Number> axis) {
  RawIMUMock<Number> imu_mock;
  Vector3D<Number>::Normalize(axis);

  constexpr double kTolerance = GetTolerance();

  imu_mock.SetTarget(axis, M_PI / 4, 1);

  IMUProcessor<Number, 1.0> imu_processor;

  Rotation<Number> result;

  imu_mock.Step();
  result = imu_processor.Process(
      typename decltype(imu_processor)::AccelG(imu_mock.GetAcceleration()),
      typename decltype(imu_processor)::GyroRadPerS(imu_mock.GetGyro()));
  auto result_x = result.Rotate({.x = 1, .y = 0, .z = 0});
  auto result_y = result.Rotate({.x = 0, .y = 1, .z = 0});
  auto result_z = result.Rotate({.x = 0, .y = 0, .z = 1});
  auto target_x = imu_mock.GetXAxis();
  auto target_y = imu_mock.GetYAxis();
  auto target_z = imu_mock.GetZAxis();
  EXPECT_NEAR(static_cast<double>(result_x.x), static_cast<double>(target_x.x),
              kTolerance);
  EXPECT_NEAR(static_cast<double>(result_x.y), static_cast<double>(target_x.y),
              kTolerance);
  EXPECT_NEAR(static_cast<double>(result_x.z), static_cast<double>(target_x.z),
              kTolerance);
  EXPECT_NEAR(static_cast<double>(result_y.x), static_cast<double>(target_y.x),
              kTolerance);
  EXPECT_NEAR(static_cast<double>(result_y.y), static_cast<double>(target_y.y),
              kTolerance);
  EXPECT_NEAR(static_cast<double>(result_y.z), static_cast<double>(target_y.z),
              kTolerance);
  EXPECT_NEAR(static_cast<double>(result_z.x), static_cast<double>(target_z.x),
              kTolerance);
  EXPECT_NEAR(static_cast<double>(result_z.y), static_cast<double>(target_z.y),
              kTolerance);
  EXPECT_NEAR(static_cast<double>(result_z.z), static_cast<double>(target_z.z),
              kTolerance);
}

template <typename Number>
void TestEnvironment<Number>::ManySteps(Vector3D<Number> axis, int steps) {
  RawIMUMock<Number> imu_mock;
  Vector3D<Number>::Normalize(axis);

  imu_mock.SetTarget(axis, 2 * M_PI, steps * 10);

  IMUProcessor<Number, 1.0> imu_processor;

  Rotation<Number> result;

  double tolerance = GetTolerance() * std::pow(6, steps - 1);

  while (imu_mock.Step()) {
    result = imu_processor.Process(
        typename decltype(imu_processor)::AccelG(imu_mock.GetAcceleration()),
        typename decltype(imu_processor)::GyroRadPerS(imu_mock.GetGyro()));
    auto result_x = result.Rotate({.x = 1, .y = 0, .z = 0});
    auto result_y = result.Rotate({.x = 0, .y = 1, .z = 0});
    auto result_z = result.Rotate({.x = 0, .y = 0, .z = 1});
    auto target_x = imu_mock.GetXAxis();
    auto target_y = imu_mock.GetYAxis();
    auto target_z = imu_mock.GetZAxis();
    EXPECT_NEAR(static_cast<double>(result_x.x),
                static_cast<double>(target_x.x), tolerance);
    EXPECT_NEAR(static_cast<double>(result_x.y),
                static_cast<double>(target_x.y), tolerance);
    EXPECT_NEAR(static_cast<double>(result_x.z),
                static_cast<double>(target_x.z), tolerance);
    EXPECT_NEAR(static_cast<double>(result_y.x),
                static_cast<double>(target_y.x), tolerance);
    EXPECT_NEAR(static_cast<double>(result_y.y),
                static_cast<double>(target_y.y), tolerance);
    EXPECT_NEAR(static_cast<double>(result_y.z),
                static_cast<double>(target_y.z), tolerance);
    EXPECT_NEAR(static_cast<double>(result_z.x),
                static_cast<double>(target_z.x), tolerance);
    EXPECT_NEAR(static_cast<double>(result_z.y),
                static_cast<double>(target_z.y), tolerance);
    EXPECT_NEAR(static_cast<double>(result_z.z),
                static_cast<double>(target_z.z), tolerance);
  }
}

std::string Sanitize(const std::string& input) {
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

class TestIMUProcessorParametrizedOneStepDouble
    : public ::testing::Test,
      public ::testing::WithParamInterface<Vector3D<double>> {};

INSTANTIATE_TEST_CASE_P(
    Test, TestIMUProcessorParametrizedOneStepDouble,
    ::testing::ValuesIn(TestEnvironment<double>::kAxisCases),
    [](const testing::TestParamInfo<
        TestIMUProcessorParametrizedOneStepDouble::ParamType>& info) {
      auto axis = info.param;
      return "axis_" + Sanitize(std::to_string(axis.x)) + "_" +
             Sanitize(std::to_string(axis.y)) + "_" +
             Sanitize(std::to_string(axis.z));
    });

TEST_P(TestIMUProcessorParametrizedOneStepDouble, Basic) {
  auto axis = GetParam();

  TestEnvironment<double>::OneStep(axis);
}

class TestIMUProcessorParametrizedManyStepsDouble
    : public ::testing::Test,
      public ::testing::WithParamInterface<std::tuple<Vector3D<double>, int>> {
};

INSTANTIATE_TEST_CASE_P(
    Test, TestIMUProcessorParametrizedManyStepsDouble,
    ::testing::Combine(::testing::ValuesIn(TestEnvironment<double>::kAxisCases),
                       ::testing::Range(1, 11)),
    [](const testing::TestParamInfo<
        TestIMUProcessorParametrizedManyStepsDouble::ParamType>& info) {
      auto values = info.param;
      auto axis = std::get<0>(values);
      auto steps = std::get<1>(values);
      return "axis_" + Sanitize(std::to_string(axis.x)) + "_" +
             Sanitize(std::to_string(axis.y)) + "_" +
             Sanitize(std::to_string(axis.z)) + "_steps_" +
             std::to_string(steps);
    });

TEST_P(TestIMUProcessorParametrizedManyStepsDouble, Basic) {
  auto values = GetParam();
  auto axis = std::get<0>(values);
  auto steps = std::get<1>(values);

  TestEnvironment<double>::ManySteps(axis, steps);
}

class TestIMUProcessorParametrizedOneStepFixedPoint
    : public ::testing::Test,
      public ::testing::WithParamInterface<Vector3D<FixedPointBase>> {};

INSTANTIATE_TEST_CASE_P(
    Test, TestIMUProcessorParametrizedOneStepFixedPoint,
    ::testing::ValuesIn(TestEnvironment<FixedPointBase>::kAxisCases),
    [](const testing::TestParamInfo<
        TestIMUProcessorParametrizedOneStepFixedPoint::ParamType>& info) {
      auto axis = info.param;
      return "axis_" + Sanitize(std::to_string(static_cast<double>(axis.x))) +
             "_" + Sanitize(std::to_string(static_cast<double>(axis.y))) + "_" +
             Sanitize(std::to_string(static_cast<double>(axis.z)));
    });

TEST_P(TestIMUProcessorParametrizedOneStepFixedPoint, Basic) {
  auto axis = GetParam();

  TestEnvironment<FixedPointBase>::OneStep(axis);
}

class TestIMUProcessorParametrizedManyStepsFixedPoint
    : public ::testing::Test,
      public ::testing::WithParamInterface<
          std::tuple<Vector3D<FixedPointBase>, int>> {};

INSTANTIATE_TEST_CASE_P(
    Test, TestIMUProcessorParametrizedManyStepsFixedPoint,
    ::testing::Combine(
        ::testing::ValuesIn(TestEnvironment<FixedPointBase>::kAxisCases),
        ::testing::Range(1, 11)),
    [](const testing::TestParamInfo<
        TestIMUProcessorParametrizedManyStepsFixedPoint::ParamType>& info) {
      auto values = info.param;
      auto axis = std::get<0>(values);
      auto steps = std::get<1>(values);
      return "axis_" + Sanitize(std::to_string(static_cast<double>(axis.x))) +
             "_" + Sanitize(std::to_string(static_cast<double>(axis.y))) + "_" +
             Sanitize(std::to_string(static_cast<double>(axis.z))) + "_steps_" +
             std::to_string(steps);
    });

TEST_P(TestIMUProcessorParametrizedManyStepsFixedPoint, Basic) {
  auto values = GetParam();
  auto axis = std::get<0>(values);
  auto steps = std::get<1>(values);

  TestEnvironment<FixedPointBase>::ManySteps(axis, steps);
}

// TEST(TestIMUProcessor, DistortionTest) {
//   Quaternion<double> rotation = Quaternion<double>({1, 2, 3, 4});
//   Quaternion<double>::Normalize(rotation);
//   IMUDistorter<double> distorter(rotation, {0.2, 0.3, 0.4});

//   RawIMUMock<double> imu_mock;
//   Vector3D<double> axis{2, 3, 1};
//   Vector3D<double>::Normalize(axis);
//   double angle_rad = 50.0 / 180 * 3.14;
//   imu_mock.SetTarget(axis, angle_rad, 1);
//   while (imu_mock.Step()) {
//   }

//   IMUProcessor<double, 1.0> imu_processor;
//   auto g_z = distorter.DistortAccel({0, 0, 1});
//   auto g_opposite_z = distorter.DistortAccel({0, 0, -1});
//   auto g_x = distorter.DistortAccel({1, 0, 0});
//   imu_processor.Calibrate(g_z, g_opposite_z, g_x);
//   auto g = imu_mock.GetAcceleration();
//   auto g_distorted = distorter.DistortAccel(g);
//   auto result = imu_processor.Process(g_distorted, {0, 0, 0});
//   auto result_g = Rotate({0, 0, -1}, result);
//   EXPECT_NEAR(static_cast<double>(result_g.x), static_cast<double>(g.x),
//   0.01); EXPECT_NEAR(static_cast<double>(result_g.y),
//   static_cast<double>(g.y), 0.01);
//   EXPECT_NEAR(static_cast<double>(result_g.z), static_cast<double>(g.z),
//   0.01);
// }
