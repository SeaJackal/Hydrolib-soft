#pragma once

#include "hydrolib_fixed_point.hpp"
#include "hydrolib_quaternions.hpp"
#include "hydrolib_vector3d.hpp"

namespace hydrolib::math {

template <ArithmeticConcept Number>
class Rotation {
 public:
  static Rotation GetMean(const Rotation& first, const Rotation& second);

  constexpr Rotation() = default;
  Rotation(Vector3D<Number> axis, Number angle_rad);
  Rotation(Vector3D<Number> from_dir, Vector3D<Number> to_dir);

  Rotation ExtractZComponent();
  [[nodiscard]] Vector3D<Number> Rotate(const Vector3D<Number>& vector) const;
  [[nodiscard]] Vector3D<Number> GetAxis() const;
  Rotation operator+(const Rotation& other) const;
  Rotation& operator+=(const Rotation& other);
  Rotation operator-(const Rotation& other) const;

  void Normalize();

 private:
  constexpr static Number kTolerance = 0.001;

  explicit Rotation(const Quaternion<Number>& quaternion);

  Quaternion<Number> quaternion_{.x = 0, .y = 0, .z = 0, .w = 1};
};

template <ArithmeticConcept Number>
inline Rotation<Number>::Rotation(Vector3D<Number> from_dir,
                                  Vector3D<Number> to_dir) {
  Number w_component =
      Vector3D<Number>::Dot(from_dir, to_dir) +
      sqrt((from_dir.x * from_dir.x + from_dir.y * from_dir.y +
            from_dir.z * from_dir.z) *
           (to_dir.x * to_dir.x + to_dir.y * to_dir.y + to_dir.z * to_dir.z));
  if (w_component > kTolerance || w_component < -kTolerance) {
    auto cross = Vector3D<Number>::Cross(from_dir, to_dir);
    quaternion_ = Quaternion<Number>(cross.x, cross.y, cross.z, w_component);
    Quaternion<Number>::Normalize(quaternion_);
  } else {
    if (from_dir.y == 0 && from_dir.x == 0) {
      quaternion_ = {.x = 1, .y = 0, .z = 0, .w = 0};
    } else {
      quaternion_ = {.x = from_dir.y, .y = -from_dir.x, .z = 0, .w = 0};
      Quaternion<Number>::Normalize(quaternion_);
    }
  }
}

template <>
inline Rotation<double>::Rotation(Vector3D<double> from_dir,
                                  Vector3D<double> to_dir) {
  double w_component =
      Vector3D<double>::Dot(from_dir, to_dir) +
      sqrt((from_dir.x * from_dir.x + from_dir.y * from_dir.y +
            from_dir.z * from_dir.z) *
           (to_dir.x * to_dir.x + to_dir.y * to_dir.y + to_dir.z * to_dir.z));
  if (w_component > std::numeric_limits<double>::epsilon() ||
      w_component < -std::numeric_limits<double>::epsilon()) {
    auto cross = Vector3D<double>::Cross(from_dir, to_dir);
    quaternion_ = Quaternion<double>(cross.x, cross.y, cross.z, w_component);
    Quaternion<double>::Normalize(quaternion_);
  } else {
    if (from_dir.y == 0 && from_dir.x == 0) {
      quaternion_ = {.x = 1, .y = 0, .z = 0, .w = 0};
    } else {
      quaternion_ = {.x = from_dir.y, .y = -from_dir.x, .z = 0, .w = 0};
      Quaternion<double>::Normalize(quaternion_);
    }
  }
}

template <ArithmeticConcept Number>
inline Rotation<Number>::Rotation(Vector3D<Number> axis, Number angle_rad) {
  Vector3D<Number>::Normalize(axis);
  quaternion_ = {.x = axis.x * sin(angle_rad / 2),  // NOLINT
                 .y = axis.y * sin(angle_rad / 2),
                 .z = axis.z * sin(angle_rad / 2),
                 .w = cos(angle_rad / 2)};
}

template <ArithmeticConcept Number>
inline Rotation<Number>::Rotation(const Quaternion<Number>& quaternion)
    : quaternion_(quaternion) {}

template <ArithmeticConcept Number>
inline Vector3D<Number> Rotation<Number>::Rotate(
    const Vector3D<Number>& vector) const {
  auto result =
      quaternion_ *
      Quaternion<Number>{.x = vector.x, .y = vector.y, .z = vector.z, .w = 0} *
      (!quaternion_);
  return {.x = result.x, .y = result.y, .z = result.z};
}

template <ArithmeticConcept Number>
inline Rotation<Number> Rotation<Number>::ExtractZComponent() {
  if (quaternion_.z == 0) {
    return Rotation<Number>({.x = 0, .y = 0, .z = 0, .w = 1});
  }
  Number new_w =
      sqrt(quaternion_.w * quaternion_.w + quaternion_.z * quaternion_.z);
  Number sin_yaw = quaternion_.z / new_w;
  Number cos_yaw = quaternion_.w / new_w;
  Number new_x = quaternion_.x * cos_yaw + quaternion_.y * sin_yaw;
  Number new_y = quaternion_.y * cos_yaw - quaternion_.x * sin_yaw;
  quaternion_.x = new_x;
  quaternion_.y = new_y;
  quaternion_.z = 0;
  quaternion_.w = new_w;
  return Rotation<Number>({.x = 0, .y = 0, .z = sin_yaw, .w = cos_yaw});
}

template <ArithmeticConcept Number>
inline Rotation<Number> Rotation<Number>::GetMean(
    const Rotation<Number>& first, const Rotation<Number>& second) {
  Rotation result;
  if (Quaternion<Number>::Dot(first.quaternion_, second.quaternion_) < 0) {
    result.quaternion_ = (first.quaternion_ - second.quaternion_) / 2;
  } else {
    result.quaternion_ = (first.quaternion_ + second.quaternion_) / 2;
  }
  Quaternion<Number>::Normalize(result.quaternion_);
  return result;
}

template <ArithmeticConcept Number>
inline Vector3D<Number> Rotation<Number>::GetAxis() const {
  Vector3D<Number> result{
      .x = quaternion_.x, .y = quaternion_.y, .z = quaternion_.z};
  Vector3D<Number>::Normalize(result);
  return result;
}

template <ArithmeticConcept Number>
inline Rotation<Number>& Rotation<Number>::operator+=(
    const Rotation<Number>& other) {
  quaternion_ = other.quaternion_ * quaternion_;
  return *this;
}

template <ArithmeticConcept Number>
inline Rotation<Number> Rotation<Number>::operator+(
    const Rotation<Number>& other) const {
  auto result = *this;
  result += other;
  return result;
}

template <ArithmeticConcept Number>
inline Rotation<Number> Rotation<Number>::operator-(
    const Rotation<Number>& other) const {
  return Rotation<Number>(quaternion_ * (!other.quaternion_));
}

template <ArithmeticConcept Number>
inline void Rotation<Number>::Normalize() {
  Quaternion<Number>::Normalize(quaternion_);
}
}  // namespace hydrolib::math
