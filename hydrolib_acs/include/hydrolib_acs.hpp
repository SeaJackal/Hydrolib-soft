#ifndef HYDROLIB_ACS_H_
#define HYDROLIB_ACS_H_

#include <cstdint>
#include <sys/ucontext.h>

#include "hydrolib_pid.hpp"
// добавляем библиотеки датчиков

namespace hydrolib::controlling
{
struct IMUData
{
    int yaw_mdeg;
    int pitch_mdeg;
    int roll_mdeg;

    int yaw_rate_mdeg_per_s;
    int pitch_rate_mdeg_per_s;
    int roll_rate_mdeg_per_s; // TODO: xyz_rate
};

struct PressureSensorData
{
    int depth;
};

struct ThrusterControlData
{
    int yaw_torque;
    int pitch_torque;
    int roll_torque;
};

struct ClosingContours
{
    int yaw_circuit;
    int pitch_circuit;
    int roll_circuit;
    int depth_curcuit;
};

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
class ControlSystem
{
public:
    ControlSystem(IMUModel &imu_model, PressureModel &pressure_model,
                  Thrusters &thruster)
        : imu_(imu_model),
          pressure_sensor_(pressure_model),
          thruster_(thruster),
          closing_contours_(1, 1, 1, 1),
          yaw_control_mdeg_(0),
          pitch_control_mdeg_(0),
          roll_control_mdeg_(0)
    {
    }

    void SetControl(int yaw_mdeg, int pitch_mdeg, int roll_mdeg)
    {
        yaw_control_mdeg_ = yaw_mdeg;
        pitch_control_mdeg_ = pitch_mdeg;
        roll_control_mdeg_ = roll_mdeg;
    }

    void SetYawP(unsigned p);
    void SetYawI(unsigned i);
    void SetYawDivideShift(unsigned divide_shift);

    void SetPitchP(unsigned p);
    void SetPitchI(unsigned i);
    void SetPitchDivideShift(unsigned divide_shift);

    void SetRollP(unsigned p);
    void SetRollI(unsigned i);
    void SetRollDivideShift(unsigned divide_shift);

    void SetYawRateP(unsigned p);
    void SetYawRateI(unsigned i);
    void SetYawRateDivideShift(unsigned divide_shift);

    void SetPitchRateP(unsigned p);
    void SetPitchRateI(unsigned i);
    void SetPitchRateDivideShift(unsigned divide_shift);

    void SetRollRateP(unsigned p);
    void SetRollRateI(unsigned i);
    void SetRollRateDivideShift(unsigned divide_shift);

    void CloseYawContour();
    void ClosePitchContour();
    void CloseRollContour();
    void CloseDepthContour();

    void OpenYawContour();
    void OpenPitchContour();
    void OpenRollContour();
    void OpenDepthContour();

    void Process();

private:
    IMUModel &imu_;
    PressureModel &pressure_sensor_;
    Thrusters &thruster_;
    ClosingContours closing_contours_;

    PID<FREQ_HZ> yaw_pid_;
    PID<FREQ_HZ> pitch_pid_;
    PID<FREQ_HZ> roll_pid_;

    PID<FREQ_HZ> yaw_rate_pid_;
    PID<FREQ_HZ> pitch_rate_pid_;
    PID<FREQ_HZ> roll_rate_pid_;

