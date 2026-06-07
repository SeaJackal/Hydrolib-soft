#include <gtest/gtest.h>

#include "hydrolib_acs.hpp"
#include "hydrolib_fixed_point.hpp"
#include "hydrolib_thrust_generator.hpp"
#include "hydrolib_vector3d.hpp"

using hydrolib::controlling::Control;
using hydrolib::controlling::ControlSystem;
using hydrolib::math::FixedPoint;
using hydrolib::math::FixedPointBase;
using hydrolib::math::Rotation;
using hydrolib::math::Vector3D;

template <typename Number>
class TestROV {
 public:
  static constexpr Number kTauX = Number{0.5};
  static constexpr Number kTauY = Number{0.1};
  static constexpr Number kTauZ = Number{0.3};
  static constexpr Number kPeriod = Number{0.01};

  Rotation<Number> GetOrientation() { return orientation_; }

  void ProcessWithFeedback(Control<Number> control) {
    control.x_torque =
        ProcessAperiodic(x_torque_filter_, control.x_torque, kTauX);
    control.y_torque =
        ProcessAperiodic(y_torque_filter_, control.y_torque, kTauY);
    control.z_torque =
        ProcessAperiodic(z_torque_filter_, control.z_torque, kTauZ);

    ApplyRotation(control);
  }

  Vector3D<Number> GetXAxis() {
    return orientation_.Rotate(
        {.x = Number{1}, .y = Number{0}, .z = Number{0}});
  }

  Vector3D<Number> GetYAxis() {
    return orientation_.Rotate(
        {.x = Number{0}, .y = Number{1}, .z = Number{0}});
  }

  Vector3D<Number> GetZAxis() {
    return orientation_.Rotate(
        {.x = Number{0}, .y = Number{0}, .z = Number{1}});
  }

 private:
  struct AperiodicState {
    Number last_input = 0;
    Number last_output = 0;
  };

  static Number ProcessAperiodic(AperiodicState& state, Number input,
                                 Number tau) {
    auto numerator = (kPeriod * (input + state.last_input)) -
                     ((kPeriod - (tau * 2)) * state.last_output);
    auto output = numerator / (kPeriod + (tau * 2));
    state.last_input = input;
    state.last_output = output;
    return output;
  }

  void ApplyRotation(const Control<Number>& control) {
    Vector3D<Number> rotation_vector = {.x = control.x_torque * kPeriod,
                                        .y = control.y_torque * kPeriod,
                                        .z = control.z_torque * kPeriod};

    orientation_ += Rotation<Number>(rotation_vector,
                                     Vector3D<Number>::Length(rotation_vector));
    orientation_.Normalize();
  }

  Rotation<Number> orientation_;
  AperiodicState x_torque_filter_;
  AperiodicState y_torque_filter_;
  AperiodicState z_torque_filter_;
};

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

template <typename Number>
std::string ToString(Number value) {
  return std::to_string(static_cast<double>(value));
}

