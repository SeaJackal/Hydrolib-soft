#pragma once

#include "hydrolib_device.hpp"

namespace hydrolib::device {
class IThrustGenerator : public Device {
 public:
  static constexpr DeviceType kSelfType = DeviceType::THRUSTGENERATOR;

 public:
  IThrustGenerator(std::string_view name) : Device(name, kSelfType) {}

 public:
  virtual void ProcessWithFeedback(void* control, void* dest) const = 0;
};

template <typename ThrustGenerator, typename Control, typename ThrustArray>
class ThrustGeneratorDevice : public IThrustGenerator {
 public:
  ThrustGeneratorDevice(std::string_view name,
                        ThrustGenerator& thrust_generator);

  void ProcessWithFeedback(void* control, void* dest) const override {
    auto* typed_control = static_cast<Control*>(control);
    auto* typed_dest = static_cast<ThrustArray*>(dest);
    ProcessWithFeedback(*typed_control, *typed_dest);
  }
  void ProcessWithFeedback(Control& control, ThrustArray& dest) const;

 private:
  ThrustGenerator& thrust_generator_;
};

template <typename ThrustGenerator, typename Control, typename ThrustArray>
ThrustGeneratorDevice<ThrustGenerator, Control,
                      ThrustArray>::ThrustGeneratorDevice(std::string_view name,
                                                          ThrustGenerator&
                                                              thrust_generator)
    : IThrustGenerator(name), thrust_generator_(thrust_generator) {}

template <typename ThrustGenerator, typename Control, typename ThrustArray>
void ThrustGeneratorDevice<ThrustGenerator, Control,
                           ThrustArray>::ProcessWithFeedback(Control& control,
                                                             ThrustArray& dest)
    const {
  return thrust_generator_.ProcessWithFeedback(control, dest);
}
}  // namespace hydrolib::device
