#pragma once

namespace hydrolib::controlling
{

struct ThrustersControlData
{
    int yaw_torque = 0;    // 
    int pitch_torque = 0;  // 
    int roll_torque = 0;   // 
    int depth_torque = 0;  // z_

    int x_torque = 0;      // 
    int y_torque = 0;      // 

    void reset()
    {
        yaw_torque = pitch_torque = roll_torque = depth_torque = x_torque =
            y_torque = 0;
    }
};

extern ThrustersControlData thrusters_data;

} // namespace hydrolib::controlling