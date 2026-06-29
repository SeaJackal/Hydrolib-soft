#pragma once

#include <string_view>

#include "hydrolib_func_concepts.hpp"
#include "hydrolib_return_codes.hpp"
#include "hydrolib_shell_environment.hpp"
#include "hydrolib_shell_input.hpp"
#include "hydrolib_shell_ostream.hpp"
#include "hydrolib_shell_parser.hpp"
#include "hydrolib_shell_viewer.hpp"

namespace hydrolib::shell {
template <typename T>
concept CommandMapConcept = requires(T map, std::string_view key) {
  { map.find(key) == map.end() } -> std::same_as<bool>;
  {
    map[key]
  } -> concepts::func::FuncConcept<int, Environment&,
                                   std::span<const std::string_view>>;
};

template <concepts::stream::ByteFullStreamConcept Stream, CommandMapConcept Map>
class Shell {
 public:
  constexpr Shell(Stream& stream, Map& handlers);
  Shell(const Shell&) = delete;
  Shell(Shell&&) = delete;
  Shell& operator=(const Shell&) = delete;
  Shell& operator=(Shell&&) = delete;
  ~Shell() = default;

  hydrolib::ReturnCode Process();

 private:
  Stream& stream_;
  Input<Stream> input_;
  Viewer<Stream> viewer_;
  FlowController flow_controller_;
  StreamWrapper<Stream> cout_wrapper_;
  Map& handlers_;
  Environment env_ = {.flow_control = FlowController::Viewer(flow_controller_),
                      .cout = Ostream(cout_wrapper_)};

  int last_error_code_ = 0;
};

template <concepts::stream::ByteFullStreamConcept Stream, CommandMapConcept Map>
constexpr Shell<Stream, Map>::Shell(Stream& stream, Map& handlers)
    : stream_(stream),
      input_(stream),
      viewer_(stream),
      cout_wrapper_(stream),
      handlers_(handlers) {}

template <concepts::stream::ByteFullStreamConcept Stream, CommandMapConcept Map>
hydrolib::ReturnCode Shell<Stream, Map>::Process() {
  auto input_result = input_.Process();
  if (input_result == hydrolib::ReturnCode::ERROR) {
    return hydrolib::ReturnCode::ERROR;
  }
  if (viewer_.Process(input_result) == hydrolib::ReturnCode::ERROR) {
    return hydrolib::ReturnCode::ERROR;
  }

  if (input_result == hydrolib::ReturnCode::NO_DATA) {
    return hydrolib::ReturnCode::NO_DATA;
  }

  if (viewer_.Enter() == hydrolib::ReturnCode::ERROR) {
    return hydrolib::ReturnCode::ERROR;
  }

  auto command_info = parser::CommandInfo(parser::SplitToArgs(input_result));

  if (handlers_.find(command_info.GetCommand()) == handlers_.end()) {
    env_.cout << "Unknown command: " << command_info.GetCommand() << "\n";
    if (viewer_.Welcome() == hydrolib::ReturnCode::ERROR) {
      return hydrolib::ReturnCode::ERROR;
    }
    return hydrolib::ReturnCode::FAIL;
  }

  last_error_code_ =
      handlers_[command_info.GetCommand()](env_, command_info.GetArgs());

  if (viewer_.Welcome() == hydrolib::ReturnCode::ERROR) {
    return hydrolib::ReturnCode::ERROR;
  }
  return hydrolib::ReturnCode::OK;
}

}  // namespace hydrolib::shell
