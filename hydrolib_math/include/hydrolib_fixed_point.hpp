#pragma once

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
  { first *second } -> std::convertible_to<T>;
  { sqrt(first) } -> std::convertible_to<T>;
  { first / second } -> std::convertible_to<T>;
  { -first } -> std::convertible_to<T>;
  { first += second } -> std::same_as<T &>;
  { first -= second } -> std::same_as<T &>;
  { first *= second } -> std::same_as<T &>;
  { first /= second } -> std::same_as<T &>;
};

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS> DegToRad(
    FixedPoint<FRACTION_BITS> value_deg);

template <int FRACTION_BITS>
class FixedPoint {
  friend FixedPoint sqrt<FRACTION_BITS>(FixedPoint value);

 public:
  static constexpr int kFractionBits = FRACTION_BITS;
  static constexpr int kUpperNotIncludedBound = (INT_MAX >> FRACTION_BITS) + 1;
  static constexpr int kLowerIncludedBound = INT_MIN >> FRACTION_BITS;
  static constexpr double kLeastBitValue = 1.0 / (1 << FRACTION_BITS);

  constexpr FixedPoint();
  constexpr FixedPoint(int value);  // NOLINT
  constexpr FixedPoint(int value, int divider);
  consteval FixedPoint(float value);        // NOLINT
  consteval FixedPoint(double value);       // NOLINT
  consteval FixedPoint(long double value);  // NOLINT

  explicit operator double() const;

  constexpr FixedPoint &operator+=(const FixedPoint &other);
  constexpr FixedPoint &operator-=(const FixedPoint &other);
  constexpr FixedPoint operator-() const;
  constexpr FixedPoint operator+(const FixedPoint &other) const;
  constexpr FixedPoint operator-(const FixedPoint &other) const;
  constexpr FixedPoint operator*(const FixedPoint &other) const;
  constexpr FixedPoint operator/(const FixedPoint &other) const;
  constexpr FixedPoint &operator*=(const FixedPoint &other);
  constexpr FixedPoint &operator/=(const FixedPoint &other);

  constexpr bool operator==(const FixedPoint &other) const;
  constexpr bool operator!=(const FixedPoint &other) const;
  constexpr bool operator<(const FixedPoint &other) const;
  constexpr bool operator<=(const FixedPoint &other) const;
  constexpr bool operator>(const FixedPoint &other) const;
  constexpr bool operator>=(const FixedPoint &other) const;

  [[nodiscard]] constexpr FixedPoint Abs() const;

  [[nodiscard]] constexpr int GetAbsIntPart() const;
  [[nodiscard]] constexpr int GetAbsFractionPart() const;

