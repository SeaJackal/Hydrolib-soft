#pragma once

#include <string_view>

namespace hydrolib::device
{

enum class DeviceType
{
    STREAM
};

class Device
{
public:
    Device(std::string_view name, DeviceType type);

public:
    std::string_view GetName() const;

    template <typename T>
    T *Upcast();

private:
    std::string_view name_;
    DeviceType type_;
};

inline Device::Device(std::string_view name, DeviceType type)
    : name_(name), type_(type)
{
}

inline std::string_view Device::GetName() const { return name_; }

template <typename T>
T *Device::Upcast()
{
    if (T::kSelfType == type_)
    {
        return static_cast<T *>(this);
    }
    return nullptr;
}

} // namespace hydrolib::device