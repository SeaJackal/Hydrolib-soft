#pragma once

#include <cstring>
#include <iostream>
#include <unistd.h>

#include "hydrolib_device_manager.hpp"
#include "hydrolib_shell.hpp"
#include "hydrolib_stream_device.hpp"
#include "hydrv_thruster.hpp"

namespace hydrolib::shell
{
class GetSpeedCommand
{
public:
    GetSpeedCommand(int argc, char *argv[]);

public:
    int Run();

private:
    hydrv::thruster::Thruster *thruster_device;
    int return_code_;
};

int GetSpeed(int argc, char *argv[]);

inline GetSpeedCommand::GetSpeedCommand(int argc, char *argv[])
    : thruster_device(nullptr), return_code_(0)
{
    int opt = getopt(argc, argv, "-:h");
    while (opt != -1)
    {
        device::Device *finded_device = nullptr;
        switch (opt)
        {
        case 'h':
            cout << "Usage: getspeed [DEVICE_NAME]";
            g_is_running = false;
            return;
        case 1:
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
}

inline int GetSpeedCommand::Run()
{
    if (!g_is_running)
    {
        return return_code_;
    }
    int speed_value = thruster_device->GetSpeed();
    cout << "Thruster speed: " << speed_value;

    return return_code_;
}

inline int GetSpeed(int argc, char *argv[])
{
    GetSpeedCommand handler(argc, argv);
    return handler.Run();
}
} // namespace hydrolib::shell
