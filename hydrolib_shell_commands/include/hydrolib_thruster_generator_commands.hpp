#pragma once

#include <unistd.h>

#include <array>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "hydrolib_device.hpp"
#include "hydrolib_device_manager.hpp"
#include "hydrolib_fixed_point.hpp"
#include "hydrolib_shell.hpp"
#include "hydrolib_thrust_generator_device.hpp"
#include "hydrolib_thruster_device.hpp"

#define THRUST_COUNT 6

namespace hydrolib::shell {
class ThrusterGeneratorShell {
 public:
  ThrusterGeneratorShell(int argc, char *argv[]);

 public:
  int Run();

 private:
  enum class CommandType {
    None,
    SetMultipliers,
  };

  device::IControlSystem *thruster_generator_device;
  controlling::Control control_;
  CommandType command_type_;
  int return_code_;
};

int ThrusterGeneratorCommands(int argc, char *argv[]);

inline ThrusterGeneratorShell::ThrusterGeneratorShell(int argc, char *argv[])
    : thruster_generator_device(nullptr),
      control_(),
      command_type_(CommandType::None),
      return_code_(0) {
  control_.x_force = math::FixedPointBase(0);
  control_.y_force = math::FixedPointBase(0);
  control_.z_force = math::FixedPointBase(0);
  control_.x_torque = math::FixedPointBase(0);
  control_.y_torque = math::FixedPointBase(0);
  control_.z_torque = math::FixedPointBase(0);

  device::Device *finded_device = nullptr;
  optind = 0;
  int opt = getopt(argc, argv, "-:x:y:z:X:Y:Z:h");
  while (opt != -1) {
    switch (opt) {
      case 'h':
        cout << "Usage: thrgen <device_name> [-x <x_force>] [-y <y_force>] "
                "[-z <z_force>] [-X <x_torque>] [-Y <y_torque>] [-Z "
                "<z_torque>]";
        g_is_running = false;
        return;
      case 1:
        if (thruster_generator_device == nullptr) {
          finded_device = (*device::g_device_manager)[optarg];
          if (finded_device == nullptr) {
            cout << "Device not found: " << optarg;
            g_is_running = false;
            return_code_ = -1;
            return;
            break;
          }
          thruster_generator_device =
              finded_device->Upcast<device::IControlSystem>();
          if (thruster_generator_device == nullptr) {
            cout << "Device is not a thruster generator: " << optarg;
            g_is_running = false;
            return_code_ = -1;
            return;
            break;
          }
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
  if (thruster_generator_device != nullptr && g_is_running) {
    command_type_ = CommandType::SetMultipliers;
  } else if (thruster_generator_device == nullptr && g_is_running) {
    cout << "No thruster generator device specified";
    g_is_running = false;
    return_code_ = -1;
  }
}

inline int ThrusterGeneratorShell::Run() {
  if (!g_is_running) {
    return return_code_;
  }
  switch (command_type_) {
    case CommandType::SetMultipliers: {
      thruster_generator_device->ControlProccess(control_);
      break;
    }
    case CommandType::None:
      break;
  }
  return return_code_;
}

inline int ThrusterGeneratorCommands(int argc, char *argv[]) {
  ThrusterGeneratorShell handler(argc, argv);
  return handler.Run();
}
}  // namespace hydrolib::shell
