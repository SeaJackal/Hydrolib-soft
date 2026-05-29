#include <gtest/gtest.h>

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <span>
#include <unordered_map>
#include <vector>

#include "hydrolib_return_codes.hpp"
#include "hydrolib_shell.hpp"
#include "hydrolib_shell_environment.hpp"
#include "mock_stream.hpp"

namespace {

using hydrolib::ReturnCode;
using hydrolib::shell::Environment;

using HandlerFn =
    std::function<int(Environment&, std::span<const std::string_view>)>;

using Handlers = std::unordered_map<std::string_view, HandlerFn>;

using Stream = hydrolib::streams::mock::MockByteStream;
using ShellUnderTest = hydrolib::shell::Shell<Stream, Handlers>;

std::string DrainAll(Stream& stream) {
  stream.MakeAllbytesAvailable();
  const auto size = stream.GetSize();
  if (size == 0) {
    return {};
  }
  std::string out;
  out.resize(size);
  const auto read_bytes = read(stream, out.data(), out.size());
  if (read_bytes <= 0) {
    return {};
  }
  out.resize(static_cast<std::size_t>(read_bytes));
  return out;
}

ReturnCode RunLine(ShellUnderTest& shell, Stream& stream,
                   std::string_view line) {
  write(stream, line.data(), line.size());
  stream.AddAvailableBytes(static_cast<int>(line.size()));

  ReturnCode result_code = ReturnCode::NO_DATA;
  for (std::size_t i = 0; i < line.size(); ++i) {
    result_code = shell.Process();
    if (result_code != ReturnCode::NO_DATA) {
      return result_code;
    }
  }
  return result_code;
}

}  // namespace

TEST(HydrolibShell, DispatchesCommandWithoutArguments) {
  Stream stream;
  Handlers handlers;

  int called = 0;
  std::vector<std::string> received;

  handlers.emplace(
      "ping",
      [&](Environment&, std::span<const std::string_view> args) -> int {
        called++;
        received.clear();
        for (auto arg : args) {
          received.emplace_back(arg);
        }
        return 0;
      });

  ShellUnderTest shell(stream, handlers);

  EXPECT_EQ(RunLine(shell, stream, "ping\n"), ReturnCode::OK);
  EXPECT_EQ(called, 1);
  EXPECT_EQ(received, (std::vector<std::string>{"ping"}));
  EXPECT_EQ(DrainAll(stream), "ping\n\rhydrosh > ");
}

TEST(HydrolibShell, DispatchesCommandWithArguments) {
  Stream stream;
  Handlers handlers;

  int called = 0;
  std::vector<std::string> received;

  handlers.emplace(
      "set", [&](Environment&, std::span<const std::string_view> args) -> int {
        called++;
        received.clear();
        for (auto arg : args) {
          received.emplace_back(arg);
        }
        return 0;
      });

  ShellUnderTest shell(stream, handlers);

  EXPECT_EQ(RunLine(shell, stream, "set value1 42\n"), ReturnCode::OK);
  EXPECT_EQ(called, 1);
  EXPECT_EQ(received, (std::vector<std::string>{"set", "value1", "42"}));
  EXPECT_EQ(DrainAll(stream), "set value1 42\n\rhydrosh > ");
}

TEST(HydrolibShell, ReportsUnknownCommand) {
  Stream stream;
  Handlers handlers;
  ShellUnderTest shell(stream, handlers);

  EXPECT_EQ(RunLine(shell, stream, "nope\n"), ReturnCode::FAIL);

  EXPECT_EQ(DrainAll(stream), "nope\n\rUnknown command: nope\nhydrosh > ");
}
