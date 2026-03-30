#ifndef HYDROLIB_ACS_H_
#define HYDROLIB_ACS_H_

#include <sys/ucontext.h>

#include "hydrolib_imu.hpp"
#include "hydrolib_pid.hpp"
#include "hydrolib_thrust_generator.hpp"

namespace hydrolib::controlling {
template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
class ControlSystem {
 public:
  ControlSystem(IMU &imu, ThrustGenerator &thrust_generator);

  void SetControl(math::Rotation<Number> rotation,
                  math::Vector3D<Number> linear);

  void SetYawP(Number p_coeff);
  void SetYawI(Number i_coeff);
  void SetYawD(Number d_coeff);

  void SetPitchP(Number p_coeff);
  void SetPitchI(Number i_coeff);
  void SetPitchD(Number d_coeff);

  void SetRollP(Number p_coeff);
  void SetRollI(Number i_coeff);
  void SetRollD(Number d_coeff);

  void CloseYawContour();
  void ClosePitchContour();
  void CloseRollContour();

  void OpenYawContour();
  void OpenPitchContour();
  void OpenRollContour();

  void Process();

 private:
  IMU &imu_;
  ThrustGenerator &thrust_generator_;

  PID<FREQ_HZ, Number> yaw_pid_;
  PID<FREQ_HZ, Number> pitch_pid_;
  PID<FREQ_HZ, Number> roll_pid_;

  bool is_yaw_closed_ = true;
  bool is_pitch_closed_ = true;
  bool is_roll_closed_ = true;

  math::Rotation<Number> target_rotation_;
  math::Vector3D<Number> target_linear_;
};

template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
ControlSystem<IMU, ThrustGenerator, FREQ_HZ, Number>::ControlSystem(
    IMU &imu, ThrustGenerator &thrust_generator)
    : imu_(imu), thrust_generator_(thrust_generator) {}

template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
void ControlSystem<IMU, ThrustGenerator, FREQ_HZ, Number>::SetControl(
    math::Rotation<Number> rotation, math::Vector3D<Number> linear) {
  target_rotation_ = rotation;
  target_linear_ = linear;
}

template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
void ControlSystem<IMU, ThrustGenerator, FREQ_HZ, Number>::CloseYawContour() {
  is_yaw_closed_ = true;
}

template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
void ControlSystem<IMU, ThrustGenerator, FREQ_HZ, Number>::ClosePitchContour() {
  is_pitch_closed_ = true;
}

template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
void ControlSystem<IMU, ThrustGenerator, FREQ_HZ, Number>::CloseRollContour() {
  is_roll_closed_ = true;
}

template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
void ControlSystem<IMU, ThrustGenerator, FREQ_HZ, Number>::OpenYawContour() {
  is_yaw_closed_ = false;
}

template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
void ControlSystem<IMU, ThrustGenerator, FREQ_HZ, Number>::OpenPitchContour() {
  is_pitch_closed_ = false;
}

template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
void ControlSystem<IMU, ThrustGenerator, FREQ_HZ, Number>::OpenRollContour() {
  is_roll_closed_ = false;
}

template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
void ControlSystem<IMU, ThrustGenerator, FREQ_HZ, Number>::SetYawP(
    Number p_coeff) {
  yaw_pid_.SetP(p_coeff);
}

template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
void ControlSystem<IMU, ThrustGenerator, FREQ_HZ, Number>::SetYawI(
    Number i_coeff) {
  yaw_pid_.SetI(i_coeff);
}

template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
void ControlSystem<IMU, ThrustGenerator, FREQ_HZ, Number>::SetYawD(
    Number d_coeff) {
  yaw_pid_.SetD(d_coeff);
}

template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
void ControlSystem<IMU, ThrustGenerator, FREQ_HZ, Number>::SetPitchP(
    Number p_coeff) {
  pitch_pid_.SetP(p_coeff);
}

template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
void ControlSystem<IMU, ThrustGenerator, FREQ_HZ, Number>::SetPitchI(
    Number i_coeff) {
  pitch_pid_.SetI(i_coeff);
}

template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
void ControlSystem<IMU, ThrustGenerator, FREQ_HZ, Number>::SetPitchD(
    Number d_coeff) {
  pitch_pid_.SetD(d_coeff);
}

template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
void ControlSystem<IMU, ThrustGenerator, FREQ_HZ, Number>::SetRollP(
    Number p_coeff) {
  roll_pid_.SetP(p_coeff);
}

template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
void ControlSystem<IMU, ThrustGenerator, FREQ_HZ, Number>::SetRollI(
    Number i_coeff) {
  roll_pid_.SetI(i_coeff);
}

template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
void ControlSystem<IMU, ThrustGenerator, FREQ_HZ, Number>::SetRollD(
    Number d_coeff) {
  roll_pid_.SetD(d_coeff);
}

template <typename IMU, typename ThrustGenerator, int FREQ_HZ, typename Number>
  requires sensors::IMUConcept<IMU, Number>
void ControlSystem<IMU, ThrustGenerator, FREQ_HZ, Number>::Process() {
  math::Rotation<Number> current_orientation = imu_.GetOrientation();
  auto delta = target_rotation_ - current_orientation;
  auto vector_part = delta.GetVectorPart();

  auto yaw_delta = vector_part.z;
  auto pitch_delta = vector_part.y;
  auto roll_delta = vector_part.x;

  Control<Number> control = {
      .x_force = target_linear_.x,
      .y_force = target_linear_.y,
      .z_force = target_linear_.z,
      .x_torque = is_roll_closed_ ? roll_pid_.Process(roll_delta) : 0,
      .y_torque = is_pitch_closed_ ? pitch_pid_.Process(pitch_delta) : 0,
      .z_torque = is_yaw_closed_ ? yaw_pid_.Process(yaw_delta) : 0};

  thrust_generator_.ProcessWithFeedback(control);

  yaw_pid_.RefineOutput(control.z_torque);
  pitch_pid_.RefineOutput(control.y_torque);
  roll_pid_.RefineOutput(control.x_torque);
}

};  // namespace hydrolib::controlling

#endif