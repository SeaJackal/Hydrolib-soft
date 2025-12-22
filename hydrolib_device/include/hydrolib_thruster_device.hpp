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
} // namespace hydrolib::device
