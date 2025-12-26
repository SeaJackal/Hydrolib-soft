#pragma once

#include <unistd.h>

#include "hydrolib_device_manager.hpp"
#include "hydrolib_shell.hpp"
#include "hydrolib_stream_device.hpp"

namespace hydrolib::shell {
class CatCommand {
 public:
  CatCommand(int argc, char *argv[]);

  int Run();

 private:
  device::IStreamDevice *device_{};

  bool hex_mode_ = false;
  int return_code_ = 0;
};

int Cat(int argc, char *argv[]);

inline CatCommand::CatCommand(int argc, char *argv[]) {
  const char *format_string = "-:hx";
  int opt = getopt(argc, argv, format_string);
  while (opt != -1) {
    device::Device *finded_device = nullptr;
    switch (opt) {
      case 'h':
        cout << "Usage: cat [DEVICE_NAME] [-x]\n";
        cout << "x - hex";
        g_is_running = false;
        return;
      case 'x':
        hex_mode_ = true;
        return;
      case 1:
        finded_device = (*device::g_device_manager)[optarg];
        if (finded_device == nullptr) {
          cout << "Device not found: " << optarg;
          g_is_running = false;
          return_code_ = -1;
          return;
        }
        device_ = finded_device->Upcast<device::IStreamDevice>();
        if (device_ == nullptr) {
          cout << "Device is not a stream device: " << optarg;
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
    opt = getopt(argc, argv, format_string);
  }
  if (device_ == nullptr) {
    cout << "No device specified";
    g_is_running = false;
    return_code_ = -1;
  }
}

inline int CatCommand::Run() {
  while (g_is_running) {
    char buffer = 0;
    int result = device_->Read(&buffer, 1);
    if (result < 0 || result > 1) {
      return -2;
    }
    if (result == 1) {
      if (hex_mode_) {
        int param = static_cast<int>(buffer);
        if (param == 0) {
          cout << '0';
        } else {
          if (param < 0) {
            cout << '-';
            param = -param;
          }

          unsigned digit = 1;
          while (digit <= static_cast<unsigned>(param)) {
            digit *= 10;
          }
          digit /= 10;

          char symbol = 0;

          while (digit != 0) {
            symbol = param / digit + '0';
            param %= digit;
            cout << symbol;
            digit /= 10;
          }
        }
        cout << ' ';
      } else {
        cout << buffer;
      }
    }
  }
  return return_code_;
}

inline int Cat(int argc, char *argv[]) {
  CatCommand handler(argc, argv);
  return handler.Run();
}
}  // namespace hydrolib::shell
