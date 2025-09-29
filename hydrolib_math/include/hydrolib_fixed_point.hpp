#pragma once

#include <cmath>

#include <concepts>

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
class FixedPoint;

template <unsigned FRACTION_BITS>
FixedPoint<FRACTION_BITS>
sqrt(FixedPoint<FRACTION_BITS> value); // TODO: fix accuracy

template <unsigned FRACTION_BITS>
class FixedPoint
{
    friend FixedPoint sqrt<FRACTION_BITS>(FixedPoint value);

public:
    explicit constexpr FixedPoint(int value);
    consteval FixedPoint(float value);
    consteval FixedPoint(double value);
    consteval FixedPoint(long double value);

public:
    operator double() const;

    FixedPoint &operator+=(const FixedPoint &other);
    FixedPoint &operator-=(const FixedPoint &other);
    FixedPoint operator-() const;
    FixedPoint operator+(const FixedPoint &other) const;
    FixedPoint operator-(const FixedPoint &other) const;
    FixedPoint operator*(const FixedPoint &other) const;
    FixedPoint operator/(const FixedPoint &other) const;
    FixedPoint &operator*=(const FixedPoint &other);
    FixedPoint &operator/=(const FixedPoint &other);

private:
    int value_;
};

using FixedPoint10 = FixedPoint<10>;

consteval FixedPoint10 operator""_fp(long double value);

///////////////////////////////////////////////////////////////////////////////

template <unsigned FRACTION_BITS>
constexpr FixedPoint<FRACTION_BITS>::FixedPoint(int value)
    : value_(value << FRACTION_BITS)
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
FixedPoint<FRACTION_BITS> sqrt(FixedPoint<FRACTION_BITS> value)
{
    if (value.value_ == 0)
    {
        return FixedPoint<FRACTION_BITS>(0);
    }

    if constexpr (FRACTION_BITS % 2)
    {
        value.value_ = value.value_ << 1;
    }

    int low = 0;
    int high = value.value_ / 2;
    int mid = low + (high - low) / 2;

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
            if constexpr (FRACTION_BITS % 2)
            {
                value.value_ = mid << ((FRACTION_BITS + 1) / 2);
            }
            else
            {
                value.value_ = mid << (FRACTION_BITS / 2);
            }
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
            if constexpr (FRACTION_BITS % 2)
            {
                value.value_ = value.value_ << ((FRACTION_BITS + 1) / 2);
            }
            else
            {
                value.value_ = value.value_ << (FRACTION_BITS / 2);
            }
            return value;
        }
        mid = low + (high - low) / 2;
    }

    return value;
}

consteval FixedPoint10 operator""_fp(long double value)
{
    return FixedPoint<10>(value);
}

} // namespace hydrolib::math