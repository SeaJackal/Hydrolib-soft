#pragma once

#include <array>
#include <climits>
#include <cmath>
#include <concepts>
#include <numbers>

namespace hydrolib::math {
template <int FRACTION_BITS>
class FixedPoint;
}

template <int FRACTION_BITS>
hydrolib::math::FixedPoint<FRACTION_BITS> sin(
    hydrolib::math::FixedPoint<FRACTION_BITS> value_rad);

template <int FRACTION_BITS>
hydrolib::math::FixedPoint<FRACTION_BITS> cos(
    hydrolib::math::FixedPoint<FRACTION_BITS> value_rad);

template <int FRACTION_BITS>
hydrolib::math::FixedPoint<FRACTION_BITS> sqrt(
    hydrolib::math::FixedPoint<FRACTION_BITS>
        value);  // TODO(vscode): fix accuracy

namespace hydrolib::math {
template <typename T>
concept ArithmeticConcept = requires(T first, T second) {
  { first + second } -> std::convertible_to<T>;
  { first - second } -> std::convertible_to<T>;
  { first * second } -> std::convertible_to<T>;
  { sqrt(first) } -> std::convertible_to<T>;
  { first / second } -> std::convertible_to<T>;
  { -first } -> std::convertible_to<T>;
  { first += second } -> std::same_as<T&>;
  { first -= second } -> std::same_as<T&>;
  { first *= second } -> std::same_as<T&>;
  { first /= second } -> std::same_as<T&>;
};

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS> DegToRad(
    FixedPoint<FRACTION_BITS> value_deg);

template <int FRACTION_BITS>
class FixedPoint {
  friend FixedPoint sqrt<FRACTION_BITS>(FixedPoint value);
  friend FixedPoint sin<FRACTION_BITS>(FixedPoint value_rad);
  friend FixedPoint cos<FRACTION_BITS>(FixedPoint value_rad);

 public:
  static constexpr int kFractionBits = FRACTION_BITS;
  static constexpr int kUpperNotIncludedBound = (INT_MAX >> FRACTION_BITS) + 1;
  static constexpr int kLowerNotIncludedBound = INT_MIN >> FRACTION_BITS;
  static constexpr double kLeastBitValue = 1.0 / (1 << FRACTION_BITS);

  constexpr FixedPoint();
  constexpr FixedPoint(int value);  // NOLINT
  constexpr FixedPoint(int value, int divider);
  consteval FixedPoint(float value);        // NOLINT
  consteval FixedPoint(double value);       // NOLINT
  consteval FixedPoint(long double value);  // NOLINT

  explicit operator double() const;
  explicit operator int() const;

  constexpr FixedPoint& operator+=(const FixedPoint& other);
  constexpr FixedPoint& operator-=(const FixedPoint& other);
  constexpr FixedPoint operator-() const;
  constexpr FixedPoint operator+(const FixedPoint& other) const;
  constexpr FixedPoint operator-(const FixedPoint& other) const;
  constexpr FixedPoint operator*(const FixedPoint& other) const;
  constexpr FixedPoint operator/(const FixedPoint& other) const;
  constexpr FixedPoint& operator*=(const FixedPoint& other);
  constexpr FixedPoint& operator/=(const FixedPoint& other);

  constexpr bool operator==(const FixedPoint& other) const;
  constexpr bool operator!=(const FixedPoint& other) const;
  constexpr bool operator<(const FixedPoint& other) const;
  constexpr bool operator<=(const FixedPoint& other) const;
  constexpr bool operator>(const FixedPoint& other) const;
  constexpr bool operator>=(const FixedPoint& other) const;

  [[nodiscard]] constexpr FixedPoint Abs() const;

  [[nodiscard]] constexpr int GetAbsIntPart() const;
  [[nodiscard]] constexpr int GetAbsFractionPart() const;

 private:
  int value_;
};

class CORDICSupporter {
  friend class CORDICConstants;

 private:
  static constexpr int kAtansCount = 20;
  using AtansArray = std::array<int, kAtansCount>;

