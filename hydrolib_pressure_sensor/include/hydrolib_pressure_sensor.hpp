#pragma once

#include <concepts>

namespace hydrolib::sensors
{

struct PressureSensorData
{
    int depth_mm;
    int depth_rate_mm_per_s;
};

template <typename T>
concept PressureSensorConcept = requires(T sensor) {
    { sensor.GetPressureData() } -> std::same_as<PressureSensorData>;
};

class MS5837
{
public:
    PressureSensorData GetPressureData()
    {
        PressureSensorData data{};

        // получение данных
        return data;
    }
};

class WIKA
{
public:
    PressureSensorData GetPressureData()
    {
        PressureSensorData data{};

        // int raw_pressure = read_pressure_from_sensor();
        // data.depth_mm = static_cast<int>(calculate_depth(raw_pressure));
        return data;
    }
};

static_assert(PressureSensorConcept<MS5837>);
static_assert(PressureSensorConcept<WIKA>);

} // namespace hydrolib::sensors