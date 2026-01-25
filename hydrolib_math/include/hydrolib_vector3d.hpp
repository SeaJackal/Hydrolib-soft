#pragma once

#include "hydrolib_fixed_point.hpp"

namespace hydrolib::math {
template <ArithmeticConcept Number>
struct Vector3D {
  static constexpr Number Dot(const Vector3D& first, const Vector3D& second);
  static constexpr Vector3D Cross(const Vector3D& first,
                                  const Vector3D& second);
  static constexpr Number Length(const Vector3D& vector);
  static void Normalize(Vector3D& vector);

  Number x;
  Number y;
  Number z;
};

template <ArithmeticConcept Number>
Vector3D<Number>& operator+=(Vector3D<Number>& first,
                             const Vector3D<Number>& second);
template <ArithmeticConcept Number>
Vector3D<Number> operator+(const Vector3D<Number>& first,
                           const Vector3D<Number>& second);
template <ArithmeticConcept Number>
Vector3D<Number> operator-(const Vector3D<Number>& vector);
template <ArithmeticConcept Number>
Vector3D<Number>& operator-=(Vector3D<Number>& first,
                             const Vector3D<Number>& second);
template <ArithmeticConcept Number>
Vector3D<Number> operator-(const Vector3D<Number>& first,
                           const Vector3D<Number>& second);
template <ArithmeticConcept Number>
Vector3D<Number> operator*(const Vector3D<Number>& vector, Number scalar);
template <ArithmeticConcept Number>
Vector3D<Number> operator*(Number scalar, const Vector3D<Number>& vector);
template <ArithmeticConcept Number>
Vector3D<Number>& operator*=(Vector3D<Number>& vector, Number scalar);
template <ArithmeticConcept Number>
Vector3D<Number> operator/(const Vector3D<Number>& vector, Number scalar);
template <ArithmeticConcept Number>
Vector3D<Number>& operator/=(Vector3D<Number>& vector, Number scalar);

template <ArithmeticConcept Number>
constexpr Number Vector3D<Number>::Dot(const Vector3D& first,
                                       const Vector3D& second) {
  return (first.x * second.x) + (first.y * second.y) + (first.z * second.z);
}

template <ArithmeticConcept Number>
constexpr Vector3D<Number> Vector3D<Number>::Cross(const Vector3D& first,
                                                   const Vector3D& second) {
  return Vector3D<Number>(first.y * second.z - first.z * second.y,
                          first.z * second.x - first.x * second.z,
                          first.x * second.y - first.y * second.x);
}

template <ArithmeticConcept Number>
constexpr Number Vector3D<Number>::Length(const Vector3D& vector) {
  return sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}

template <ArithmeticConcept Number>
inline void Vector3D<Number>::Normalize(Vector3D& vector) {
  Number len = Length(vector);
  if (len == 0) {
    vector.x = 0;
    vector.y = 0;
    vector.z = 0;
  } else {
    vector.x /= len;
    vector.y /= len;
    vector.z /= len;
  }
}

template <ArithmeticConcept Number>
inline Vector3D<Number>& operator+=(Vector3D<Number>& first,
                                    const Vector3D<Number>& second) {
  first.x += second.x;
  first.y += second.y;
  first.z += second.z;
  return first;
}

template <ArithmeticConcept Number>
inline Vector3D<Number> operator+(const Vector3D<Number>& first,
                                  const Vector3D<Number>& second) {
  auto result = first;
  result += second;
  return result;
}

template <ArithmeticConcept Number>
inline Vector3D<Number> operator-(const Vector3D<Number>& vector) {
  return Vector3D<Number>(-vector.x, -vector.y, -vector.z);
}

template <ArithmeticConcept Number>
inline Vector3D<Number>& operator-=(Vector3D<Number>& first,
                                    const Vector3D<Number>& second) {
  first += -second;
  return first;
}

template <ArithmeticConcept Number>
inline Vector3D<Number> operator-(const Vector3D<Number>& first,
                                  const Vector3D<Number>& second) {
  auto result = first;
  result -= second;
  return result;
}

template <ArithmeticConcept Number>
inline Vector3D<Number> operator*(const Vector3D<Number>& vector,
                                  Number scalar) {
  return Vector3D<Number>(vector.x * scalar, vector.y * scalar,
                          vector.z * scalar);
}

template <ArithmeticConcept Number>
inline Vector3D<Number> operator*(Number scalar,
                                  const Vector3D<Number>& vector) {
  return vector * scalar;
}

template <ArithmeticConcept Number>
inline Vector3D<Number>& operator*=(Vector3D<Number>& vector, Number scalar) {
  vector.x *= scalar;
  vector.y *= scalar;
  vector.z *= scalar;
  return vector;
}

template <ArithmeticConcept Number>
inline Vector3D<Number> operator/(const Vector3D<Number>& vector,
                                  Number scalar) {
  return Vector3D<Number>(vector.x / scalar, vector.y / scalar,
                          vector.z / scalar);
}

template <ArithmeticConcept Number>
inline Vector3D<Number>& operator/=(Vector3D<Number>& vector, Number scalar) {
  vector.x /= scalar;
  vector.y /= scalar;
  vector.z /= scalar;
  return vector;
}
}  // namespace hydrolib::math