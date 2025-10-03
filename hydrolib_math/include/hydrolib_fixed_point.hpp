#pragma once

#include <concepts>
#include <cmath>


namespace hydrolib::math
{
template <unsigned FRACTION_BITS>
class FixedPoint;
}

template <unsigned FRACTION_BITS>
hydrolib::math::FixedPoint<FRACTION_BITS>
sin(hydrolib::math::FixedPoint<FRACTION_BITS> value_rad);

template <unsigned FRACTION_BITS>
hydrolib::math::FixedPoint<FRACTION_BITS>
cos(hydrolib::math::FixedPoint<FRACTION_BITS> value_rad);

template <unsigned FRACTION_BITS>
hydrolib::math::FixedPoint<FRACTION_BITS>
sqrt(hydrolib::math::FixedPoint<FRACTION_BITS> value); // TODO: fix accuracy

namespace hydrolib::math
{
template <typename T>
concept ArithmeticConcept = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
    { a - b } -> std::convertible_to<T>;
    { a *b } -> std::convertible_to<T>;
    { sqrt(a) } -> std::convertible_to<T>;
    { a / b } -> std::convertible_to<T>;
    { -a } -> std::convertible_to<T>;
    { a += b } -> std::same_as<T &>;
    { a -= b } -> std::same_as<T &>;
    { a *= b } -> std::same_as<T &>;
    { a /= b } -> std::same_as<T &>;
};

template <unsigned FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS>
DegToRad(FixedPoint<FRACTION_BITS> value_deg);

template <unsigned FRACTION_BITS>
class FixedPoint
{
    friend FixedPoint sqrt<FRACTION_BITS>(FixedPoint value);

public:
    constexpr FixedPoint();
    constexpr FixedPoint(int value);
    constexpr FixedPoint(int value, int divider);
    consteval FixedPoint(float value);
    consteval FixedPoint(double value);
    consteval FixedPoint(long double value);

public:
    explicit operator double() const;

    FixedPoint &operator+=(const FixedPoint &other);
    FixedPoint &operator-=(const FixedPoint &other);
    FixedPoint operator-() const;
    FixedPoint operator+(const FixedPoint &other) const;
    FixedPoint operator-(const FixedPoint &other) const;
    FixedPoint operator*(const FixedPoint &other) const;
    FixedPoint operator/(const FixedPoint &other) const;
    FixedPoint &operator*=(const FixedPoint &other);
    FixedPoint &operator/=(const FixedPoint &other);

    bool operator==(const FixedPoint &other) const;
    bool operator!=(const FixedPoint &other) const;
    bool operator<(const FixedPoint &other) const;
    bool operator<=(const FixedPoint &other) const;
    bool operator>(const FixedPoint &other) const;
    bool operator>=(const FixedPoint &other) const;

    // Comparison with int
    bool operator==(int other) const;
    bool operator!=(int other) const;
    bool operator<(int other) const;
    bool operator<=(int other) const;
    bool operator>(int other) const;
    bool operator>=(int other) const;

    constexpr FixedPoint Abs() const;

    static constexpr int GetFractionBits();
    constexpr int GetAbsIntPart() const;
    constexpr int GetAbsFractionPart() const;

private:
    int value_;
};

using FixedPointBase = FixedPoint<10>;

consteval FixedPointBase operator""_fp(long double value);

///////////////////////////////////////////////////////////////////////////////

template <unsigned FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS>::FixedPoint() : value_(0)
{
}

template <unsigned FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS>::FixedPoint(int value)
    : value_(value << FRACTION_BITS)
{
}

template <unsigned FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS>::FixedPoint(int value, int divider)
    : value_(value * (1 << FRACTION_BITS) / divider)
{
}

template <unsigned FRACTION_BITS>
consteval FixedPoint<FRACTION_BITS>::FixedPoint(float value)
    : value_(static_cast<int>(value * (1 << FRACTION_BITS)))
{
}

template <unsigned FRACTION_BITS>
consteval FixedPoint<FRACTION_BITS>::FixedPoint(double value)
    : value_(static_cast<int>(value * (1 << FRACTION_BITS)))
{
}

template <unsigned FRACTION_BITS>
consteval FixedPoint<FRACTION_BITS>::FixedPoint(long double value)
    : value_(static_cast<int>(value * (1 << FRACTION_BITS)))
{
}

