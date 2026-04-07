#include <gtest/gtest.h>

#include <cmath>
#include <iostream>
#include <numbers>

#include "hydrolib_fixed_point.hpp"
#include "hydrolib_pid.hpp"

using namespace hydrolib;

TEST(TestPID, HarmonicTest) {
  constexpr unsigned kFreqHz = 128;
  constexpr unsigned kTestTimeS = 5;

  constexpr unsigned kControlAmplitude = 10;
  constexpr unsigned kControlFreqHz = 1;

  constexpr int kPCoef = 1;
  constexpr int kICoef = 100;
  constexpr int kDCoef = 10;

  constexpr double kControlFreqRad = kControlFreqHz * 2 * std::numbers::pi;

  constexpr double kPeriod = 1.0 / kFreqHz;
  constexpr double kDifferentialPeriod = kPeriod * 4;

  controlling::PID<kFreqHz, math::FixedPointBase> pid;
  pid.SetP(kPCoef);
  pid.SetI(kICoef);
  pid.SetD(kDCoef);

  for (unsigned time = 0; time < kFreqHz * kTestTimeS; time++) {
    double sin_value =
        std::sin(static_cast<double>(time) * kControlFreqRad / kFreqHz);
    double cos_value =
        std::cos(static_cast<double>(time) * kControlFreqRad / kFreqHz);
    double control = kControlAmplitude * sin_value;
    constexpr double kCosAmplitude =
        kControlAmplitude *
        (-kICoef / kControlFreqRad +
         kControlFreqRad *
             (kDCoef - kICoef * kDifferentialPeriod * kDifferentialPeriod)) /
        (kDifferentialPeriod * kDifferentialPeriod * kControlFreqRad *
             kControlFreqRad +
         1);
    constexpr double kSinAmplitude =
        kControlAmplitude *
        (kPCoef + (kDCoef + kPCoef * kDifferentialPeriod) *
                      kDifferentialPeriod * kControlFreqRad * kControlFreqRad) /
        (kDifferentialPeriod * kDifferentialPeriod * kControlFreqRad *
             kControlFreqRad +
         1);
    double target_output =
        (kSinAmplitude * sin_value + kCosAmplitude * cos_value) +
        (kControlAmplitude * kICoef / kControlFreqRad);
    auto real_output = pid.Process(math::FixedPointBase(
        static_cast<int>(control * (1 << math::FixedPointBase::kFractionBits)),
        (1 << math::FixedPointBase::kFractionBits)));

    // std::cout << target_output << " : " << static_cast<double>(real_output)
    //           << "\n";

    if (time >= kFreqHz) {
      EXPECT_LT(std::abs(target_output - static_cast<double>(real_output)),
                kControlAmplitude * 0.06);
    }
  }
}

TEST(TestPID, ControlTest) {
  constexpr int kFreqHz = 128;
  constexpr unsigned kTestTimeS = 5;

  constexpr int control = 10;
  constexpr double tau = 1;

  constexpr math::FixedPointBase kPCoef = 1;
  constexpr math::FixedPointBase kICoef = 10;
  constexpr math::FixedPointBase kDCoef = 0.1;

  controlling::PID<kFreqHz, math::FixedPointBase> pid;
  pid.SetP(kPCoef);
  pid.SetI(kICoef);
  pid.SetD(kDCoef);

  double controlled_value = 0;
  double last_control = 0;

  for (unsigned t = 0; t < kFreqHz * kTestTimeS; t++) {
    double current_control =
        static_cast<double>(pid.Process(math::FixedPointBase(
            static_cast<int>((control - controlled_value) *
                             (1 << math::FixedPointBase::kFractionBits)),
            (1 << math::FixedPointBase::kFractionBits))));
    controlled_value = ((last_control + current_control) / kFreqHz -
                        (1.0 / kFreqHz - 2 * tau) * controlled_value) /
                       (2 * tau + 1.0 / kFreqHz);

    last_control = current_control;

    // std::cout << controlled_value << " : 0\n";
  }

  EXPECT_LT(std::abs(controlled_value - control), control * 0.01);
}