 private:
  int value_;
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
constexpr FixedPoint<FRACTION_BITS> &FixedPoint<FRACTION_BITS>::operator+=(
    const FixedPoint<FRACTION_BITS> &other) {
  value_ += other.value_;
  return *this;
}

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS> &FixedPoint<FRACTION_BITS>::operator-=(
    const FixedPoint<FRACTION_BITS> &other) {
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
    const FixedPoint<FRACTION_BITS> &other) const {
  FixedPoint<FRACTION_BITS> result = *this;
  result += other;
  return result;
}

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS> FixedPoint<FRACTION_BITS>::operator-(
    const FixedPoint<FRACTION_BITS> &other) const {
  FixedPoint<FRACTION_BITS> result = *this;
  result += -other;
  return result;
}

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS> FixedPoint<FRACTION_BITS>::operator*(
    const FixedPoint<FRACTION_BITS> &other) const {
  FixedPoint<FRACTION_BITS> result = *this;
  int64_t result_value =
      (static_cast<int64_t>(result.value_) * other.value_) >> FRACTION_BITS;
  result.value_ = static_cast<int>(result_value);
  return result;
}

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS> FixedPoint<FRACTION_BITS>::operator/(
    const FixedPoint<FRACTION_BITS> &other) const {
  FixedPoint<FRACTION_BITS> result = *this;
  int64_t result_value =
      (static_cast<int64_t>(result.value_) << FRACTION_BITS) / other.value_;
  result.value_ = static_cast<int>(result_value);
  return result;
}

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS> &FixedPoint<FRACTION_BITS>::operator*=(
    const FixedPoint<FRACTION_BITS> &other) {
  *this = *this * other;
  return *this;
}

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS> &FixedPoint<FRACTION_BITS>::operator/=(
    const FixedPoint<FRACTION_BITS> &other) {
  *this = *this / other;
  return *this;
}

template <int FRACTION_BITS>
constexpr bool FixedPoint<FRACTION_BITS>::operator==(
    const FixedPoint &other) const {
  return value_ == other.value_;
}

template <int FRACTION_BITS>
constexpr bool FixedPoint<FRACTION_BITS>::operator!=(
    const FixedPoint &other) const {
  return value_ != other.value_;
}

template <int FRACTION_BITS>
constexpr bool FixedPoint<FRACTION_BITS>::operator<(
    const FixedPoint &other) const {
  return value_ < other.value_;
}

template <int FRACTION_BITS>
constexpr bool FixedPoint<FRACTION_BITS>::operator<=(
    const FixedPoint &other) const {
  return value_ <= other.value_;
}

template <int FRACTION_BITS>
constexpr bool FixedPoint<FRACTION_BITS>::operator>(
    const FixedPoint &other) const {
  return value_ > other.value_;
}

template <int FRACTION_BITS>
constexpr bool FixedPoint<FRACTION_BITS>::operator>=(
    const FixedPoint &other) const {
  return value_ >= other.value_;
}

consteval FixedPointBase operator""_fp(long double value) { return {value}; }

constexpr FixedPointBase kPi = std::numbers::pi_v<long double>;
constexpr FixedPointBase kDegsInPi = 180;

template <int FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS> DegToRad(
    FixedPoint<FRACTION_BITS> value_deg) {
  return value_deg * kPi / kDegsInPi;
}

}  // namespace hydrolib::math

template <int FRACTION_BITS>
hydrolib::math::FixedPoint<FRACTION_BITS> sin(
    hydrolib::math::FixedPoint<FRACTION_BITS> value_rad) {
  hydrolib::math::FixedPoint<FRACTION_BITS> result = value_rad;
  int iteration = 3;
  hydrolib::math::FixedPoint<FRACTION_BITS> diff =
      -(value_rad * value_rad * value_rad) / 3 / 2;
  while (diff.Abs() > 0) {
    result += diff;
    iteration += 2;
    diff *= -(value_rad * value_rad) / (iteration * (iteration - 1));
  }
  return result;
}

template <int FRACTION_BITS>
hydrolib::math::FixedPoint<FRACTION_BITS> cos(
    hydrolib::math::FixedPoint<FRACTION_BITS> value_rad) {
  hydrolib::math::FixedPoint<FRACTION_BITS> result = 1;
  int iteration = 2;
  hydrolib::math::FixedPoint<FRACTION_BITS> diff = -(value_rad * value_rad) / 2;
  while (diff.Abs() > 0) {
    result += diff;
    iteration += 2;
    diff *= -(value_rad * value_rad) / (iteration * (iteration - 1));
  }
  return result;
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

  while (low < high) {
    int64_t square = static_cast<int64_t>(mid) * mid;
    if (square > target_sq) {
      high = mid;
    } else if (square < target_sq) {
      low = mid;
    } else {
      value.value_ = mid;
      return value;
    }
    if (high - low == 1) {
      if (static_cast<int64_t>(high) * high - target_sq <
          target_sq - static_cast<int64_t>(low) * low) {
        value.value_ = high;
      } else {
        value.value_ = low;
      }
      return value;
    }
    mid = low + (high - low) / 2;
  }

  return value;
}
