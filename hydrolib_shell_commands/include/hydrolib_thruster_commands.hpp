#pragma once

#include <unistd.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "hydrolib_device.hpp"
#include "hydrolib_device_manager.hpp"
#include "hydrolib_shell.hpp"
#include "hydrolib_thruster_device.hpp"

namespace hydrolib::shell {

class ThrusterShell {
 public:
  ThrusterShell(int argc, char *argv[]);

 public:
  int Run();
  void ParseSetSpeed(int argc, char *argv[]);
  void ParseGetSpeed(int argc, char *argv[]);

 private:
  enum class CommandType { None, SetSpeed, GetSpeed };

  device::IThruster *thruster_device;
  int target_speed;
  bool speed_received_;
  CommandType command_type_;
  int return_code_;
};

int ThrusterCommands(int argc, char *argv[]);

inline ThrusterShell::ThrusterShell(int argc, char *argv[])
    : thruster_device(nullptr),
      target_speed(0),
      speed_received_(false),
      command_type_(CommandType::None),
      return_code_(0) {
  optind = 0;
  int opt = getopt(argc, argv, "-:h");
  while (opt != -1) {
    switch (opt) {
      case 'h':
        cout << "Usage: thr {setsp -d [DEVICE_NAME] -s [SPEED] | getsp "
                "[DEVICE_NAME]}";
        g_is_running = false;
        return;
      case 1:
        if (strcmp(optarg, "setsp") == 0) {
          command_type_ = CommandType::SetSpeed;
          ParseSetSpeed(argc, argv);
          return;
        } else if (strcmp(optarg, "getsp") == 0) {
          command_type_ = CommandType::GetSpeed;
          ParseGetSpeed(argc, argv);
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

inline void ThrusterShell::ParseSetSpeed(int argc, char *argv[]) {
  device::Device *finded_device = nullptr;
  int opt = getopt(argc, argv, ":d:s:h");
  while (opt != -1) {
    switch (opt) {
      case 'h':
        cout << "Usage: thr setsp -d [DEVICE_NAME] -s [SPEED]";
        g_is_running = false;
        return;
      case 'd':
        if (thruster_device == nullptr) {
          finded_device = (*device::g_device_manager)[optarg];
          if (finded_device == nullptr) {
            cout << "Device not found: " << optarg;
            g_is_running = false;
            return_code_ = -1;
            return;
            break;
          }
          thruster_device = finded_device->Upcast<device::IThruster>();
          if (thruster_device == nullptr) {
            cout << "Device is not a thruster: " << optarg;
            g_is_running = false;
            return_code_ = -1;
            return;
            break;
          }
        } else {
          cout << "Too many arguments";
          g_is_running = false;
          return_code_ = -1;
          return;
        }
        break;
      case 's': {
        if (!speed_received_) {
          char *endptr;
          int speed = static_cast<int>(strtol(optarg, &endptr, 10));
          if (!speed && optarg[0] != '0') {
            cout << "Invalid speed: " << optarg;
            g_is_running = false;
            return_code_ = -1;
            return;
          }
          target_speed = speed;
          speed_received_ = true;
          break;
        }
        break;
      }
      default:
        cout << "Invalid option: " << static_cast<char>(optopt);
        g_is_running = false;
        return_code_ = -1;
        return;
    }
    opt = getopt(argc, argv, ":d:s:h");
  }
  if (thruster_device == nullptr) {
    cout << "No thruster device specified";
    g_is_running = false;
    return_code_ = -1;
  } else if (!speed_received_) {
    cout << "No speed value specified";
    g_is_running = false;
    return_code_ = -1;
  }
}

inline void ThrusterShell::ParseGetSpeed(int argc, char *argv[]) {
  device::Device *finded_device = nullptr;
  int opt = getopt(argc, argv, "-:h");
  while (opt != -1) {
    switch (opt) {
      case 'h':
        cout << "Usage: thr getsp [DEVICE_NAME]";
        g_is_running = false;
        return;
      case 1:
        if (thruster_device == nullptr) {
          finded_device = (*device::g_device_manager)[optarg];
          if (finded_device == nullptr) {
            cout << "Device not found: " << optarg;
            g_is_running = false;
            return_code_ = -1;
            return;
            break;
          }
          thruster_device = finded_device->Upcast<device::IThruster>();
          if (thruster_device == nullptr) {
            cout << "Device is not a thruster: " << optarg;
            g_is_running = false;
            return_code_ = -1;
            return;
            break;
          }
        } else {
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
  if (thruster_device == nullptr && g_is_running) {
    cout << "No thruster device specified";
    g_is_running = false;
    return_code_ = -1;
  }
}

inline int ThrusterShell::Run() {
  if (!g_is_running) {
    return return_code_;
  }
  switch (command_type_) {
    case CommandType::SetSpeed: {
      thruster_device->SetSpeed(target_speed);
      char set_speed_buffer[10];
      snprintf(set_speed_buffer, sizeof(set_speed_buffer), "%d", target_speed);
      cout << "Thruster set speed: " << set_speed_buffer;
      break;
    }
    case CommandType::GetSpeed: {
      int speed_value = thruster_device->GetSpeed();
      char get_speed_buffer[10];
      snprintf(get_speed_buffer, sizeof(get_speed_buffer), "%d", speed_value);
      cout << "Thruster speed: " << get_speed_buffer;
      break;
    }
    case CommandType::None:
      break;
  }
  return return_code_;
}

inline int ThrusterCommands(int argc, char *argv[]) {
  ThrusterShell handler(argc, argv);
  return handler.Run();
}
}  // namespace hydrolib::shell