template <unsigned FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS> FixedPoint<FRACTION_BITS>::Abs() const
{
    FixedPoint<FRACTION_BITS> result;
    result.value_ = value_ < 0 ? -value_ : value_;
    return result;
}

template <unsigned FRACTION_BITS>
constexpr int FixedPoint<FRACTION_BITS>::GetFractionBits()
{
    return FRACTION_BITS;
}

template <unsigned FRACTION_BITS>
constexpr int FixedPoint<FRACTION_BITS>::GetAbsIntPart() const
{
    return (value_ < 0 ? -value_ : value_) >> FRACTION_BITS;
}

template <unsigned FRACTION_BITS>
constexpr int FixedPoint<FRACTION_BITS>::GetAbsFractionPart() const
{
    if (value_ < 0)
    {
        return (~(value_ - 1)) & ((1 << FRACTION_BITS) - 1);
    }
    else
    {
        return value_ & ((1 << FRACTION_BITS) - 1);
    }
}

template <unsigned FRACTION_BITS>
FixedPoint<FRACTION_BITS>::operator double() const
{
    return static_cast<double>(value_) / (1 << FRACTION_BITS);
}

template <unsigned FRACTION_BITS>
FixedPoint<FRACTION_BITS> &
FixedPoint<FRACTION_BITS>::operator+=(const FixedPoint<FRACTION_BITS> &other)
{
    value_ += other.value_;
    return *this;
}

template <unsigned FRACTION_BITS>
FixedPoint<FRACTION_BITS> &
FixedPoint<FRACTION_BITS>::operator-=(const FixedPoint<FRACTION_BITS> &other)
{
    *this += -other;
    return *this;
}

template <unsigned FRACTION_BITS>
FixedPoint<FRACTION_BITS> FixedPoint<FRACTION_BITS>::operator-() const
{
    FixedPoint<FRACTION_BITS> result = *this;
    result.value_ = -result.value_;
    return result;
}

template <unsigned FRACTION_BITS>
FixedPoint<FRACTION_BITS> FixedPoint<FRACTION_BITS>::operator+(
    const FixedPoint<FRACTION_BITS> &other) const
{
    FixedPoint<FRACTION_BITS> result = *this;
    result += other;
    return result;
}

template <unsigned FRACTION_BITS>
FixedPoint<FRACTION_BITS> FixedPoint<FRACTION_BITS>::operator-(
    const FixedPoint<FRACTION_BITS> &other) const
{
    FixedPoint<FRACTION_BITS> result = *this;
    result += -other;
    return result;
}

template <unsigned FRACTION_BITS>
FixedPoint<FRACTION_BITS> FixedPoint<FRACTION_BITS>::operator*(
    const FixedPoint<FRACTION_BITS> &other) const
{
    FixedPoint<FRACTION_BITS> result = *this;
    result.value_ = (result.value_ * other.value_) >> FRACTION_BITS;
    return result;
}

template <unsigned FRACTION_BITS>
FixedPoint<FRACTION_BITS> FixedPoint<FRACTION_BITS>::operator/(
    const FixedPoint<FRACTION_BITS> &other) const
{
    FixedPoint<FRACTION_BITS> result = *this;
    result.value_ = (result.value_ << FRACTION_BITS) / other.value_;
    return result;
}

template <unsigned FRACTION_BITS>
FixedPoint<FRACTION_BITS> &
FixedPoint<FRACTION_BITS>::operator*=(const FixedPoint<FRACTION_BITS> &other)
{
    *this = *this * other;
    return *this;
}

template <unsigned FRACTION_BITS>
FixedPoint<FRACTION_BITS> &
FixedPoint<FRACTION_BITS>::operator/=(const FixedPoint<FRACTION_BITS> &other)
{
    *this = *this / other;
    return *this;
}

template <unsigned FRACTION_BITS>
bool FixedPoint<FRACTION_BITS>::operator==(const FixedPoint &other) const
{
    return (value_ - other.value_) < (1 << (FRACTION_BITS / 2));
}

template <unsigned FRACTION_BITS>
bool FixedPoint<FRACTION_BITS>::operator!=(const FixedPoint &other) const
{
    return value_ != other.value_;
}

