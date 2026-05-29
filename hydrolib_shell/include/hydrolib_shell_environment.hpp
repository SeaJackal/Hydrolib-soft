#pragma once

#include "hydrolib_shell_ostream.hpp"

namespace hydrolib::shell {

class FlowController {
 public:
  class Viewer;

  void Launch();

 private:
  bool is_running_ = false;
};

class FlowController::Viewer {
 public:
  explicit Viewer(FlowController& flow_controller);

  [[nodiscard]] bool IsRunning() const;
  void Terminate();

 private:
  FlowController* flow_controller_ = nullptr;
};

struct Environment {
  FlowController::Viewer flow_control;
  Ostream cout;
};

inline void FlowController::Launch() { is_running_ = false; }

inline FlowController::Viewer::Viewer(FlowController& flow_controller)
    : flow_controller_(&flow_controller) {};

inline bool FlowController::Viewer::IsRunning() const {
  return flow_controller_->is_running_;
}

inline void FlowController::Viewer::Terminate() {
  flow_controller_->is_running_ = false;
}

};  // namespace hydrolib::shell