  static constexpr int kTrigonometryFractionBits = (sizeof(int) * 8) - 3;

  static constexpr AtansArray CountAtans() {
    AtansArray atans = {};
    for (int i = 0; i < kAtansCount; i++) {
      atans[i] = static_cast<int>(std::atan(1.0 / (1 << i)) *
                                  (1 << kTrigonometryFractionBits));
    }
    return atans;
  }
  static constexpr int CountXShift() {
    double x_shift = 1;
    for (int i = 0; i < kAtansCount; i++) {
      x_shift *= std::cos(std::atan(1.0 / (1 << i)));
    }
    return static_cast<int>(x_shift * (1 << kTrigonometryFractionBits));
  }
};

class CORDICConstants {
 public:
  static constexpr CORDICSupporter::AtansArray kAtans =
      CORDICSupporter::CountAtans();
  static constexpr int kXShift = CORDICSupporter::CountXShift();
  static constexpr int kTrigonometryFractionBits =
      CORDICSupporter::kTrigonometryFractionBits;
};

using FixedPointBase = FixedPoint<16>;  // NOLINT

consteval FixedPointBase operator""_fp(long double value);

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS>::FixedPoint() : value_(0) {}

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS>::FixedPoint(int value)
    : value_(value << FRACTION_BITS) {}

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS>::FixedPoint(int value, int divider)
    : value_((static_cast<int64_t>(value) << FRACTION_BITS) / divider) {}

template <int FRACTION_BITS>
consteval FixedPoint<FRACTION_BITS>::FixedPoint(float value)
    : value_(static_cast<int>(value * (1 << FRACTION_BITS))) {}

template <int FRACTION_BITS>
consteval FixedPoint<FRACTION_BITS>::FixedPoint(double value)
    : value_(static_cast<int>(value * (1 << FRACTION_BITS))) {}

template <int FRACTION_BITS>
consteval FixedPoint<FRACTION_BITS>::FixedPoint(long double value)
    : value_(static_cast<int>(value * (1 << FRACTION_BITS))) {}

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS> FixedPoint<FRACTION_BITS>::Abs() const {
  FixedPoint<FRACTION_BITS> result;
  result.value_ = value_ < 0 ? -value_ : value_;
  return result;
}

template <int FRACTION_BITS>
constexpr int FixedPoint<FRACTION_BITS>::GetAbsIntPart() const {
  return (value_ < 0 ? -value_ : value_) >> FRACTION_BITS;
}

template <int FRACTION_BITS>
constexpr int FixedPoint<FRACTION_BITS>::GetAbsFractionPart() const {
  if (value_ < 0) {
    return (~(value_ - 1)) & ((1 << FRACTION_BITS) - 1);
  }
  return value_ & ((1 << FRACTION_BITS) - 1);
}

template <int FRACTION_BITS>
FixedPoint<FRACTION_BITS>::operator double() const {
  return static_cast<double>(value_) / (1 << FRACTION_BITS);
}

template <int FRACTION_BITS>
FixedPoint<FRACTION_BITS>::operator int() const {
  if (value_ < 0) {
    return -((-value_) >> FRACTION_BITS);
  }
  return value_ >> FRACTION_BITS;
}

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS>& FixedPoint<FRACTION_BITS>::operator+=(
    const FixedPoint<FRACTION_BITS>& other) {
  value_ += other.value_;
  return *this;
}

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS>& FixedPoint<FRACTION_BITS>::operator-=(
    const FixedPoint<FRACTION_BITS>& other) {
  *this += -other;
  return *this;
}

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS> FixedPoint<FRACTION_BITS>::operator-()
    const {
  FixedPoint<FRACTION_BITS> result = *this;
  result.value_ = -result.value_;
  return result;
}

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS> FixedPoint<FRACTION_BITS>::operator+(
    const FixedPoint<FRACTION_BITS>& other) const {
  FixedPoint<FRACTION_BITS> result = *this;
  result += other;
  return result;
}

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS> FixedPoint<FRACTION_BITS>::operator-(
    const FixedPoint<FRACTION_BITS>& other) const {
  FixedPoint<FRACTION_BITS> result = *this;
  result += -other;
  return result;
}

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS> FixedPoint<FRACTION_BITS>::operator*(
    const FixedPoint<FRACTION_BITS>& other) const {
  FixedPoint<FRACTION_BITS> result = *this;
  int64_t result_value =
      (static_cast<int64_t>(result.value_) * other.value_) >> FRACTION_BITS;
  result.value_ = static_cast<int>(result_value);
  return result;
}

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS> FixedPoint<FRACTION_BITS>::operator/(
    const FixedPoint<FRACTION_BITS>& other) const {
  FixedPoint<FRACTION_BITS> result = *this;
  int64_t result_value =
      (static_cast<int64_t>(result.value_) << FRACTION_BITS) / other.value_;
  result.value_ = static_cast<int>(result_value);
  return result;
}

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS>& FixedPoint<FRACTION_BITS>::operator*=(
    const FixedPoint<FRACTION_BITS>& other) {
  *this = *this * other;
  return *this;
}

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS>& FixedPoint<FRACTION_BITS>::operator/=(
    const FixedPoint<FRACTION_BITS>& other) {
  *this = *this / other;
  return *this;
}

template <int FRACTION_BITS>
constexpr bool FixedPoint<FRACTION_BITS>::operator==(
    const FixedPoint& other) const {
  return value_ == other.value_;
}

template <int FRACTION_BITS>
constexpr bool FixedPoint<FRACTION_BITS>::operator!=(
    const FixedPoint& other) const {
  return value_ != other.value_;
}

template <int FRACTION_BITS>
constexpr bool FixedPoint<FRACTION_BITS>::operator<(
    const FixedPoint& other) const {
  return value_ < other.value_;
}

template <int FRACTION_BITS>
constexpr bool FixedPoint<FRACTION_BITS>::operator<=(
    const FixedPoint& other) const {
  return value_ <= other.value_;
}

template <int FRACTION_BITS>
constexpr bool FixedPoint<FRACTION_BITS>::operator>(
    const FixedPoint& other) const {
  return value_ > other.value_;
}

template <int FRACTION_BITS>
constexpr bool FixedPoint<FRACTION_BITS>::operator>=(
    const FixedPoint& other) const {
  return value_ >= other.value_;
}

consteval FixedPointBase operator""_fp(long double value) { return {value}; }

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS> kPi = std::numbers::pi_v<long double>;
constexpr FixedPointBase kDegsInPi = 180;

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS> DegToRad(
    FixedPoint<FRACTION_BITS> value_deg) {
  return value_deg * kPi<FRACTION_BITS> / kDegsInPi;
}

}  // namespace hydrolib::math

