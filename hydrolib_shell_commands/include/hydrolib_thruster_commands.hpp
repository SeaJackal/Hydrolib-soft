#pragma once

#include <cstddef>
#include <cstdio>
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
    void ParseSetSpeed(int argc, char *argv[]);
    void ParseGetSpeed(int argc, char *argv[]);

private:
    device::IThruster *thruster_device;
    int target_speed;
    bool speed_received_;
    bool setspeed_flag;
    bool getspeed_flag;
    bool negative_speed;
    int return_code_;
};

int ThrusterCommands(int argc, char *argv[]);

inline ThrusterShell::ThrusterShell(int argc, char *argv[])
    : thruster_device(nullptr),
      target_speed(0),
      speed_received_(false),
      setspeed_flag(false),
      getspeed_flag(false),
      negative_speed(false),
      return_code_(0)
{
    int opt = getopt(argc, argv, "-:h");
    while (opt != -1)
    {
        switch (opt)
        {
        case 'h':
            cout << "Usage: thr {setsp [DEVICE_NAME] [SPEED] | getsp "
                    "[DEVICE_NAME]}";
            g_is_running = false;
            return;
        case 1:
            if (strcmp(optarg, "setsp") == 0)
            {
                setspeed_flag = true;
                ParseSetSpeed(argc, argv);
                return;
            }
            else if (strcmp(optarg, "getsp") == 0)
            {
                getspeed_flag = true;
                ParseGetSpeed(argc, argv);
                return;
            }
            else
            {
                cout << "Invalid command: " << optarg;
                g_is_running = false;
                return_code_ = -1;
                return;
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
}

inline void ThrusterShell::ParseSetSpeed(int argc, char *argv[])
{
    device::Device *finded_device = nullptr;
    int opt = getopt(argc, argv, "-:hn");
    while (opt != -1)
    {
        switch (opt)
        {
        case 'h':
            cout << "Usage: thr setsp [DEVICE_NAME] [SPEED]\r\n";
            cout << "Usage for negative speed value: thr setsp "
                    "[DEVICE_NAME] -n [SPEED]";
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
                    break;
                }
                thruster_device = finded_device->Upcast<device::IThruster>();
                if (thruster_device == nullptr)
                {
                    cout << "Device is not a thruster: " << optarg;
                    g_is_running = false;
                    return_code_ = -1;
                    return;
                    break;
                }
            }
            else if (!speed_received_)
            {
                char *endptr;
                long int speed = std::strtol(optarg, &endptr, 10);
                if (endptr == optarg || *endptr != '\0')
                {
                    cout << "Invalid speed: " << optarg;
                    g_is_running = false;
                    return_code_ = -1;
                    return;
                }
                target_speed = static_cast<int>(speed);
                speed_received_ = true;
            }
            else
            {
                cout << "Too many arguments";
                g_is_running = false;
                return_code_ = -1;
                return;
            }
            break;
        case 'n':
            if (setspeed_flag)
            {
                negative_speed = true;
            }
            break;
        default:
            cout << "Invalid option: " << static_cast<char>(optopt);
            g_is_running = false;
            return_code_ = -1;
            return;
        }
        opt = getopt(argc, argv, "-:hn");
    }
    if (thruster_device == nullptr && g_is_running)
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

inline void ThrusterShell::ParseGetSpeed(int argc, char *argv[])
{
    device::Device *finded_device = nullptr;
    int opt = getopt(argc, argv, "-:h");
    while (opt != -1)
    {
        switch (opt)
        {
        case 'h':
            cout << "Usage: thr getsp [DEVICE_NAME]";
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
                    break;
                }
                thruster_device = finded_device->Upcast<device::IThruster>();
                if (thruster_device == nullptr)
                {
                    cout << "Device is not a thruster: " << optarg;
                    g_is_running = false;
                    return_code_ = -1;
                    return;
                    break;
                }
            }
            else
            {
                cout << "Too many arguments";
                g_is_running = false;
                return_code_ = -1;
                return;
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
    if (thruster_device == nullptr && g_is_running)
    {
        cout << "No thruster device specified";
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
        if (negative_speed)
        {
            thruster_device->SetSpeed(target_speed * (-1));
        }
        else
            thruster_device->SetSpeed(target_speed);
        char set_speed_buffer[6];
        snprintf(set_speed_buffer, sizeof(set_speed_buffer), "%d",
                 target_speed);
        if (negative_speed)
        {
            cout << "Thruster set speed: -" << set_speed_buffer;
        }
        else
        {
            cout << "Thruster set speed: " << set_speed_buffer;
        }
    }
    else if (getspeed_flag)
    {
        int speed_value = thruster_device->GetSpeed();
        char get_speed_buffer[6];
        snprintf(get_speed_buffer, sizeof(get_speed_buffer), "%d", speed_value);
        cout << "Thruster speed: " << get_speed_buffer;
    }
    return return_code_;
}

inline int ThrusterCommands(int argc, char *argv[])
{
    ThrusterShell handler(argc, argv);
    return handler.Run();
}
} // namespace hydrolib::shell
