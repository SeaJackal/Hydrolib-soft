#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <string_view>

#include "hydrolib_shell_input.hpp"
#include "hydrolib_shell_params.hpp"
#include "mock_stream.hpp"
#include "pty_line_device.hpp"

namespace {
using hydrolib::shell::tests::GenerateRandomString;
using hydrolib::shell::tests::PtyLineDevice;
using hydrolib::shell::tests::RandomSeed;

class TerminalInputTest : public ::testing::TestWithParam<std::string> {
 protected:
  PtyLineDevice& GetDev() { return dev_; }
  hydrolib::streams::mock::MockByteStream& GetStream() { return stream_; }
  hydrolib::shell::Input<hydrolib::streams::mock::MockByteStream>& GetInput() {
    return input_;
  }

  void ProcessNextSymbol(std::string_view seq,
                         hydrolib::ReturnCode expected_result) {
    GetStream().AddAvailableBytes(1);
    auto step = GetInput().Process();
    EXPECT_EQ(static_cast<hydrolib::ReturnCode>(step), expected_result);
    const auto pty = GetDev().Process(seq);
    EXPECT_EQ(pty.to_program, static_cast<std::string_view>(step));
  }

 private:
  PtyLineDevice dev_;
  hydrolib::streams::mock::MockByteStream stream_;
  hydrolib::shell::Input<hydrolib::streams::mock::MockByteStream> input_{
      stream_};
};

INSTANTIATE_TEST_SUITE_P(
    CanonicalEraseMatchesInput, TerminalInputTest,
    ::testing::Values("abc", "a\bb", "ab\bc", "\b\b\babc", "bcd\b\b\b", "\br",
                      GenerateRandomString(RandomSeed{0}, 10),
                      GenerateRandomString(RandomSeed{1}, 20),
                      GenerateRandomString(
                          RandomSeed{2}, hydrolib::shell::kMaxCommandLength)));

TEST_P(TerminalInputTest, ParsedLineMatches) {
  const auto& seq = GetParam();

  write(GetStream(), seq.data(), seq.size());

  for (std::size_t i = 0; i < seq.size(); ++i) {
    ProcessNextSymbol(seq.substr(0, i + 1), hydrolib::ReturnCode::NO_DATA);
  }

  constexpr std::string_view kEnter = "\n";
  write(GetStream(), kEnter.data(), 1);
  ProcessNextSymbol(seq, hydrolib::ReturnCode::OK);
}

}  // namespace
