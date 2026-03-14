#pragma once

#include "hydrolib_device.hpp"
#include "hydrolib_thrust_generator.hpp"

namespace hydrolib::device {
class IControlSystem : public Device {
 public:
  static constexpr DeviceType kSelfType = DeviceType::THRUSTGENERATOR;

 public:
  IControlSystem(std::string_view name) : Device(name, kSelfType) {}

 public:
  virtual void ControlProccess(controlling::Control& control) const = 0;
};

template <controlling::ThrusterConcept Thruster, int THRUSTERS_COUNT>
class ThrustGeneratorDevice : public IControlSystem {
 public:
  ThrustGeneratorDevice(std::string_view name,
                        controlling::ThrustGenerator<Thruster, THRUSTERS_COUNT>&
                            thrust_generator);

  using ThrustArray = std::array<math::FixedPointBase, THRUSTERS_COUNT>;

  void ControlProccess(controlling::Control& control) const override;

 private:
  controlling::ThrustGenerator<Thruster, THRUSTERS_COUNT>& thrust_generator_;
};

template <controlling::ThrusterConcept Thruster, int THRUSTERS_COUNT>
ThrustGeneratorDevice<Thruster, THRUSTERS_COUNT>::ThrustGeneratorDevice(
    std::string_view name,
    controlling::ThrustGenerator<Thruster, THRUSTERS_COUNT>& thrust_generator)
    : IControlSystem(name), thrust_generator_(thrust_generator) {}

template <controlling::ThrusterConcept Thruster, int THRUSTERS_COUNT>
void ThrustGeneratorDevice<Thruster, THRUSTERS_COUNT>::ControlProccess(
    controlling::Control& control) const {
  return controlling::ThrustGenerator<
      Thruster, THRUSTERS_COUNT>::ProcessWithFeedback(control);
}

}  // namespace hydrolib::device
