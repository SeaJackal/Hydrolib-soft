#pragma once

#include "hydrolib_device.hpp"

#include "hydrolib_return_codes.hpp"

namespace hydrolib::device
{
class IThruster : public Device
{
public:
    static constexpr DeviceType kSelfType = DeviceType::THRUSTER;

public:
    IThruster(std::string_view name) : Device(name, kSelfType) {}

public:
    virtual hydrolib::ReturnCode SetSpeed(int speed) = 0;
    virtual int GetSpeed() = 0;
};

template <typename Thruster>
class ThrusterDevice : public IThruster
{
public:
    ThrusterDevice(std::string_view name, Thruster &thruster);

    hydrolib::ReturnCode SetSpeed(int speed) override;
    int GetSpeed() override;

private:
    Thruster &thruster_;
    int current_speed_;
};

template <typename Thruster>
ThrusterDevice<Thruster>::ThrusterDevice(std::string_view name,
                                         Thruster &thruster)
    : IThruster(name), thruster_(thruster)
{
}
} // namespace hydrolib::device
