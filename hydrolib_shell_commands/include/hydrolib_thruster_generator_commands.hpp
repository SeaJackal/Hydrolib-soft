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
struct Control {
  math::FixedPointBase x_force;
  math::FixedPointBase y_force;
  math::FixedPointBase z_force;

  math::FixedPointBase x_torque;
  math::FixedPointBase y_torque;
  math::FixedPointBase z_torque;
};
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

  device::IThrustGenerator *thruster_generator_device;
  Control control_;
  struct ControlOutput {
    char x_force[16];
    char y_force[16];
    char z_force[16];
    char x_torque[16];
    char y_torque[16];
    char z_torque[16];
  } control_output;
  std::array<hydrolib::math::FixedPointBase, THRUST_COUNT> thrusts{};
  CommandType command_type_;
  int return_code_;
};

int ThrusterGeneratorCommands(int argc, char *argv[]);

inline ThrusterGeneratorShell::ThrusterGeneratorShell(int argc, char *argv[])
    : thruster_generator_device(nullptr),
      control_(),
      control_output(),
      command_type_(CommandType::None),
      return_code_(0) {
  control_.x_force = math::FixedPointBase(0);
  control_.y_force = math::FixedPointBase(0);
  control_.z_force = math::FixedPointBase(0);
  control_.x_torque = math::FixedPointBase(0);
  control_.y_torque = math::FixedPointBase(0);
  control_.z_torque = math::FixedPointBase(0);

  snprintf(control_output.x_force, sizeof(control_output.x_force), "%d", 0);
  snprintf(control_output.y_force, sizeof(control_output.y_force), "%d", 0);
  snprintf(control_output.z_force, sizeof(control_output.z_force), "%d", 0);
  snprintf(control_output.x_torque, sizeof(control_output.x_torque), "%d", 0);
  snprintf(control_output.y_torque, sizeof(control_output.y_torque), "%d", 0);
  snprintf(control_output.z_torque, sizeof(control_output.z_torque), "%d", 0);

  device::Device *finded_device = nullptr;
  optind = 0;
  int opt = getopt(argc, argv, "-:x:y:z:X:Y:Z:h");
  while (opt != -1) {
    switch (opt) {
      case 'h':
        cout << "Usage: thrgen <device_name> [-x <x_force>] [-y <y_force>] "
                "[-z <z_force>] [-Z <x_torque>] [-Y <y_torque>] [-Z "
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
              finded_device->Upcast<device::IThrustGenerator>();
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
        snprintf(control_output.x_force, sizeof(control_output.x_force), "%d",
                 value);
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
        snprintf(control_output.y_force, sizeof(control_output.y_force), "%d",
                 value);
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
        snprintf(control_output.z_force, sizeof(control_output.z_force), "%d",
                 value);
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
        snprintf(control_output.x_torque, sizeof(control_output.x_torque), "%d",
                 value);
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
        snprintf(control_output.y_torque, sizeof(control_output.y_torque), "%d",
                 value);
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
        snprintf(control_output.z_torque, sizeof(control_output.z_torque), "%d",
                 value);
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
      thruster_generator_device->ProcessWithFeedback(&control_, &thrusts);
      cout << "Controls: "
           << "Fx = " << control_output.x_force << "; "
           << "Fy = " << control_output.y_force << "; "
           << "Fz = " << control_output.z_force << " | "
           << "Mx = " << control_output.x_torque << "; "
           << "My = " << control_output.y_torque << "; "
           << "Mz = " << control_output.z_torque;
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
