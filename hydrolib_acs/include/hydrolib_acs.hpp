#ifndef HYDROLIB_ACS_H_
#define HYDROLIB_ACS_H_

#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

#include "hydrolib_pid.hpp"
// добавляем библиотеки датчиков

namespace hydrolib::controlling
{
// структура для данных с imu
struct IMUData
{
    int roll_mdeg;
    int pitch_mdeg;
    int yaw_mdeg;

    int roll_rate_mdeg_per_s;
    int pitch_rate_mdeg_per_s;
    int yaw_rate_mdeg_per_s;
};

// структура для данных с датчика давления
struct PressureSensorData
{
    int depth;
};

// структура для данных, посылаемых на движки
struct ThrusterControlData
{
    int yaw_torque;
    int pitch_torque;
    int roll_torque;
};

template <typename IMUModel, typename PressureModel>
class ControlSystem
{
public:
    // Принимаем конкретные модели датчиков
    ControlSystem(IMUModel &imu_model, PressureModel &pressure_model)
        : imu_(imu_model),
          pressure_sensor_(pressure_model),
          imu_data_({.roll_mdeg = 0,
                     .pitch_mdeg = 0,
                     .yaw_mdeg = 0,

                     .roll_rate_mdeg_per_s = 0,
                     .pitch_rate_mdeg_per_s = 0,
                     .yaw_rate_mdeg_per_s = 0}),
          yaw_mdeg_(0),
          pitch_mdeg_(0),
          roll_mdeg_(0)
    {
    }

    void SetControl(int yaw_mdeg, int pitch_mdeg, int roll_mdeg)
    {
        yaw_mdeg_ = yaw_mdeg;
        pitch_mdeg_ = pitch_mdeg;
        roll_mdeg_ = roll_mdeg;
    }

    

private:
    IMUModel &imu_;
    PressureModel &pressure_sensor_;

    IMUData imu_data_;
    // PressureSensorData pressure_sensor_data_;

    int yaw_mdeg_;
    int pitch_mdeg_;
    int roll_mdeg_;

    void ReadSensors_()
    {
        imu_data_ = imu_.GetData();
        //pressure_sensor_data_ = pressure_sensor_.GetData();
    }
};
}; // namespace hydrolib::controlling

#endif