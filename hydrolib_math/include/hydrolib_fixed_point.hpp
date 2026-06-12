#pragma once

#include <array>
#include <charconv>
#include <climits>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <numbers>
#include <string_view>

#include "hydrolib_return_codes.hpp"
#include "hydrolib_stream_concepts.hpp"

namespace hydrolib::math {
template <int kFractionBitsParam>
class FixedPoint;
}

template <int kFractionBitsParam>
hydrolib::math::FixedPoint<kFractionBitsParam> sin(  // NOLINT
    hydrolib::math::FixedPoint<kFractionBitsParam> value_rad);

template <int kFractionBitsParam>
hydrolib::math::FixedPoint<kFractionBitsParam> cos(  // NOLINT
    hydrolib::math::FixedPoint<kFractionBitsParam> value_rad);

template <int kFractionBitsParam>
hydrolib::math::FixedPoint<kFractionBitsParam> sqrt(  // NOLINT
    hydrolib::math::FixedPoint<kFractionBitsParam>
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

template <int kFractionBitsParam>
constexpr FixedPoint<kFractionBitsParam> DegToRad(
    FixedPoint<kFractionBitsParam> value_deg);

template <int kFractionBitsParam>
class FixedPoint {
  friend FixedPoint sqrt<kFractionBitsParam>(FixedPoint value);     // NOLINT
  friend FixedPoint sin<kFractionBitsParam>(FixedPoint value_rad);  // NOLINT
  friend FixedPoint cos<kFractionBitsParam>(FixedPoint value_rad);  // NOLINT

  template <int>
  friend constexpr int operator*(int first,
                                 FixedPoint<kFractionBitsParam> second);
  template <int>
  friend constexpr int operator/(int first,
                                 FixedPoint<kFractionBitsParam> second);
  template <int>
  friend constexpr int& operator/=(int& first,
                                   FixedPoint<kFractionBitsParam> second);
  template <int>
  friend constexpr int& operator*=(int& first,
                                   FixedPoint<kFractionBitsParam> second);
  template <int>
  friend class FixedPoint;

 public:
  static constexpr int kFractionBits = kFractionBitsParam;
  static constexpr int kUpperNotIncludedBound =
      (INT_MAX >> kFractionBitsParam) + 1;
  static constexpr int kLowerNotIncludedBound = INT_MIN >> kFractionBitsParam;
  static constexpr double kLeastBitValue = 1.0 / (1 << kFractionBitsParam);

  constexpr FixedPoint();
  constexpr FixedPoint(int value);  // NOLINT
  constexpr FixedPoint(int value, int divider);
  consteval FixedPoint(float value);        // NOLINT
  consteval FixedPoint(double value);       // NOLINT
  consteval FixedPoint(long double value);  // NOLINT
  template <int kOtherFractionBits>
  constexpr explicit FixedPoint(FixedPoint<kOtherFractionBits> value);

  explicit operator double() const;
  explicit operator int() const;

  template <concepts::stream::ByteWritableStreamConcept DestType>
  ReturnCode ToBytes(DestType& buffer) const;

  constexpr FixedPoint& operator+=(const FixedPoint& other);
  constexpr FixedPoint& operator-=(const FixedPoint& other);
  constexpr FixedPoint operator-() const;
  constexpr FixedPoint operator+(const FixedPoint& other) const;
  constexpr FixedPoint operator-(const FixedPoint& other) const;
  template <int kOtherFractionBits>
  constexpr FixedPoint operator*(FixedPoint<kOtherFractionBits> other) const;
  template <int kOtherFractionBits>
  constexpr FixedPoint operator/(FixedPoint<kOtherFractionBits> other) const;
  template <int kOtherFractionBits>
  constexpr FixedPoint& operator*=(FixedPoint<kOtherFractionBits> other);
  template <int kOtherFractionBits>
  constexpr FixedPoint& operator/=(FixedPoint<kOtherFractionBits> other);
  constexpr FixedPoint operator*(int other) const;
  constexpr FixedPoint operator/(int other) const;
  constexpr FixedPoint& operator*=(int other);
  constexpr FixedPoint& operator/=(int other);

  constexpr bool operator==(const FixedPoint& other) const;
  constexpr bool operator!=(const FixedPoint& other) const;
  constexpr bool operator<(const FixedPoint& other) const;
  constexpr bool operator<=(const FixedPoint& other) const;
  constexpr bool operator>(const FixedPoint& other) const;
  constexpr bool operator>=(const FixedPoint& other) const;

  [[nodiscard]] constexpr FixedPoint Abs() const;

  [[nodiscard]] int32_t Serialize() const;
  static FixedPoint Deserialize(int32_t value);

 private:
  [[nodiscard]] constexpr int GetAbsIntPart() const;
  [[nodiscard]] constexpr int GetAbsFractionPart() const;

  int value_;
};

template <int kFractionBitsParam>
constexpr int operator*(int first, FixedPoint<kFractionBitsParam> second);
template <int kFractionBitsParam>
constexpr int operator/(int first, FixedPoint<kFractionBitsParam> second);
template <int kFractionBitsParam>
constexpr int& operator/=(int& first, FixedPoint<kFractionBitsParam> second);
template <int kFractionBitsParam>
constexpr int& operator*=(int& first, FixedPoint<kFractionBitsParam> second);

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

consteval FixedPointBase operator""_fp(long double value);  // NOLINT

template <int kFractionBitsParam>
constexpr FixedPoint<kFractionBitsParam>::FixedPoint() : value_(0) {}

template <int kFractionBitsParam>
constexpr FixedPoint<kFractionBitsParam>::FixedPoint(int value)
    : value_(value << kFractionBitsParam) {}

template <int kFractionBitsParam>
constexpr FixedPoint<kFractionBitsParam>::FixedPoint(int value, int divider)
    : value_((static_cast<int64_t>(value) << kFractionBitsParam) / divider) {}

template <int kFractionBitsParam>
consteval FixedPoint<kFractionBitsParam>::FixedPoint(float value)
    : value_(static_cast<int>(value * (1 << kFractionBitsParam))) {}

template <int kFractionBitsParam>
consteval FixedPoint<kFractionBitsParam>::FixedPoint(double value)
    : value_(static_cast<int>(value * (1 << kFractionBitsParam))) {}

template <int kFractionBitsParam>
consteval FixedPoint<kFractionBitsParam>::FixedPoint(
    long double value)  // NOLINT
    : value_(static_cast<int>(value * (1 << kFractionBitsParam))) {}

template <int kFractionBitsParam>
template <int kOtherFractionBits>
constexpr FixedPoint<kFractionBitsParam>::FixedPoint(
    FixedPoint<kOtherFractionBits> value)
    : value_(0) {
  if constexpr (kFractionBitsParam > kOtherFractionBits) {
    value_ = value.value_ << (kFractionBitsParam - kOtherFractionBits);
  } else {
    value_ = value.value_ >> (kOtherFractionBits - kFractionBitsParam);
  }
}

template <int kFractionBitsParam>
constexpr FixedPoint<kFractionBitsParam> FixedPoint<kFractionBitsParam>::Abs()
    const {
  FixedPoint<kFractionBitsParam> result;
  result.value_ = value_ < 0 ? -value_ : value_;
  return result;
}

template <int kFractionBitsParam>
[[nodiscard]] int32_t FixedPoint<kFractionBitsParam>::Serialize() const {
  return static_cast<int32_t>(value_);
}

template <int kFractionBitsParam>
FixedPoint<kFractionBitsParam> FixedPoint<kFractionBitsParam>::Deserialize(
    int32_t value) {
  FixedPoint<kFractionBitsParam> result;
  result.value_ = static_cast<int>(value);
  return result;
}

template <int kFractionBitsParam>
constexpr int FixedPoint<kFractionBitsParam>::GetAbsIntPart() const {
  return (value_ < 0 ? -value_ : value_) >> kFractionBitsParam;
}

template <int kFractionBitsParam>
constexpr int FixedPoint<kFractionBitsParam>::GetAbsFractionPart() const {
  if (value_ < 0) {
    return (~(value_ - 1)) & ((1 << kFractionBitsParam) - 1);
  }
  return value_ & ((1 << kFractionBitsParam) - 1);
}

template <int kFractionBitsParam>
FixedPoint<kFractionBitsParam>::operator double() const {
  return static_cast<double>(value_) / (1 << kFractionBitsParam);
}

template <int kFractionBitsParam>
FixedPoint<kFractionBitsParam>::operator int() const {
  if (value_ < 0) {
    return -((-value_) >> kFractionBitsParam);
  }
  return value_ >> kFractionBitsParam;
}

template <int kFractionBitsParam>
constexpr FixedPoint<kFractionBitsParam>&
FixedPoint<kFractionBitsParam>::operator+=(
    const FixedPoint<kFractionBitsParam>& other) {
  value_ += other.value_;
  return *this;
}

template <int kFractionBitsParam>
constexpr FixedPoint<kFractionBitsParam>&
FixedPoint<kFractionBitsParam>::operator-=(
    const FixedPoint<kFractionBitsParam>& other) {
  *this += -other;
  return *this;
}

template <int kFractionBitsParam>
constexpr FixedPoint<kFractionBitsParam>
FixedPoint<kFractionBitsParam>::operator-() const {
  FixedPoint<kFractionBitsParam> result = *this;
  result.value_ = -result.value_;
  return result;
}

template <int kFractionBitsParam>
constexpr FixedPoint<kFractionBitsParam>
FixedPoint<kFractionBitsParam>::operator+(
    const FixedPoint<kFractionBitsParam>& other) const {
  FixedPoint<kFractionBitsParam> result = *this;
  result += other;
  return result;
}

template <int kFractionBitsParam>
constexpr FixedPoint<kFractionBitsParam>
FixedPoint<kFractionBitsParam>::operator-(
    const FixedPoint<kFractionBitsParam>& other) const {
  FixedPoint<kFractionBitsParam> result = *this;
  result += -other;
  return result;
}

template <int kFractionBitsParam>
template <int kOtherFractionBits>
constexpr FixedPoint<kFractionBitsParam>
FixedPoint<kFractionBitsParam>::operator*(
    FixedPoint<kOtherFractionBits> other) const {
  FixedPoint<kFractionBitsParam> result = *this;
  const int64_t result_value =
      (static_cast<int64_t>(result.value_) * other.value_) >>
      kOtherFractionBits;
  result.value_ = static_cast<int>(result_value);
  return result;
}

template <int kFractionBitsParam>
template <int kOtherFractionBits>
constexpr FixedPoint<kFractionBitsParam>
FixedPoint<kFractionBitsParam>::operator/(
    FixedPoint<kOtherFractionBits> other) const {
  FixedPoint<kFractionBitsParam> result = *this;
  const int64_t result_value =
      (static_cast<int64_t>(result.value_) << kOtherFractionBits) /
      other.value_;
  result.value_ = static_cast<int>(result_value);
  return result;
}

template <int kFractionBitsParam>
template <int kOtherFractionBits>
constexpr FixedPoint<kFractionBitsParam>&
FixedPoint<kFractionBitsParam>::operator*=(
    FixedPoint<kOtherFractionBits> other) {
  *this = *this * other;
  return *this;
}

template <int kFractionBitsParam>
template <int kOtherFractionBits>
constexpr FixedPoint<kFractionBitsParam>&
FixedPoint<kFractionBitsParam>::operator/=(
    FixedPoint<kOtherFractionBits> other) {
  *this = *this / other;
  return *this;
}

template <int kFractionBitsParam>
constexpr FixedPoint<kFractionBitsParam>
FixedPoint<kFractionBitsParam>::operator*(int other) const {
  FixedPoint<kFractionBitsParam> result = *this;
  result.value_ = result.value_ * other;
  return result;
}

template <int kFractionBitsParam>
constexpr FixedPoint<kFractionBitsParam>
FixedPoint<kFractionBitsParam>::operator/(int other) const {
  FixedPoint<kFractionBitsParam> result = *this;
  result.value_ = result.value_ / other;
  return result;
}

template <int kFractionBitsParam>
constexpr int operator*(int first, FixedPoint<kFractionBitsParam> second) {
  return static_cast<int>((static_cast<int64_t>(first) * second.value_) >>
                          kFractionBitsParam);
}

template <int kFractionBitsParam>
constexpr int operator/(int first, FixedPoint<kFractionBitsParam> second) {
  return static_cast<int>((static_cast<int64_t>(first) << kFractionBitsParam) /
                          second.value_);
}

template <int kFractionBitsParam>
constexpr int& operator*=(int& first, FixedPoint<kFractionBitsParam> second) {
  first = first * second;
  return first;
}

template <int kFractionBitsParam>
constexpr int& operator/=(int& first, FixedPoint<kFractionBitsParam> second) {
  first = first / second;
  return first;
}

template <int kFractionBitsParam>
constexpr bool FixedPoint<kFractionBitsParam>::operator==(
    const FixedPoint& other) const {
  return value_ == other.value_;
}

template <int kFractionBitsParam>
constexpr bool FixedPoint<kFractionBitsParam>::operator!=(
    const FixedPoint& other) const {
  return value_ != other.value_;
}

template <int kFractionBitsParam>
constexpr bool FixedPoint<kFractionBitsParam>::operator<(
    const FixedPoint& other) const {
  return value_ < other.value_;
}

template <int kFractionBitsParam>
constexpr bool FixedPoint<kFractionBitsParam>::operator<=(
    const FixedPoint& other) const {
  return value_ <= other.value_;
}

template <int kFractionBitsParam>
constexpr bool FixedPoint<kFractionBitsParam>::operator>(
    const FixedPoint& other) const {
  return value_ > other.value_;
}

template <int kFractionBitsParam>
constexpr bool FixedPoint<kFractionBitsParam>::operator>=(
    const FixedPoint& other) const {
  return value_ >= other.value_;
}

consteval FixedPointBase operator""_fp(long double value) {  // NOLINT
  return {value};
}

template <int kFractionBitsParam>
constexpr FixedPoint<kFractionBitsParam> kPi = std::numbers::pi_v<double>;
constexpr FixedPointBase kDegsInPi = 180;

template <int kFractionBitsParam>
constexpr FixedPoint<kFractionBitsParam> DegToRad(
    FixedPoint<kFractionBitsParam> value_deg) {
  return value_deg * kPi<kFractionBitsParam> / kDegsInPi;
}

template <int kFractionBitsParam>
template <concepts::stream::ByteWritableStreamConcept DestType>
ReturnCode FixedPoint<kFractionBitsParam>::ToBytes(DestType& buffer) const {
  if (value_ < 0) {
    constexpr char kMinusSymbol = '-';
    auto minus_res = write(buffer, &kMinusSymbol, 1);
    if (minus_res == -1) {
      return ReturnCode::ERROR;
    }
    if (minus_res != 1) {
      return ReturnCode::OVERFLOW;
    }
  }

  int integer_part = GetAbsIntPart();

  int fractional = (GetAbsFractionPart() * 1000) >> kFractionBitsParam;
  int next_number = ((GetAbsFractionPart() * 10000) >> kFractionBitsParam) -
                    (fractional * 10);
  if (next_number >= 5) {
    fractional++;
  }

  if (fractional >= 1000) {
    integer_part++;
    fractional = 0;
  }

  std::array<char, 12> integer_bytes = {};
  auto int_convertion_result =
      std::to_chars(integer_bytes.data(),
                    integer_bytes.data() + integer_bytes.size(), integer_part);
  auto int_writing_result =
      write(buffer, integer_bytes.data(),
            int_convertion_result.ptr - integer_bytes.data());
  if (int_writing_result == -1) {
    return ReturnCode::ERROR;
  }
  if (int_writing_result != int_convertion_result.ptr - integer_bytes.data()) {
    return ReturnCode::OVERFLOW;
  }
  constexpr char kPoint = '.';
  auto point_res = write(buffer, &kPoint, 1);
  if (point_res == -1) {
    return ReturnCode::ERROR;
  }
  if (point_res != 1) {
    return ReturnCode::OVERFLOW;
  }
  if (fractional == 0) {
    constexpr std::string_view kNullFractional = "000";
    int null_res =
        write(buffer, kNullFractional.data(), kNullFractional.size() - 1);
    if (null_res == -1) {
      return ReturnCode::ERROR;
    }
    if (null_res != 3) {
      return ReturnCode::OVERFLOW;
    }
    return ReturnCode::OK;
  }
  int nulls_counter = 1000 / 10;
  while (nulls_counter > fractional) {
    nulls_counter /= 10;
    constexpr char kNullChar = '0';
    auto null_res = write(buffer, &kNullChar, 1);
    if (null_res == -1) {
      return ReturnCode::ERROR;
    }
    if (null_res != 1) {
      return ReturnCode::OVERFLOW;
    }
  }
  auto fract_convertion_result =
      std::to_chars(integer_bytes.data(),
                    integer_bytes.data() + integer_bytes.size(), fractional);
  auto fract_writing_result =
      write(buffer, integer_bytes.data(),
            fract_convertion_result.ptr - integer_bytes.data());
  if (fract_writing_result == -1) {
    return ReturnCode::ERROR;
  }
  if (fract_writing_result !=
      fract_convertion_result.ptr - integer_bytes.data()) {
    return ReturnCode::OVERFLOW;
  }
  return ReturnCode::OK;
}

}  // namespace hydrolib::math

