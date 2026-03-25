#pragma once

#include <array>

namespace hydrolib::controlling {
template <int FREQ_HZ, typename Number>
class PID {
 public:
  PID() = default;

  void SetP(Number p_coeff);
  void SetI(Number i_coeff);
  void SetD(Number d_coeff);

  Number Process(Number input);

 private:
  static constexpr Number kPeriod = 1.0 / FREQ_HZ;
  static constexpr Number kDifferentialPeriod = kPeriod * 4;

  static constexpr Number kCurrentOutputCoeff =
      (kDifferentialPeriod * 2) + kPeriod;
  static constexpr Number kPrevOutputCoeff = -kDifferentialPeriod * 4;
  static constexpr Number kPrevPrevOutputCoeff =
      (kDifferentialPeriod * 2) - kPeriod;

  void RecalculateCoefficients();

  Number p_ = 0;
  Number i_ = 0;
  Number d_ = 0;

  Number current_input_coeff_ = 0;
  Number prev_input_coeff_ = 0;
  Number prev_prev_input_coeff_ = 0;

  std::array<Number, 3> input_ = {};
  std::array<Number, 2> output_ = {};
};

template <int FREQ_HZ, typename Number>
void PID<FREQ_HZ, Number>::SetP(Number p_coeff) {
  p_ = p_coeff;
  RecalculateCoefficients();
}

template <int FREQ_HZ, typename Number>
void PID<FREQ_HZ, Number>::SetI(Number i_coeff) {
  i_ = i_coeff;
  RecalculateCoefficients();
}

template <int FREQ_HZ, typename Number>
void PID<FREQ_HZ, Number>::SetD(Number d_coeff) {
  d_ = d_coeff;
  RecalculateCoefficients();
}

template <int FREQ_HZ, typename Number>
Number PID<FREQ_HZ, Number>::Process(Number input) {
  input_[2] = input_[1];
  input_[1] = input_[0];
  input_[0] = input;

  Number current_output =
      (current_input_coeff_ * input_[0] + prev_input_coeff_ * input_[1] +
       prev_prev_input_coeff_ * input_[2] - kPrevOutputCoeff * output_[0] -
       kPrevPrevOutputCoeff * output_[1]) /
      kCurrentOutputCoeff;

  output_[1] = output_[0];
  output_[0] = current_output;

  return current_output;
}

template <int FREQ_HZ, typename Number>
void PID<FREQ_HZ, Number>::RecalculateCoefficients() {
  current_input_coeff_ =
      (p_ + i_ * (kDifferentialPeriod + kPeriod / 2)) * kPeriod +
      (d_ + p_ * kDifferentialPeriod) * 2;
  prev_input_coeff_ =
      i_ * kPeriod * kPeriod - (d_ + p_ * kDifferentialPeriod) * 4;
  prev_prev_input_coeff_ =
      (-p_ + i_ * (-kDifferentialPeriod + kPeriod / 2)) * kPeriod +
      (d_ + p_ * kDifferentialPeriod) * 2;
}
}  // namespace hydrolib::controlling