template <unsigned FRACTION_BITS>
bool FixedPoint<FRACTION_BITS>::operator<(const FixedPoint &other) const
{
    return value_ < other.value_;
}

template <unsigned FRACTION_BITS>
bool FixedPoint<FRACTION_BITS>::operator<=(const FixedPoint &other) const
{
    return value_ <= other.value_;
}

template <unsigned FRACTION_BITS>
bool FixedPoint<FRACTION_BITS>::operator>(const FixedPoint &other) const
{
    return value_ > other.value_;
}

template <unsigned FRACTION_BITS>
bool FixedPoint<FRACTION_BITS>::operator>=(const FixedPoint &other) const
{
    return value_ >= other.value_;
}

template <unsigned FRACTION_BITS>
bool FixedPoint<FRACTION_BITS>::operator==(int other) const
{
    return value_ == (other << FRACTION_BITS);
}

template <unsigned FRACTION_BITS>
bool FixedPoint<FRACTION_BITS>::operator!=(int other) const
{
    return value_ != (other << FRACTION_BITS);
}

template <unsigned FRACTION_BITS>
bool FixedPoint<FRACTION_BITS>::operator<(int other) const
{
    return value_ < (other << FRACTION_BITS);
}

template <unsigned FRACTION_BITS>
bool FixedPoint<FRACTION_BITS>::operator<=(int other) const
{
    return value_ <= (other << FRACTION_BITS);
}

template <unsigned FRACTION_BITS>
bool FixedPoint<FRACTION_BITS>::operator>(int other) const
{
    return value_ > (other << FRACTION_BITS);
}

template <unsigned FRACTION_BITS>
bool FixedPoint<FRACTION_BITS>::operator>=(int other) const
{
    return value_ >= (other << FRACTION_BITS);
}

consteval FixedPointBase operator""_fp(long double value)
{
    return FixedPointBase(value);
}

constexpr FixedPointBase pi = 3.14159265358979323846_fp;

template <unsigned FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS>
DegToRad(FixedPoint<FRACTION_BITS> value_deg)
{
    return value_deg * pi / 180;
}

} // namespace hydrolib::math

template <unsigned FRACTION_BITS>
hydrolib::math::FixedPoint<FRACTION_BITS>
sin(hydrolib::math::FixedPoint<FRACTION_BITS> value_rad)
{
    hydrolib::math::FixedPoint<FRACTION_BITS> result = value_rad;
    int n = 3;
    hydrolib::math::FixedPoint<FRACTION_BITS> diff =
        -(value_rad * value_rad * value_rad) / 3 / 2;
    while (diff.Abs() > 0)
    {
        result += diff;
        n += 2;
        diff *= -(value_rad * value_rad) / (n * (n - 1));
    }
    return result;
}

template <unsigned FRACTION_BITS>
hydrolib::math::FixedPoint<FRACTION_BITS>
cos(hydrolib::math::FixedPoint<FRACTION_BITS> value_rad)
{
    hydrolib::math::FixedPoint<FRACTION_BITS> result = 1;
    int n = 2;
    hydrolib::math::FixedPoint<FRACTION_BITS> diff =
        -(value_rad * value_rad) / 2;
    while (diff.Abs() > 0)
    {
        result += diff;
        n += 2;
        diff *= -(value_rad * value_rad) / (n * (n - 1));
    }
    return result;
}

template <unsigned FRACTION_BITS>
hydrolib::math::FixedPoint<FRACTION_BITS>
sqrt(hydrolib::math::FixedPoint<FRACTION_BITS> value)
{
    if (value.value_ == 0)
    {
        return hydrolib::math::FixedPoint<FRACTION_BITS>(0);
    }

    value.value_ = value.value_ << FRACTION_BITS;

    int low = 0;
    int high = 46341;
    int mid = high / 2;

    while (low < high)
    {
        int sq = mid * mid;
        if (sq > value.value_)
        {
            high = mid;
        }
        else if (sq < value.value_)
        {
            low = mid;
        }
        else
        {
            value.value_ = mid;
            return value;
        }
        if (high - low == 1)
        {
            if (high * high - value.value_ < value.value_ - low * low)
            {
                value.value_ = high;
            }
            else
            {
                value.value_ = low;
            }
            return value;
        }
        mid = low + (high - low) / 2;
    }

    return value;
}
