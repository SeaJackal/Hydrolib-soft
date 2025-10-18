#pragma once

#include "hydrolib_thrusters_control.hpp"

namespace hydrolib::controlling
{

template <unsigned THRUSTERS_COUNT, bool ENABLE_SUM_CLAMP = false>
class ThrustGenerator
{

public:
    constexpr ThrustGenerator(int *x_rotation_gain, int *y_rotation_gain,
                              int *z_rotation_gain, int *x_linear_gain,
                              int *y_linear_gain, int *z_linear_gain,
                              int single_clamp, int sum_clamp = 0);

public:
    void ProcessWithFeedback(ThrustersControlData &thrusters_control_data,
                             int *dest);

private:
    int x_rotation_gain_[THRUSTERS_COUNT];
    int y_rotation_gain_[THRUSTERS_COUNT];
    int z_rotation_gain_[THRUSTERS_COUNT];

    int x_linear_gain_[THRUSTERS_COUNT];
    int y_linear_gain_[THRUSTERS_COUNT];
    int z_linear_gain_[THRUSTERS_COUNT];

    int single_clamp_;

    int sum_clamp_;
};

template <unsigned THRUSTERS_COUNT, bool ENABLE_SUM_CLAMP>
constexpr ThrustGenerator<THRUSTERS_COUNT, ENABLE_SUM_CLAMP>::ThrustGenerator(
    int *x_rotation_gain, int *y_rotation_gain, int *z_rotation_gain,
    int *x_linear_gain, int *y_linear_gain, int *z_linear_gain,
    int single_clamp, int sum_clamp)
    : single_clamp_(single_clamp), sum_clamp_(sum_clamp)
{
    for (unsigned i = 0; i < THRUSTERS_COUNT; i++)
    {
        x_rotation_gain_[i] = x_rotation_gain[i];
        x_linear_gain_[i] = x_linear_gain[i];

        y_rotation_gain_[i] = y_rotation_gain[i];
        y_linear_gain_[i] = y_linear_gain[i];

        z_rotation_gain_[i] = z_rotation_gain[i];
        z_linear_gain_[i] = z_linear_gain[i];
    }
}

template <unsigned THRUSTERS_COUNT, bool ENABLE_SUM_CLAMP>
void ThrustGenerator<THRUSTERS_COUNT, ENABLE_SUM_CLAMP>::ProcessWithFeedback(
    ThrustersControlData &thrusters_control_data, int *dest)
{
    int max = 0;
    int sum = 0;
    for (unsigned i = 0; i < THRUSTERS_COUNT; i++)
    {
        dest[i] = x_rotation_gain_[i] * thrusters_control_data.roll_torque +
                  x_linear_gain_[i] * thrusters_control_data.x_force +
                  y_rotation_gain_[i] * thrusters_control_data.pitch_torque +
                  y_linear_gain_[i] * thrusters_control_data.y_force +
                  z_rotation_gain_[i] * thrusters_control_data.yaw_torque +
                  z_linear_gain_[i] * thrusters_control_data.z_force;
        int dest_abs = dest[i] >= 0 ? dest[i] : -dest[i];

        if (dest_abs > max)
        {
            max = dest_abs;
        }

        if constexpr (ENABLE_SUM_CLAMP)
        {
            sum += dest_abs;
        }
    }

    unsigned enumerator = 1;
    unsigned denumerator = 1;

    if (max > single_clamp_)
    {
        enumerator = single_clamp_;
        denumerator = max;
    }

    if constexpr (ENABLE_SUM_CLAMP)
    {
        if (sum > sum_clamp_)
        {
            if (sum_clamp_ * denumerator < sum * enumerator)
            {
                enumerator = sum_clamp_;
                denumerator = sum;
            }
        }
    }

    if (enumerator == denumerator)
    {
        for (unsigned i = 0; i < THRUSTERS_COUNT; i++)
        {
            dest[i] = dest[i] * enumerator / denumerator;
        }
        thrusters_control_data.roll_torque =
            thrusters_control_data.roll_torque * enumerator / denumerator;
        thrusters_control_data.pitch_torque =
            thrusters_control_data.pitch_torque * enumerator / denumerator;
        thrusters_control_data.yaw_torque =
            thrusters_control_data.yaw_torque * enumerator / denumerator;
        thrusters_control_data.x_force =
            thrusters_control_data.x_force * enumerator / denumerator;
        thrusters_control_data.y_force =
            thrusters_control_data.y_force * enumerator / denumerator;
        thrusters_control_data.z_force =
            thrusters_control_data.z_force * enumerator / denumerator;
    }
}
} // namespace hydrolib::controlling
