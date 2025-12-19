#pragma once

#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "hydrolib_device_manager.hpp"
#include "hydrolib_shell.hpp"
#include "hydrolib_stream_device.hpp"
#include "hydrv_thruster.hpp"

namespace hydrolib::shell
{
class SetSpeedCommand
{
public:
    SetSpeedCommand(int argc, char *argv[]);

public:
    int Run();

private:
    hydrv::thruster::Thruster *thruster_device;
    int target_speed;
    bool speed_received_;
    int return_code_;
};

int SetSpeed(int argc, char *argv[]);

inline SetSpeedCommand::SetSpeedCommand(int argc, char *argv[])
    : thruster_device(nullptr),
      target_speed(0),
      speed_received_(false),
      return_code_(0)
{
    int arg_counter = 0;

    int opt = getopt(argc, argv, "-:h");
    while (opt != -1)
    {
        device::Device *finded_device = nullptr;
        switch (opt)
        {
        case 'h':
            cout << "Usage: setspeed [DEVICE_NAME] [SPEED]";
            g_is_running = false;
            return;
        case 1:
            if (arg_counter == 0)
            {
                if (thruster_device == nullptr)
                {
                    finded_device = (*device::g_device_manager)[optarg];
                    if (finded_device == nullptr)
                    {
                        cout << "Device not found: " << optarg;
                        g_is_running = false;
                        return_code_ = -1;
                        return;
                    }
                    thruster_device =
                        finded_device->Upcast<hydrv::thruster::Thruster>();
                    if (thruster_device == nullptr)
                    {
                        cout << "Device is not a thruster: " << optarg;
                        g_is_running = false;
                        return_code_ = -1;
                        return;
                    }
                    arg_counter++;
                }
            }
            else if (arg_counter == 1)
            {
                target_speed = std::atoi(optarg);
                arg_counter++;
                speed_received_ = true;
            }
            break;
        default:
            cout << "Invalid option: " << static_cast<char>(optopt);
            g_is_running = false;
            return_code_ = -1;
            return;
        }
        opt = getopt(argc, argv, "-:h");
    }
    if (thruster_device == nullptr)
    {
        cout << "No thruster device specified";
        g_is_running = false;
        return_code_ = -1;
    }
    else if (!speed_received_)
    {
        cout << "No speed value specified";
        g_is_running = false;
        return_code_ = -1;
    }
}

inline int SetSpeedCommand::Run()
{
    if (!g_is_running)
    {
        return return_code_;
    }
    thruster_device->SetSpeed(target_speed);
    return return_code_;
}

inline int SetSpeed(int argc, char *argv[])
{
    SetSpeedCommand handler(argc, argv);
    return handler.Run();
}
} // namespace hydrolib::shell
