#pragma once

#include <unistd.h>

#include <cstddef>
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
  void ParseMultiplyPull(int argc, char *argv[]);
  void ParseStop();
  inline const char *NumToDeviceName(int num);

 private:
  enum class CommandType { None, SetSpeed, GetSpeed, MultiplyPull, Stop };

  device::IThruster *thruster_device;
  int target_speed;
  bool speed_received_;
  bool negative_speed;
  int thruster_num;
  int thrust_percent;
  CommandType command_type_;
  int return_code_;
};

int ThrusterCommands(int argc, char *argv[]);

inline ThrusterShell::ThrusterShell(int argc, char *argv[])
    : thruster_device(nullptr),
      target_speed(0),
      speed_received_(false),
      negative_speed(false),
      thruster_num(0),
      thrust_percent(0),
      command_type_(CommandType::None),
      return_code_(0) {
  optind = 0;
  int opt = getopt(argc, argv, "-:h");
  while (opt != -1) {
    switch (opt) {
      case 'h':
        cout << "Usage: thr setsp [DEVICE_NAME] [SPEED] - to set thrusters "
                "speed\r\n";
        cout << "Usage: thr getsp [DEVICE_NAME] - to get thrusters speed\r\n";
        cout << "Usage: thr multi -t [THRUSTER_NUM] [PERCENT] - to set "
                "thrusters pull multipliers\r\n";
        cout << "Usage: thr stop - to stop all thrusters";
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
        } else if (strcmp(optarg, "multi") == 0) {
          command_type_ = CommandType::MultiplyPull;
          ParseMultiplyPull(argc, argv);
          return;
        } else if (strcmp(optarg, "stop") == 0) {
          command_type_ = CommandType::Stop;
          ParseStop();
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

inline void ThrusterShell::ParseMultiplyPull(int argc, char *argv[]) {
  device::Device *finded_device = nullptr;
  const char *device = nullptr;
  int opt = getopt(argc, argv, "-:t:h");
  while (opt != -1) {
    switch (opt) {
      case 'h':
        cout << "Usage: thr multi -t [THRUSTER_NUM] [PERCENT] - to set "
                "thrusters pull multipliers\r\n";
        cout << "For many engines usage: thr multi -t [THRUSTER_NUM] [PERCENT] "
                "-t [...] [...] ...\r\n";
        cout << "Example: thr multi -t 1 50 -t 2 75 -t 3 30";
        g_is_running = false;
        return;
      case 't':
        if (thruster_device == nullptr) {
          thruster_num = std::atoi(optarg);
          device = NumToDeviceName(thruster_num);
          finded_device = (*device::g_device_manager)[device];
          if (finded_device == nullptr) {
            cout << "Device not found: " << device;
            g_is_running = false;
            return_code_ = -1;
            return;
            break;
          }
          thruster_device = finded_device->Upcast<device::IThruster>();
          if (thruster_device == nullptr) {
            cout << "Device is not a thruster: " << device;
            g_is_running = false;
            return_code_ = -1;
            return;
            break;
          }
        }
        if (optind < argc && thruster_device != nullptr) {
          int thrust_percent = std::atoi(argv[optind]);
          optind++;

          int speed = (thrust_percent * 1000) / 100;
          thruster_device->SetSpeed(speed);

          char percent_buffer[4];
          snprintf(percent_buffer, sizeof(percent_buffer), "%d",
                   thrust_percent);
          cout << device << " set to " << percent_buffer << "%";

          thruster_device = nullptr;
        }
        break;
      default:
        cout << "Invalid option: " << static_cast<char>(optopt);
        g_is_running = false;
        return_code_ = -1;
        return;
    }
    opt = getopt(argc, argv, "-:t:h");
  }
}

inline void ThrusterShell::ParseStop() {
  for (int i = 0; i < 6; i++) {
    const char *device = NumToDeviceName(i);
    device::Device *finded_device = (*device::g_device_manager)[device];
    if (finded_device == nullptr) {
      continue;
    }
    thruster_device = finded_device->Upcast<device::IThruster>();
    if (thruster_device == nullptr) {
      continue;
    }
    thruster_device->SetSpeed(0);
  }
  g_is_running = true;
  return_code_ = 0;
  cout << "All thrusters are stopped";
}

inline void ThrusterShell::ParseSetSpeed(int argc, char *argv[]) {
  device::Device *finded_device = nullptr;
  int opt = getopt(argc, argv, "-:hn");
  while (opt != -1) {
    switch (opt) {
      case 'h':
        cout << "Usage: thr setsp [DEVICE_NAME] [SPEED]\r\n";
        cout << "Usage for negative speed value: thr setsp "
                "[DEVICE_NAME] -n [SPEED]";
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
        } else if (!speed_received_) {
          int speed = std::atoi(optarg);
          if (optarg[0] == '0') {
            speed = 0;
          } else if (!speed) {
            cout << "Invalid speed: " << optarg;
            g_is_running = false;
            return_code_ = -1;
            return;
          }
          target_speed = static_cast<int>(speed);
          speed_received_ = true;
        } else {
          cout << "Too many arguments";
          g_is_running = false;
          return_code_ = -1;
          return;
        }
        break;
      case 'n':
        negative_speed = true;
        break;
      default:
        cout << "Invalid option: " << static_cast<char>(optopt);
        g_is_running = false;
        return_code_ = -1;
        return;
    }
    opt = getopt(argc, argv, "-:hn");
  }
  if (thruster_device == nullptr && g_is_running) {
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
      if (negative_speed) {
        thruster_device->SetSpeed(target_speed * (-1));
      } else
        thruster_device->SetSpeed(target_speed);
      char set_speed_buffer[6];
      snprintf(set_speed_buffer, sizeof(set_speed_buffer), "%d", target_speed);
      if (negative_speed) {
        cout << "Thruster set speed: -" << set_speed_buffer;
      } else {
        cout << "Thruster set speed: " << set_speed_buffer;
      }
      break;
    }
    case CommandType::GetSpeed: {
      int speed_value = thruster_device->GetSpeed();
      char get_speed_buffer[6];
      snprintf(get_speed_buffer, sizeof(get_speed_buffer), "%d", speed_value);
      cout << "Thruster speed: " << get_speed_buffer;
      break;
    }
    case CommandType::MultiplyPull: {
      return_code_ = 0;
      break;
    }
    case CommandType::None:
      break;
  }
  return return_code_;
}

inline const char *ThrusterShell::NumToDeviceName(int num) {
  switch (num) {
    case 0:
      return "thr0";
    case 1:
      return "thr1";
    case 2:
      return "thr2";
    case 3:
      return "thr3";
    case 4:
      return "thr4";
    case 5:
      return "thr5";
    default:
      return "unknown";
  }
}

inline int ThrusterCommands(int argc, char *argv[]) {
  ThrusterShell handler(argc, argv);
  return handler.Run();
}
}  // namespace hydrolib::shell
