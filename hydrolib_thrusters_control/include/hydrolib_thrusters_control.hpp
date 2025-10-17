#pragma once

namespace hydrolib::controlling
{
struct ThrustersControlData
{
    int yaw_torque;
    int pitch_torque;
    int roll_torque;

    int x_force;
    int y_force;
    int z_force;
};
} // namespace hydrolib::controlling