template <int kFractionBitsParam>
hydrolib::math::FixedPoint<kFractionBitsParam> sin(
    hydrolib::math::FixedPoint<kFractionBitsParam> value_rad) {
  auto full_circles = static_cast<int>(
      value_rad / (hydrolib::math::kPi<kFractionBitsParam> * 2));

  constexpr std::array<hydrolib::math::FixedPoint<kFractionBitsParam>, 7>
      kMultiplesOfPi = {
          std::numbers::pi_v<double> * 2,  std::numbers::pi_v<double> * 4,
          std::numbers::pi_v<double> * 8,  std::numbers::pi_v<double> * 16,
          std::numbers::pi_v<double> * 32, std::numbers::pi_v<double> * 64,
          std::numbers::pi_v<double> * 128};
  hydrolib::math::FixedPoint<kFractionBitsParam> normalized_value_rad =
      value_rad;
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

  if (normalized_value_rad > hydrolib::math::kPi<kFractionBitsParam>) {
    normalized_value_rad -= hydrolib::math::kPi<kFractionBitsParam> * 2;
  } else if (normalized_value_rad < -hydrolib::math::kPi<kFractionBitsParam>) {
    normalized_value_rad += hydrolib::math::kPi<kFractionBitsParam> * 2;
  }

  if (normalized_value_rad > hydrolib::math::kPi<kFractionBitsParam> / 2) {
    normalized_value_rad =
        hydrolib::math::kPi<kFractionBitsParam> - normalized_value_rad;
  } else if (normalized_value_rad <
             -hydrolib::math::kPi<kFractionBitsParam> / 2) {
    normalized_value_rad =
        -hydrolib::math::kPi<kFractionBitsParam> - normalized_value_rad;
  }

  int current_diff =
      normalized_value_rad.value_ *
      (1 << (hydrolib::math::CORDICConstants::kTrigonometryFractionBits -
             kFractionBitsParam));
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
  return hydrolib::math::FixedPoint<kFractionBitsParam>(
      y_coordinate,
      1 << hydrolib::math::CORDICConstants::kTrigonometryFractionBits);
}

template <int kFractionBitsParam>
hydrolib::math::FixedPoint<kFractionBitsParam> cos(
    hydrolib::math::FixedPoint<kFractionBitsParam> value_rad) {
  if (value_rad > 0) {
    value_rad -= hydrolib::math::kPi<kFractionBitsParam> * 2;
  } else if (value_rad < 0) {
    value_rad += hydrolib::math::kPi<kFractionBitsParam> * 2;
  }
  return sin(value_rad + (hydrolib::math::kPi<kFractionBitsParam> / 2));
}

template <int kFractionBitsParam>
hydrolib::math::FixedPoint<kFractionBitsParam> sqrt(
    hydrolib::math::FixedPoint<kFractionBitsParam> value) {
  if (value.value_ == 0) {
    return hydrolib::math::FixedPoint<kFractionBitsParam>(0);
  }

  int64_t target_sq = static_cast<int64_t>(value.value_) << kFractionBitsParam;

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
