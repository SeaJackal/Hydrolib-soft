#pragma once

#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "hydrolib_device.hpp"
#include "hydrolib_device_manager.hpp"
#include "hydrolib_shell.hpp"
#include "hydrolib_thruster_device.hpp"

namespace hydrolib::shell
{
class ThrusterShell
{
public:
    ThrusterShell(int argc, char *argv[]);

public:
    int Run();

private:
    device::IThruster *thruster_device;
    int target_speed;
    bool speed_received_;
    bool setspeed_flag;
    int return_code_;
};

int ThrusterCommands(int argc, char *argv[]);

inline ThrusterShell::ThrusterShell(int argc, char *argv[])
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
            cout << "Usage: thruster setspeed [DEVICE_NAME] [SPEED] / thruster "
                    "getspeed [DEVICE_NAME]";
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
                        finded_device->Upcast<device::IThruster>();
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
                if (strcmp(optarg, "setspeed") == 0)
                {
                    setspeed_flag = true;
                }
                arg_counter++;
            }
            else if (arg_counter == 2 && setspeed_flag)
            {
                target_speed = std::atoi(optarg);
                speed_received_ = true;
                arg_counter++;
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
    else if (!speed_received_ && setspeed_flag)
    {
        cout << "No speed value specified";
        g_is_running = false;
        return_code_ = -1;
    }
}

inline int ThrusterShell::Run()
{
    if (!g_is_running)
    {
        return return_code_;
    }
    if (setspeed_flag)
    {
        thruster_device->SetSpeed(target_speed);
    }
    else
    {
        int speed_value = thruster_device->GetSpeed();
        cout << "Thruster speed: " << speed_value;
    }
    return return_code_;
}

inline int ThrusterCommands(int argc, char *argv[])
{
    ThrusterShell handler(argc, argv);
    return handler.Run();
}
} // namespace hydrolib::shell
