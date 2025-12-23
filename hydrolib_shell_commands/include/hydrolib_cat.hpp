#pragma once

#include <iostream>
#include <unistd.h>

#include "hydrolib_device_manager.hpp"
#include "hydrolib_shell.hpp"
#include "hydrolib_shell_interpreter.hpp"
#include "hydrolib_stream_device.hpp"

namespace hydrolib::shell
{
class CatCommand
{
public:
    CatCommand(int argc, char *argv[]);

public:
    int Run();

private:
    device::IStreamDevice *device_;

    int return_code_;
};

int Cat(int argc, char *argv[]);

inline CatCommand::CatCommand(int argc, char *argv[])
    : device_(nullptr), return_code_(0)
{
    int opt;
    while ((opt = getopt(argc, argv, "-:h")) != -1)
    {
        device::Device *finded_device = nullptr;
        switch (opt)
        {
        case 'h':
            std::cout << "Usage: cat [DEVICE_NAME]";
            SetRunningFlase(); 
            break;
        case 1:
            finded_device = (*device::g_device_manager)[optarg];
            if (finded_device == nullptr)
            {
                std::cout << "Device not found: " << optarg;
                SetRunningFalse();
                return_code_ = -1;
                break;
            }
            device_ = finded_device->Upcast<device::IStreamDevice>();
            if (device_ == nullptr)
            {
                std::cout << "Device is not a stream device: " << optarg;
                SetRunningFalse();
                return_code_ = -1;
                break;
            }
            break;
        default:
            std::cout << "Invalid option: " << static_cast<char>(optopt);
            SetRunningFalse(); 
            return_code_ = -1;
            break;
        }
    }
}

inline int CatCommand::Run()
{

    //найдено ли устройство
    if(device_ == nullptr)
    {
        std::cout << "No device specified" << std::endl;
        return -1;
    }

    while (ExecutionManager::isRunning())
    {
        char buffer;
        int result = device_->Read(&buffer, 1);
        if (result < 0 || result > 1)
        {
            return -2;
        }
        else if (result == 1)
        {
            std::cout << buffer;
        }
    }
    return return_code_;
}

inline int Cat(int argc, char *argv[])
{
    CatCommand handler(argc, argv);
    return handler.Run();
}
} // namespace hydrolib::shell