    int yaw_control_mdeg_;
    int pitch_control_mdeg_;
    int roll_control_mdeg_;
};

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters, FREQ_HZ>::SetYawP(
    unsigned p)
{
    yaw_pid_.SetP(p);
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters, FREQ_HZ>::SetYawI(
    unsigned i)
{
    yaw_pid_.SetI(i);
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters,
                   FREQ_HZ>::SetYawDivideShift(unsigned divide_shift)
{
    yaw_pid_.SetDivideShift(divide_shift);
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters, FREQ_HZ>::SetPitchP(
    unsigned p)
{
    pitch_pid_.SetP(p);
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters, FREQ_HZ>::SetPitchI(
    unsigned i)
{
    pitch_pid_.SetI(i);
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters,
                   FREQ_HZ>::SetPitchDivideShift(unsigned divide_shift)
{
    pitch_pid_.SetDivideShift(divide_shift);
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters, FREQ_HZ>::SetRollP(
    unsigned p)
{
    roll_pid_.SetP(p);
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters, FREQ_HZ>::SetRollI(
    unsigned i)
{
    roll_pid_.SetI(i);
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters,
                   FREQ_HZ>::SetRollDivideShift(unsigned divide_shift)
{
    roll_pid_.SetDivideShift(divide_shift);
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters, FREQ_HZ>::SetYawRateP(
    unsigned p)
{
    yaw_rate_pid_.SetP(p);
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters, FREQ_HZ>::SetYawRateI(
    unsigned i)
{
    yaw_rate_pid_.SetI(i);
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters,
                   FREQ_HZ>::SetYawRateDivideShift(unsigned divide_shift)
{
    yaw_rate_pid_.SetDivideShift(divide_shift);
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters, FREQ_HZ>::SetPitchRateP(
    unsigned p)
{
    pitch_rate_pid_.SetP(p);
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters, FREQ_HZ>::SetPitchRateI(
    unsigned i)
{
    pitch_rate_pid_.SetI(i);
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters,
                   FREQ_HZ>::SetPitchRateDivideShift(unsigned divide_shift)
{
    pitch_rate_pid_.SetDivideShift(divide_shift);
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters, FREQ_HZ>::SetRollRateP(
    unsigned p)
{
    roll_rate_pid_.SetP(p);
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters, FREQ_HZ>::SetRollRateI(
    unsigned i)
{
    roll_rate_pid_.SetI(i);
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters,
                   FREQ_HZ>::SetRollRateDivideShift(unsigned divide_shift)
{
    roll_rate_pid_.SetDivideShift(divide_shift);
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters,
                   FREQ_HZ>::CloseYawContour()
{
    closing_contours_.yaw_circuit = 1;
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters,
                   FREQ_HZ>::ClosePitchContour()
{
    closing_contours_.pitch_circuit = 1;
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters,
                   FREQ_HZ>::CloseRollContour()
{
    closing_contours_.roll_circuit = 1;
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters,
                   FREQ_HZ>::CloseDepthContour()
{
    closing_contours_.depth_curcuit = 1;
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters,
                   FREQ_HZ>::OpenYawContour()
{
    closing_contours_.yaw_circuit = 0;
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters,
                   FREQ_HZ>::OpenPitchContour()
{
    closing_contours_.pitch_circuit = 0;
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters,
                   FREQ_HZ>::OpenRollContour()
{
    closing_contours_.roll_circuit = 0;
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters,
                   FREQ_HZ>::OpenDepthContour()
{
    closing_contours_.depth_curcuit = 0;
}

template <typename IMUModel, typename PressureModel, typename Thrusters,
          unsigned FREQ_HZ>
void ControlSystem<IMUModel, PressureModel, Thrusters, FREQ_HZ>::Process()
{
    IMUData imu_data = imu_.GetData();
    int yaw_epsilon =
        yaw_control_mdeg_ - imu_data.yaw_mdeg * closing_contours_.yaw_circuit;
    int pitch_epsilon = pitch_control_mdeg_ -
                        imu_data.pitch_mdeg * closing_contours_.pitch_circuit;
    int roll_epsilon = roll_control_mdeg_ -
                       imu_data.roll_mdeg * closing_contours_.roll_circuit;

    int yaw_rate_epsilon =
        yaw_pid_.Process(yaw_epsilon) -
        imu_data.yaw_rate_mdeg_per_s * closing_contours_.yaw_circuit;
    int pitch_rate_epsilon =
        pitch_pid_.Process(pitch_epsilon) -
        imu_data.pitch_rate_mdeg_per_s * closing_contours_.pitch_circuit;
    int roll_rate_epsilon =
        roll_pid_.Process(roll_epsilon) -
        imu_data.roll_rate_mdeg_per_s * closing_contours_.roll_circuit;

    ThrusterControlData thruster_control = {
        .yaw_torque = yaw_rate_pid_.Process(yaw_rate_epsilon),
        .pitch_torque = pitch_rate_pid_.Process(pitch_rate_epsilon),
        .roll_torque = roll_rate_pid_.Process(roll_rate_epsilon)};
    thruster_.SetControl(thruster_control);
}

}; // namespace hydrolib::controlling

#endif