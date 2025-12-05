#pragma once

#include <cstring>
#include <unistd.h>

#include "hydrolib_device_manager.hpp"
#include "hydrolib_shell.hpp"
#include "hydrolib_stream_device.hpp"

namespace hydrolib::shell
{
class EchoCommand
{
public:
    EchoCommand(int argc, char *argv[]);

public:
    int Run();

private:
    device::IStreamDevice *device_;
    const char *message_;

    int return_code_;
};

int Echo(int argc, char *argv[]);

inline EchoCommand::EchoCommand(int argc, char *argv[])
    : device_(nullptr), message_(nullptr), return_code_(0)
{
    int opt = getopt(argc, argv, "-:h");
    while (opt != -1)
    {
        device::Device *finded_device = nullptr;
        switch (opt)
        {
        case 'h':
            cout << "Usage: echo [DEVICE_NAME] [STRING]";
            g_is_running = false;
            return;
        case 1:
            if (device_ == nullptr)
            {
                finded_device = (*device::g_device_manager)[optarg];
                if (finded_device == nullptr)
                {
                    cout << "Device not found: " << optarg;
                    g_is_running = false;
                    return_code_ = -1;
                    return;
                }
                device_ = finded_device->Upcast<device::IStreamDevice>();
                if (device_ == nullptr)
                {
                    cout << "Device is not a stream device: " << optarg;
                    g_is_running = false;
                    return_code_ = -1;
                    return;
                }
            }
            else
            {
                if (message_ == nullptr)
                {
                    message_ = optarg;
                }
                else
                {
                    cout << "Too many arguments";
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
    if (device_ == nullptr)
    {
        cout << "No device specified";
        g_is_running = false;
        return_code_ = -1;
        return;
    }
    if (message_ == nullptr || *message_ == '\0')
    {
        cout << "No data specified";
        g_is_running = false;
        return_code_ = -1;
        return;
    }
}

inline int EchoCommand::Run()
{
    if (!g_is_running)
    {
        return return_code_;
    }
    const int expected = static_cast<int>(std::strlen(message_));
    int written = device_->Write(message_, expected);
    if (written != expected || written < 0)
    {
        return -2;
    }
    return return_code_;
}

inline int Echo(int argc, char *argv[])
{
    EchoCommand handler(argc, argv);
    return handler.Run();
}
} // namespace hydrolib::shell


