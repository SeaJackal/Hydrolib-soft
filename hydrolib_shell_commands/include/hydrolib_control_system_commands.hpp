#pragma once

#include <unistd.h>

#include <array>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "hydrolib_control_system_device.hpp"
#include "hydrolib_device.hpp"
#include "hydrolib_device_manager.hpp"
#include "hydrolib_fixed_point.hpp"
#include "hydrolib_shell.hpp"
#include "hydrolib_thruster_device.hpp"

namespace hydrolib::shell {
class ControlSystemShell {
 public:
  ControlSystemShell(int argc, char *argv[]);

 public:
  int Run();

 private:
  enum class CommandType {
    None,
    SetMultipliers,
  };

  device::IControlSystem *control_system_device;
  controlling::Control<math::FixedPointBase> control_;
  CommandType command_type_;
  int return_code_;
};

int ControlSystemCommands(int argc, char *argv[]);

inline ControlSystemShell::ControlSystemShell(int argc, char *argv[])
    : control_system_device(nullptr),
      control_(),
      command_type_(CommandType::None),
      return_code_(0) {
  device::Device *finded_device = nullptr;
  optind = 0;
  int opt = getopt(argc, argv, "-:x:y:z:X:Y:Z:h");
  while (opt != -1) {
    switch (opt) {
      case 'h':
        cout << "Usage: ctrl <device_name> [-x <x_force>] [-y <y_force>] "
                "[-z <z_force>] [-X <x_torque>] [-Y <y_torque>] [-Z "
                "<z_torque>]\r\n";
        cout << "Usage: ctrl <device_name> stop - for all 0 values";
        g_is_running = false;
        return;
      case 1:
        if (control_system_device == nullptr) {
          finded_device = (*device::g_device_manager)[optarg];
          if (finded_device == nullptr) {
            cout << "Device not found: " << optarg;
            g_is_running = false;
            return_code_ = -1;
            return;
            break;
          }
          control_system_device =
              finded_device->Upcast<device::IControlSystem>();
          if (control_system_device == nullptr) {
            cout << "Device is not a system control: " << optarg;
            g_is_running = false;
            return_code_ = -1;
            return;
            break;
          }
        } else if (strcmp(optarg, "stop") == 0) {
          control_.x_force = math::FixedPointBase(0);
          control_.y_force = math::FixedPointBase(0);
          control_.z_force = math::FixedPointBase(0);
          control_.x_torque = math::FixedPointBase(0);
          control_.y_torque = math::FixedPointBase(0);
          control_.z_torque = math::FixedPointBase(0);
          control_system_device->ControlProcess(control_);
          return;
          break;
        } else {
          cout << "Invalid command: " << optarg;
          g_is_running = false;
          return_code_ = -1;
          return;
        }
        break;
      case 'x': {
        char *endptr;
        int value = static_cast<int>(strtol(optarg, &endptr, 10));
        if (!value && optarg[0] != '0') {
          cout << "Invalid force value: " << optarg;
          g_is_running = false;
          return_code_ = -1;
          return;
        }
        control_.x_force = math::FixedPointBase(value);
        break;
      }
      case 'y': {
        char *endptr;
        int value = static_cast<int>(strtol(optarg, &endptr, 10));
        if (!value && optarg[0] != '0') {
          cout << "Invalid force value: " << optarg;
          g_is_running = false;
          return_code_ = -1;
          return;
        }
        control_.y_force = math::FixedPointBase(value);
        break;
      }
      case 'z': {
        char *endptr;
        int value = static_cast<int>(strtol(optarg, &endptr, 10));
        if (!value && optarg[0] != '0') {
          cout << "Invalid force value: " << optarg;
          g_is_running = false;
          return_code_ = -1;
          return;
        }
        control_.z_force = math::FixedPointBase(value);
        break;
      }
      case 'X': {
        char *endptr;
        int value = static_cast<int>(strtol(optarg, &endptr, 10));
        if (!value && optarg[0] != '0') {
          cout << "Invalid torque value: " << optarg;
          g_is_running = false;
          return_code_ = -1;
          return;
        }
        control_.x_torque = math::FixedPointBase(value);
        break;
      }
      case 'Y': {
        char *endptr;
        int value = static_cast<int>(strtol(optarg, &endptr, 10));
        if (!value && optarg[0] != '0') {
          cout << "Invalid torque value: " << optarg;
          g_is_running = false;
          return_code_ = -1;
          return;
        }
        control_.y_torque = math::FixedPointBase(value);
        break;
      }
      case 'Z': {
        char *endptr;
        int value = static_cast<int>(strtol(optarg, &endptr, 10));
        if (!value && optarg[0] != '0') {
          cout << "Invalid torque value: " << optarg;
          g_is_running = false;
          return_code_ = -1;
          return;
        }
        control_.z_torque = math::FixedPointBase(value);
        break;
      }

      default:
        cout << "Invalid option: " << static_cast<char>(optopt);
        g_is_running = false;
        return_code_ = -1;
        return;
    }
    opt = getopt(argc, argv, "-:x:y:z:X:Y:Z:h");
  }
  if (control_system_device != nullptr && g_is_running) {
    command_type_ = CommandType::SetMultipliers;
  } else if (control_system_device == nullptr && g_is_running) {
    cout << "No thruster generator device specified";
    g_is_running = false;
    return_code_ = -1;
  }
}

inline int ControlSystemShell::Run() {
  if (!g_is_running) {
    return return_code_;
  }
  switch (command_type_) {
    case CommandType::SetMultipliers: {
      control_system_device->ControlProcess(control_);
      break;
    }
    case CommandType::None:
      break;
  }
  return return_code_;
}

inline int ControlSystemCommands(int argc, char *argv[]) {
  ControlSystemShell handler(argc, argv);
  return handler.Run();
}
}  // namespace hydrolib::shell
