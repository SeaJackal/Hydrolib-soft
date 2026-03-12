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

// hydrosh> bfs setf 10.5 2.3 5.0
// hydrosh> bfs sett 1.2 0.8 0.5
// hydrosh> bfs generate

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
  std::array<hydrolib::math::FixedPointBase, THRUST_COUNT> thrusts{};
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

  int opt = getopt(argc, argv, "-:h");
  while (opt != -1) {
    switch (opt) {
      case 'h':
        cout << "Usage: bfs {setsp [DEVICE_NAME] [SPEED] | getsp "
                "[DEVICE_NAME]}";
        g_is_running = false;
        return;
      case 1:
        if (strcmp(optarg, "setsp") == 0) {
          command_type_ = CommandType::SetMultipliers;
          return;
        } else {
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

inline int ThrusterGeneratorShell::Run() {
  if (!g_is_running) {
    return return_code_;
  }
  switch (command_type_) {
    case CommandType::SetMultipliers: {
      thruster_generator_device->ProcessWithFeedback(&control_, &thrusts);
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
