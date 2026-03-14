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
  virtual void ControlProccess(void* control, void* dest) const = 0;
};

template <int THRUSTERS_COUNT>
class ThrustGeneratorDevice : public IControlSystem {
 public:
  ThrustGeneratorDevice(
      std::string_view name,
      controlling::ThrustGenerator<THRUSTERS_COUNT>& thrust_generator);

  using ThrustArray = std::array<math::FixedPointBase, THRUSTERS_COUNT>;

  void ControlProccess(void* control, void* dest) const override;

 private:
  controlling::ThrustGenerator<THRUSTERS_COUNT>& thrust_generator_;
};

template <int THRUSTERS_COUNT>
ThrustGeneratorDevice<THRUSTERS_COUNT>::ThrustGeneratorDevice(
    std::string_view name,
    controlling::ThrustGenerator<THRUSTERS_COUNT>& thrust_generator)
    : IControlSystem(name), thrust_generator_(thrust_generator) {}

template <int THRUSTERS_COUNT>
void ThrustGeneratorDevice<THRUSTERS_COUNT>::ControlProccess(void* control,
                                                             void* dest) const {
  auto* typed_control = static_cast<controlling::Control*>(control);
  auto* typed_dest = static_cast<ThrustArray*>(dest);
  return ProcessWithFeedback(*typed_control, *typed_dest);
}

}  // namespace hydrolib::device
