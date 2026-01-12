#pragma once

#include "hydrolib_fixed_point.hpp"

namespace hydrolib::math {

template <ArithmeticConcept Number>
struct Quaternion {
  static Number Dot(const Quaternion<Number>& first,
                    const Quaternion<Number>& second);
  static Number GetNorm(const Quaternion<Number>& first);
  static void Normalize(Quaternion<Number>& first);

  Number x;
  Number y;
  Number z;
  Number w;
};

template <ArithmeticConcept Number>
Quaternion<Number>& operator+=(Quaternion<Number>& first,
                               const Quaternion<Number>& second);
template <ArithmeticConcept Number>
Quaternion<Number> operator-(const Quaternion<Number>& first);
template <ArithmeticConcept Number>
Quaternion<Number> operator+(const Quaternion<Number>& first,
                             const Quaternion<Number>& second);
template <ArithmeticConcept Number>
Quaternion<Number> operator-(const Quaternion<Number>& first,
                             const Quaternion<Number>& second);
template <ArithmeticConcept Number>
Quaternion<Number> operator*(const Quaternion<Number>& first,
                             const Quaternion<Number>& second);
template <ArithmeticConcept Number>
Quaternion<Number> operator!(const Quaternion<Number>& first);
template <ArithmeticConcept Number>
Quaternion<Number> operator*(Number multiplier,
                             const Quaternion<Number>& first);
template <ArithmeticConcept Number>
Quaternion<Number> operator*(const Quaternion<Number>& first,
                             Number multiplier);
template <ArithmeticConcept Number>
Quaternion<Number> operator/(const Quaternion<Number>& first, Number divider);

template <ArithmeticConcept Number>
inline Quaternion<Number>& operator+=(Quaternion<Number>& first,
                                      const Quaternion<Number>& second) {
  first.x += second.x;
  first.y += second.y;
  first.z += second.z;
  first.w += second.w;
  return first;
}

template <ArithmeticConcept Number>
inline Quaternion<Number> operator-(const Quaternion<Number>& first) {
  return Quaternion<Number>(
      {.x = -first.x, .y = -first.y, .z = -first.z, .w = -first.w});
}

template <ArithmeticConcept Number>
inline Quaternion<Number> operator+(const Quaternion<Number>& first,
                                    const Quaternion<Number>& second) {
  Quaternion<Number> result = first;
  result += second;
  return result;
}

template <ArithmeticConcept Number>
inline Quaternion<Number> operator-(const Quaternion<Number>& first,
                                    const Quaternion<Number>& second) {
  Quaternion<Number> result = first;
  result += -second;
  return result;
}

template <ArithmeticConcept Number>
inline Quaternion<Number> operator*(const Quaternion<Number>& first,
                                    const Quaternion<Number>& second) {
  return Quaternion<Number>({.x = (first.x * second.w) + (first.w * second.x) +
                                  (first.y * second.z) - (first.z * second.y),
                             .y = (first.z * second.x) + (first.y * second.w) +
                                  (first.w * second.y) - (first.x * second.z),
                             .z = (first.w * second.z) + (first.x * second.y) -
                                  (first.y * second.x) + (first.z * second.w),
                             .w = (first.w * second.w) - (first.y * second.y) -
                                  (first.z * second.z) - (first.x * second.x)});
}

template <ArithmeticConcept Number>
inline Quaternion<Number> operator!(const Quaternion<Number>& first) {
  return Quaternion<Number>(
      {.x = -first.x, .y = -first.y, .z = -first.z, .w = first.w});
}

template <ArithmeticConcept Number>
inline Quaternion<Number> operator*(Number multiplier,
                                    const Quaternion<Number>& first) {
  return Quaternion<Number>({.x = first.x * multiplier,
                             .y = first.y * multiplier,
                             .z = first.z * multiplier,
                             .w = first.w * multiplier});
}

template <ArithmeticConcept Number>
inline Quaternion<Number> operator*(const Quaternion<Number>& first,
                                    Number multiplier) {
  return multiplier * first;
}

template <ArithmeticConcept Number>
inline Quaternion<Number> operator/(const Quaternion<Number>& first,
                                    Number divider) {
  return Quaternion<Number>({.x = first.x / divider,
                             .y = first.y / divider,
                             .z = first.z / divider,
                             .w = first.w / divider});
}

template <ArithmeticConcept Number>
inline Number Quaternion<Number>::Dot(const Quaternion<Number>& first,
                                      const Quaternion<Number>& second) {
  return (first.x * second.x) + (first.y * second.y) + (first.z * second.z) +
         (first.w * second.w);
}

template <ArithmeticConcept Number>
inline Number Quaternion<Number>::GetNorm(const Quaternion<Number>& first) {
  return sqrt((first.x * first.x) + (first.y * first.y) + (first.z * first.z) +
              (first.w * first.w));
}

template <ArithmeticConcept Number>
inline void Quaternion<Number>::Normalize(Quaternion<Number>& first) {
  Number norm = GetNorm(first);
  if (norm != 0) {
    first.x /= norm;
    first.y /= norm;
    first.z /= norm;
    first.w /= norm;
  }
}

}  // namespace hydrolib::math
