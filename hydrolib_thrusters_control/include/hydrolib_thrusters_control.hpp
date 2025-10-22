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

template <typename T>
concept ThrusterConcept =
    requires(T thruster, ThrustersControlData thruster_control) {
        { thruster.SetControl(thruster_control) };
    };
} // namespace hydrolib::controlling