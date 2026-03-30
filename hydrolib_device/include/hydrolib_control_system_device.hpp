#pragma once

#include "hydrolib_device.hpp"
#include "hydrolib_thrust_generator.hpp"

namespace hydrolib::device {
class IControlSystem : public Device {
 public:
  static constexpr DeviceType kSelfType = DeviceType::CONTROLSYSTEM;

 public:
  IControlSystem(std::string_view name) : Device(name, kSelfType) {}

 public:
  virtual void ControlProcess(controlling::Control<math::FixedPointBase>& control) = 0;
};

template <controlling::ThrusterConcept Thruster, int THRUSTERS_COUNT>
class ThrustGeneratorDevice : public IControlSystem {
 public:
  ThrustGeneratorDevice(std::string_view name,
                        controlling::ThrustGenerator<Thruster, THRUSTERS_COUNT>&
                            thrust_generator);

  using ThrustArray = std::array<math::FixedPointBase, THRUSTERS_COUNT>;

  void ControlProcess(controlling::Control<math::FixedPointBase>& control) override;

 private:
  controlling::ThrustGenerator<Thruster, THRUSTERS_COUNT>& thrust_generator_;
};

template <controlling::ThrusterConcept Thruster, int THRUSTERS_COUNT>
ThrustGeneratorDevice<Thruster, THRUSTERS_COUNT>::ThrustGeneratorDevice(
    std::string_view name,
    controlling::ThrustGenerator<Thruster, THRUSTERS_COUNT>& thrust_generator)
    : IControlSystem(name), thrust_generator_(thrust_generator) {}

template <controlling::ThrusterConcept Thruster, int THRUSTERS_COUNT>
void ThrustGeneratorDevice<Thruster, THRUSTERS_COUNT>::ControlProcess(
    controlling::Control<math::FixedPointBase>& control) {
  return thrust_generator_.ProcessWithFeedback(control);
}

}  // namespace hydrolib::device
