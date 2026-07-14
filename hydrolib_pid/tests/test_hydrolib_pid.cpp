#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>

#ifdef OUTPUT_TEST_VALUES
#include <iostream>
#endif
#include <cmath>
#include <iomanip>
#include <numbers>
#include <sstream>
#include <string>
#include <tuple>

#include "hydrolib_fixed_point.hpp"
#include "hydrolib_pid.hpp"

namespace {
using HarmonicParams =
    std::tuple<hydrolib::math::FixedPointBase, hydrolib::math::FixedPointBase,
               hydrolib::math::FixedPointBase, hydrolib::math::FixedPointBase>;

hydrolib::math::FixedPoint<24> ToFixedPoint(double value) {
  constexpr int kScale = (1 << hydrolib::math::FixedPoint<24>::kFractionBits);
  return hydrolib::math::FixedPoint<24>(static_cast<int>(value * kScale),
                                        kScale);
}

class HarmonicTest : public ::testing::TestWithParam<HarmonicParams> {};

std::string FormNameForTest(
    const testing::TestParamInfo<HarmonicTest::ParamType>& info) {
  static const auto sanitize = [](const std::string& input) {
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
  auto control_freq_hz = std::get<0>(values);
  auto p_coef = std::get<1>(values);
  auto i_coef = std::get<2>(values);
  auto d_coef = std::get<3>(values);
  return sanitize(std::to_string(static_cast<double>(control_freq_hz))) + "_" +
         sanitize(std::to_string(static_cast<double>(p_coef))) + "_" +
         sanitize(std::to_string(static_cast<double>(i_coef))) + "_" +
         sanitize(std::to_string(static_cast<double>(d_coef)));
}

constexpr std::array kControlFreqHzCases = {
    hydrolib::math::FixedPointBase(1), hydrolib::math::FixedPointBase(0.1),
    hydrolib::math::FixedPointBase(3), hydrolib::math::FixedPointBase(5.75)};

constexpr std::array kCoefCases = {
    hydrolib::math::FixedPointBase(0),   hydrolib::math::FixedPointBase(1),
    hydrolib::math::FixedPointBase(0.1), hydrolib::math::FixedPointBase(0.5),
    hydrolib::math::FixedPointBase(10),  hydrolib::math::FixedPointBase(50)};
}  // namespace

TEST_P(HarmonicTest, HarmonicResponse) {
  constexpr unsigned kFreqHz = 128;
  constexpr unsigned kTestTimeS = 100;
  constexpr unsigned kControlAmplitude = 10;

  const auto [control_freq_hz_fp, p_fp, i_fp, d_fp] = GetParam();
  const double control_freq_hz = static_cast<double>(control_freq_hz_fp);
  const double p = static_cast<double>(p_fp);
  const double i = static_cast<double>(i_fp);
  const double d = static_cast<double>(d_fp);

  const double control_freq_rad = control_freq_hz * 2 * std::numbers::pi;
  ASSERT_GT(control_freq_rad, 0.0);

  const double period = 1.0 / kFreqHz;
  const double differential_period = period * 4;

  hydrolib::controlling::PID<kFreqHz, hydrolib::math::FixedPoint<24>> pid;
  pid.SetP(hydrolib::math::FixedPoint<24>(p_fp));
  pid.SetI(hydrolib::math::FixedPoint<24>(i_fp));
  pid.SetD(hydrolib::math::FixedPoint<24>(d_fp));

  const double cos_amplitude =
      kControlAmplitude *
      ((-i / control_freq_rad) +
       (control_freq_rad *
        (d - (i * differential_period * differential_period)))) /
      ((differential_period * differential_period * control_freq_rad *
        control_freq_rad) +
       1);
  const double sin_amplitude =
      kControlAmplitude *
      (p + ((d + (p * differential_period)) * differential_period *
            control_freq_rad * control_freq_rad)) /
      ((differential_period * differential_period * control_freq_rad *
        control_freq_rad) +
       1);
  const double dc_offset = kControlAmplitude * i / control_freq_rad;

  const double target_amplitude =
      std::sqrt(sin_amplitude * sin_amplitude + cos_amplitude * cos_amplitude);

  for (unsigned time = 0; time < kFreqHz * kTestTimeS; time++) {
    const double phase = static_cast<double>(time) * control_freq_rad / kFreqHz;
    const double sin_value = std::sin(phase);
    const double cos_value = std::cos(phase);
    const double control = kControlAmplitude * sin_value;

    const double target_output =
        ((sin_amplitude * sin_value) + (cos_amplitude * cos_value)) + dc_offset;

    const auto real_output = pid.Process(ToFixedPoint(control));

#ifndef OUTPUT_TEST_VALUES
    std::cout << target_output << " : " << static_cast<double>(real_output)
              << "\n";
#endif

#ifndef OUTPUT_TEST_VALUES
    if (time >= kFreqHz) {
      EXPECT_NEAR(target_output, static_cast<double>(real_output),
                  target_amplitude * 0.07);
    }
#endif
  }
}

// INSTANTIATE_TEST_SUITE_P(
//     FrequenciesAndCoefs, HarmonicTest,
//     ::testing::Combine(::testing::ValuesIn(kControlFreqHzCases),
//                        ::testing::ValuesIn(kCoefCases),
//                        ::testing::ValuesIn(kCoefCases),
//                        ::testing::ValuesIn(kCoefCases)),
//     FormNameForTest);

INSTANTIATE_TEST_SUITE_P(FrequenciesAndCoefs, HarmonicTest,
                         ::testing::Values(HarmonicParams{
                             hydrolib::math::FixedPointBase(5),
                             hydrolib::math::FixedPointBase(1),
                             hydrolib::math::FixedPointBase(0),
                             hydrolib::math::FixedPointBase(0)}),
                         FormNameForTest);

TEST(TestPID, ControlTest) {
  constexpr int kFreqHz = 128;
  constexpr unsigned kTestTimeS = 5;

  constexpr int kControl = 10;
  constexpr double kTau = 1;

  constexpr hydrolib::math::FixedPointBase kPCoef = 1;
  constexpr hydrolib::math::FixedPointBase kICoef = 10;
  constexpr hydrolib::math::FixedPointBase kDCoef = 0.1;

  hydrolib::controlling::PID<kFreqHz, hydrolib::math::FixedPointBase> pid;
  pid.SetP(kPCoef);
  pid.SetI(kICoef);
  pid.SetD(kDCoef);

  double controlled_value = 0;
  double last_control = 0;

  for (unsigned time = 0; time < kFreqHz * kTestTimeS; time++) {
    double current_control =
        static_cast<double>(pid.Process(hydrolib::math::FixedPointBase(
            static_cast<int>(
                (kControl - controlled_value) *
                (1 << hydrolib::math::FixedPointBase::kFractionBits)),
            (1 << hydrolib::math::FixedPointBase::kFractionBits))));
    controlled_value = (((last_control + current_control) / kFreqHz) -
                        ((1.0 / kFreqHz) - (2 * kTau)) * controlled_value) /
                       (2 * kTau + 1.0 / kFreqHz);

    last_control = current_control;

#ifdef OUTPUT_TEST_VALUES
    std::cout << controlled_value << " : 0\n";
#endif
  }

  EXPECT_LT(std::abs(controlled_value - kControl), kControl * 0.01);
}
