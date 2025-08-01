#include "hydrolib_acs.hpp"

#include <gtest/gtest.h>
#include <iostream>

using namespace hydrolib::controlling;
class TestROV
{
public:
    constexpr static double kTau = 0.1;
    constexpr static double kPeriod_s = 0.01;

public:
    IMUData GetData() { return imu_data_; }
    void SetControl(ThrusterControlData thruster_control)
    {
        thruster_control_last_ = thruster_control_current_;
        thruster_control_current_ = thruster_control;
    }
    void Process()
    {
        int last_yaw = imu_data_.yaw_mdeg;
        int last_pitch = imu_data_.pitch_mdeg;
        int last_roll = imu_data_.roll_mdeg;

        imu_data_.yaw_mdeg =
            1 / (2 * kTau + kPeriod_s) *
            (kPeriod_s * (thruster_control_current_.yaw_torque +
                          thruster_control_last_.yaw_torque) -
             (kPeriod_s - 2 * kTau) * last_yaw);

        imu_data_.pitch_mdeg =
            1 / (2 * kTau + kPeriod_s) *
            (kPeriod_s * (thruster_control_current_.pitch_torque +
                          thruster_control_last_.pitch_torque) -
             (kPeriod_s - 2 * kTau) * last_pitch);

        imu_data_.roll_mdeg =
            1 / (2 * kTau + kPeriod_s) *
            (kPeriod_s * (thruster_control_current_.roll_torque +
                          thruster_control_last_.roll_torque) -
             (kPeriod_s - 2 * kTau) * last_roll);

        imu_data_.yaw_rate_mdeg_per_s =
            2 * (imu_data_.yaw_mdeg - last_yaw) / kPeriod_s -
            imu_data_.yaw_rate_mdeg_per_s;

        imu_data_.pitch_rate_mdeg_per_s =
            2 * (imu_data_.pitch_mdeg - last_pitch) / kPeriod_s -
            imu_data_.pitch_rate_mdeg_per_s;

        imu_data_.roll_rate_mdeg_per_s =
            2 * (imu_data_.roll_mdeg - last_roll) / kPeriod_s -
            imu_data_.roll_rate_mdeg_per_s;
    }

private:
    IMUData imu_data_ = {0, 0, 0, 0, 0, 0};
    ThrusterControlData thruster_control_current_ = {0, 0, 0};
    ThrusterControlData thruster_control_last_ = {0, 0, 0};
    // ClosingCircuits closing_circuits = {1, 1, 1, 0};
};

TEST(TestACS, ControlTest)
{
    TestROV rov;
    ControlSystem<TestROV, TestROV, TestROV,
                  static_cast<int>(1 / TestROV::kPeriod_s)>
        control_system(rov, rov, rov);
    int yaw = 180000;
    int pitch = 90000;
    int roll = 180000;
    control_system.SetYawP(1);
    control_system.SetYawI(10);
    control_system.SetYawDivideShift(0);

    control_system.SetPitchP(1);
    control_system.SetPitchI(10);
    control_system.SetPitchDivideShift(0);

    control_system.SetRollP(1);
    control_system.SetRollI(10);
    control_system.SetRollDivideShift(0);

    control_system.SetYawRateP(1);
    control_system.SetYawRateI(0);
    control_system.SetYawRateDivideShift(0);

    control_system.SetPitchRateP(1);
    control_system.SetPitchRateI(0);
    control_system.SetPitchRateDivideShift(0);

    control_system.SetRollRateP(1);
    control_system.SetRollRateI(0);
    control_system.SetRollRateDivideShift(0);

    control_system.SetControl(yaw, pitch, roll);

    // control_system.OpenYawContour();

    for (int i = 0; i < 1000; i++)
    {
        control_system.Process();
        rov.Process();
        std::cout << rov.GetData().yaw_mdeg << std::endl;
        // std::cout << rov.GetData().pitch_mdeg << std::endl;
        // std::cout << rov.GetData().roll_mdeg << std::endl;
    }
    EXPECT_NEAR(yaw, rov.GetData().yaw_mdeg, yaw * 0.1);
    // EXPECT_NEAR(pitch, rov.GetData().pitch_mdeg, pitch * 0.1);
    // EXPECT_NEAR(roll, rov.GetData().roll_mdeg, roll * 0.1);
}