template <int FRACTION_BITS>
hydrolib::math::FixedPoint<FRACTION_BITS> sin(
    hydrolib::math::FixedPoint<FRACTION_BITS> value_rad) {
  auto full_circles =
      static_cast<int>(value_rad / (hydrolib::math::kPi<FRACTION_BITS> * 2));

  constexpr std::array<hydrolib::math::FixedPoint<FRACTION_BITS>, 7>
      kMultiplesOfPi = {std::numbers::pi_v<long double> * 2,
                        std::numbers::pi_v<long double> * 4,
                        std::numbers::pi_v<long double> * 8,
                        std::numbers::pi_v<long double> * 16,
                        std::numbers::pi_v<long double> * 32,
                        std::numbers::pi_v<long double> * 64,
                        std::numbers::pi_v<long double> * 128};
  hydrolib::math::FixedPoint<FRACTION_BITS> normalized_value_rad = value_rad;
  int multiple_index = kMultiplesOfPi.size() - 1;
  while (full_circles != 0) {
    if (std::abs(full_circles) >= (1 << multiple_index)) {
      if (full_circles > 0) {
        normalized_value_rad -= kMultiplesOfPi[multiple_index];
        full_circles -= 1 << multiple_index;
      } else {
        normalized_value_rad += kMultiplesOfPi[multiple_index];
        full_circles += 1 << multiple_index;
      }
    } else {
      multiple_index--;
    }
  }

  if (normalized_value_rad > hydrolib::math::kPi<FRACTION_BITS>) {
    normalized_value_rad -= hydrolib::math::kPi<FRACTION_BITS> * 2;
  } else if (normalized_value_rad < -hydrolib::math::kPi<FRACTION_BITS>) {
    normalized_value_rad += hydrolib::math::kPi<FRACTION_BITS> * 2;
  }

  if (normalized_value_rad > hydrolib::math::kPi<FRACTION_BITS> / 2) {
    normalized_value_rad =
        hydrolib::math::kPi<FRACTION_BITS> - normalized_value_rad;
  } else if (normalized_value_rad < -hydrolib::math::kPi<FRACTION_BITS> / 2) {
    normalized_value_rad =
        -hydrolib::math::kPi<FRACTION_BITS> - normalized_value_rad;
  }

  int current_diff =
      normalized_value_rad.value_ *
      (1 << (hydrolib::math::CORDICConstants::kTrigonometryFractionBits -
             FRACTION_BITS));
  int x_coordinate = hydrolib::math::CORDICConstants::kXShift;
  int y_coordinate = 0;

  const auto& kAtans = hydrolib::math::CORDICConstants::kAtans;

  for (int i = 0; i < static_cast<int>(kAtans.size()); i++) {
    int new_x_coordinate = 0;
    int new_y_coordinate = 0;
    if (current_diff > 0) {
      current_diff -= kAtans[i];
      new_x_coordinate = x_coordinate - (y_coordinate >> i);
      new_y_coordinate = y_coordinate + (x_coordinate >> i);
    } else if (current_diff < 0) {
      current_diff += kAtans[i];
      new_x_coordinate = x_coordinate + (y_coordinate >> i);
      new_y_coordinate = y_coordinate - (x_coordinate >> i);
    } else {
      break;
    }
    x_coordinate = new_x_coordinate;
    y_coordinate = new_y_coordinate;
  }
  return hydrolib::math::FixedPoint<FRACTION_BITS>(
      y_coordinate,
      1 << hydrolib::math::CORDICConstants::kTrigonometryFractionBits);
}

