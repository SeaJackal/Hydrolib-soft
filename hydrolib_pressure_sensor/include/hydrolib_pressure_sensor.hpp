#ifndef HYDROLIB_PRESSURE_H_
#define HYDROLIB_PRESSURE_H_

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

template <typename Model>
class PressureSensor
{
public:
    auto GetPressureData() { return static_cast<Model *>(this)->ReadPressureData(); }
};

class MS5837 : public PressureSensor<MS5837>
{
public:
    PressureSensorData ReadPressureData()
    {
        PressureSensorData data{};

        // получение данных
        return data;
    }
};

class WIKA : public PressureSensor<WIKA>
{
public:
    PressureSensorData ReadPressureData()
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

#endif