template <typename Number>
class TestACSParametrized : public ::testing::Test,
                            public ::testing::WithParamInterface<
                                std::tuple<Vector3D<Number>, Number>> {
 public:
  static constexpr int kFreqHz = 100;

  static constexpr std::array kAxisCases = {
      Vector3D<Number>{.x = Number{1}, .y = Number{0}, .z = Number{0}},
      Vector3D<Number>{.x = Number{0}, .y = Number{1}, .z = Number{0}},
      Vector3D<Number>{.x = Number{0}, .y = Number{0}, .z = Number{1}},
      Vector3D<Number>{.x = Number{1}, .y = Number{1}, .z = Number{1}},
      Vector3D<Number>{.x = Number{0.5}, .y = Number{0.6}, .z = Number{0.7}},
  };
  static constexpr std::array kAngleCases = {
      Number{1.0}, Number{3.14}, Number{0.5}, Number{-0.3}};

  void RunControlTest() {
    auto values = this->GetParam();
    auto axis = std::get<0>(values);
    auto angle = std::get<1>(values);

    auto target = Rotation<Number>(axis, angle);

    auto target_x =
        target.Rotate({.x = Number{1}, .y = Number{0}, .z = Number{0}});
    auto target_y =
        target.Rotate({.x = Number{0}, .y = Number{1}, .z = Number{0}});
    auto target_z =
        target.Rotate({.x = Number{0}, .y = Number{0}, .z = Number{1}});

    std::cout << "x_target:" << static_cast<double>(target_x.x) << " "
              << static_cast<double>(target_x.y) << " "
              << static_cast<double>(target_x.z) << "\n";
    std::cout << "y_target:" << static_cast<double>(target_y.x) << " "
              << static_cast<double>(target_y.y) << " "
              << static_cast<double>(target_y.z) << "\n";
    std::cout << "z_target:" << static_cast<double>(target_z.x) << " "
              << static_cast<double>(target_z.y) << " "
              << static_cast<double>(target_z.z) << "\n";
    std::cout << "\n";

    control_system.SetYawP(Number{3});
    control_system.SetYawI(Number{0.01});
    control_system.SetYawD(Number{2});

    control_system.SetPitchP(Number{6.25});
    control_system.SetPitchI(Number{0});
    control_system.SetPitchD(Number{0.1});

    control_system.SetRollP(Number{2});
    control_system.SetRollI(Number{0});
    control_system.SetRollD(Number{0.5});

    control_system.SetControl(target,
                              {.x = Number{0}, .y = Number{0}, .z = Number{0}});

    for (int i = 0; i < 1000; i++) {
      control_system.Process();
      std::cout << "x:" << static_cast<double>(rov.GetXAxis().x) << " "
                << static_cast<double>(rov.GetXAxis().y) << " "
                << static_cast<double>(rov.GetXAxis().z) << "\n";
      std::cout << "y:" << static_cast<double>(rov.GetYAxis().x) << " "
                << static_cast<double>(rov.GetYAxis().y) << " "
                << static_cast<double>(rov.GetYAxis().z) << "\n";
      std::cout << "z:" << static_cast<double>(rov.GetZAxis().x) << " "
                << static_cast<double>(rov.GetZAxis().y) << " "
                << static_cast<double>(rov.GetZAxis().z) << "\n";
      std::cout << "\n";
    }

    EXPECT_NEAR(static_cast<double>(rov.GetXAxis().x),
                static_cast<double>(target_x.x), 0.05);
    EXPECT_NEAR(static_cast<double>(rov.GetXAxis().y),
                static_cast<double>(target_x.y), 0.05);
    EXPECT_NEAR(static_cast<double>(rov.GetYAxis().x),
                static_cast<double>(target_y.x), 0.05);
    EXPECT_NEAR(static_cast<double>(rov.GetYAxis().y),
                static_cast<double>(target_y.y), 0.05);
  }

  TestROV<Number> rov;
  ControlSystem<TestROV<Number>, TestROV<Number>, kFreqHz, Number>
      control_system{rov, rov};
};

using TestACSParametrizedDouble = TestACSParametrized<double>;
using TestACSParametrizedFixedPoint = TestACSParametrized<FixedPoint<22>>;

INSTANTIATE_TEST_CASE_P(
    TestDouble, TestACSParametrizedDouble,
    ::testing::Combine(
        ::testing::ValuesIn(TestACSParametrizedDouble::kAxisCases),
        ::testing::ValuesIn(TestACSParametrizedDouble::kAngleCases)),
    [](const testing::TestParamInfo<TestACSParametrizedDouble::ParamType>&
           info) {
      auto values = info.param;
      auto axis = std::get<0>(values);
      auto angle = std::get<1>(values);
      return "axis_" + Sanitize(ToString(axis.x)) + "_" +
             Sanitize(ToString(axis.y)) + "_" + Sanitize(ToString(axis.z)) +
             "_angle_" + Sanitize(ToString(angle));
    });

INSTANTIATE_TEST_CASE_P(
    TestFixedPoint, TestACSParametrizedFixedPoint,
    ::testing::Combine(
        ::testing::ValuesIn(TestACSParametrizedFixedPoint::kAxisCases),
        ::testing::ValuesIn(TestACSParametrizedFixedPoint::kAngleCases)),
    [](const testing::TestParamInfo<TestACSParametrizedFixedPoint::ParamType>&
           info) {
      auto values = info.param;
      auto axis = std::get<0>(values);
      auto angle = std::get<1>(values);
      return "axis_" + Sanitize(ToString(axis.x)) + "_" +
             Sanitize(ToString(axis.y)) + "_" + Sanitize(ToString(axis.z)) +
             "_angle_" + Sanitize(ToString(angle));
    });

TEST_P(TestACSParametrizedDouble, ControlTest) { RunControlTest(); }

TEST_P(TestACSParametrizedFixedPoint, ControlTest) { RunControlTest(); }