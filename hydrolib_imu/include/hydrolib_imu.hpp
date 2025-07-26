#ifndef HYDROLIB_IMU_H_
#define HYDROLIB_IMU_H_

template<typename Model>
class IMUSensor {
public:
    // Метод для получения данных от IMU
    auto GetData() {
        return static_cast<Model*>(this)->ReadIMUData();
    }
};

class IMU456 : public IMUSensor<IMU456> {
public:
    struct IMUData {
        int roll_mdeg;
        int pitch_mdeg;
        int yaw_mdeg;
        
        int roll_rate_mdeg_per_s;
        int pitch_rate_mdeg_per_s;
        int yaw_rate_mdeg_per_s;
    };

    IMUData ReadIMUData() {
        // Реализация для <модель>
        return IMUData{};
    }
};

// Модель IMU321
class IMU321 : public IMUSensor<IMU321> {
public:
    struct IMUData {
        double roll;
        double pitch;
        double yaw;
        double lag;
        double march;
    };

    IMUData ReadIMUData() {
        // Реализация для <модель>
        return IMUData{};
    }
};

#endif