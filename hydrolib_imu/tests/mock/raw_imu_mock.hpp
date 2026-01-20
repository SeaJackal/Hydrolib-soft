#pragma once

#include "hydrolib_fixed_point.hpp"
#include "hydrolib_quaternions.hpp"
#include "hydrolib_rotations.hpp"
#include "hydrolib_vector3d.hpp"

namespace hydrolib::sensors {

template <math::ArithmeticConcept Number>
class RawIMUMock {
 public:
  RawIMUMock() = default;

  void SetTarget(math::Vector3D<Number> axis, Number angle_rad, int n);

  bool Step();

  math::Vector3D<Number> GetAcceleration() const;
  math::Vector3D<Number> GetGyro() const;
  math::Rotation<Number> GetOrientation() const;

  math::Vector3D<Number> GetXAxis() const;
  math::Vector3D<Number> GetYAxis() const;
  math::Vector3D<Number> GetZAxis() const;

 private:
  int counter_{};
  math::Rotation<Number> orientation_;
  math::Rotation<Number> delta_orientation_;
  math::Vector3D<Number> w_{.x = 0, .y = 0, .z = 0};
};

template <math::ArithmeticConcept Number>
inline void RawIMUMock<Number>::SetTarget(math::Vector3D<Number> axis,
                                          Number angle_rad, int n) {
  delta_orientation_ = math::Rotation<Number>(axis, angle_rad / n);
  w_ = axis * (angle_rad / n);
  counter_ = n;
}

template <math::ArithmeticConcept Number>
inline bool RawIMUMock<Number>::Step() {
  if (counter_ == 0) {
    w_ = {.x = 0, .y = 0, .z = 0};
    return false;
  }
  counter_--;
  orientation_ += delta_orientation_;
  return true;
}

template <math::ArithmeticConcept Number>
inline math::Vector3D<Number> RawIMUMock<Number>::GetAcceleration() const {
  math::Vector3D<Number> gravity{.x = 0, .y = 0, .z = -1};
  auto result = orientation_.Rotate(gravity);
  math::Vector3D<Number>::Normalize(result);
  return result;
}

template <math::ArithmeticConcept Number>
inline math::Vector3D<Number> RawIMUMock<Number>::GetGyro() const {
  return w_;
}

template <math::ArithmeticConcept Number>
inline math::Rotation<Number> RawIMUMock<Number>::GetOrientation() const {
  return orientation_;
}

template <math::ArithmeticConcept Number>
inline math::Vector3D<Number> RawIMUMock<Number>::GetXAxis() const {
  return orientation_.Rotate({.x = 1, .y = 0, .z = 0});
}

template <math::ArithmeticConcept Number>
inline math::Vector3D<Number> RawIMUMock<Number>::GetYAxis() const {
  return orientation_.Rotate({.x = 0, .y = 1, .z = 0});
}

template <math::ArithmeticConcept Number>
inline math::Vector3D<Number> RawIMUMock<Number>::GetZAxis() const {
  return orientation_.Rotate({.x = 0, .y = 0, .z = 1});
}

}  // namespace hydrolib::sensors