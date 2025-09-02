#ifndef HYDROLIB_IMU_H_
#define HYDROLIB_IMU_H_

#include <concepts>

namespace hydrolib::sensors
{

struct IMUData
{
    int yaw_mdeg;
    int pitch_mdeg;
    int roll_mdeg;

    int yaw_rate_mdeg_per_s;
    int pitch_rate_mdeg_per_s;
    int roll_rate_mdeg_per_s;
};

template<typename T>
concept IMUConcept = requires(T imu) {
    { imu.GetData() } -> std::same_as<IMUData>;
};

template<typename Model>
class IMUSensor {
public:
    auto GetData() {
        return static_cast<Model*>(this)->ReadIMUData();
    }
};

class IMU456 : public IMUSensor<IMU456> {
public:
    IMUData ReadIMUData() {
        IMUData data{};
        // something
        return data;
    }
};

class IMU321 : public IMUSensor<IMU321> {
public:
    IMUData ReadIMUData() {
        IMUData data{};
        // something
        return data;
    }
};

static_assert(IMUConcept<IMU456>);
static_assert(IMUConcept<IMU321>);

} // namespace hydrolib::sensors

#endif