template <int FRACTION_BITS>
hydrolib::math::FixedPoint<FRACTION_BITS> cos(
    hydrolib::math::FixedPoint<FRACTION_BITS> value_rad) {
  if (value_rad > 0) {
    value_rad -= hydrolib::math::kPi<FRACTION_BITS> * 2;
  } else if (value_rad < 0) {
    value_rad += hydrolib::math::kPi<FRACTION_BITS> * 2;
  }
  return sin(value_rad + (hydrolib::math::kPi<FRACTION_BITS> / 2));
}

template <int FRACTION_BITS>
hydrolib::math::FixedPoint<FRACTION_BITS> sqrt(
    hydrolib::math::FixedPoint<FRACTION_BITS> value) {
  if (value.value_ == 0) {
    return hydrolib::math::FixedPoint<FRACTION_BITS>(0);
  }

  int64_t target_sq = static_cast<int64_t>(value.value_) << FRACTION_BITS;

  int low = 0;
  int high = INT_MAX;
  int mid = high / 2;

  while (low + 1 < high) {
    int64_t square = static_cast<int64_t>(mid) * mid;
    if (square > target_sq) {
      high = mid;
    } else if (square < target_sq) {
      low = mid;
    } else {
      value.value_ = mid;
      return value;
    }
    mid = low + ((high - low) / 2);
  }
  if ((static_cast<int64_t>(high) * high) - target_sq <
      target_sq - (static_cast<int64_t>(low) * low)) {
    value.value_ = high;
  } else {
    value.value_ = low;
  }
  return value;
}
