#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <string_view>

#include "hydrolib_shell_viewer.hpp"
#include "mock_stream.hpp"
#include "pty_line_device.hpp"

namespace {
using hydrolib::shell::Viewer;
using hydrolib::shell::tests::GenerateRandomString;
using hydrolib::shell::tests::PtyLineDevice;
using hydrolib::shell::tests::RandomSeed;

class TerminalViewTest : public ::testing::TestWithParam<std::string> {
 protected:
  std::string DrainStream() {
    stream_.MakeAllbytesAvailable();
    const auto size = stream_.GetSize();
    if (size == 0) {
      return {};
    }

    std::string out;
    out.resize(size);
    const auto read_bytes = read(stream_, out.data(), out.size());
    if (read_bytes <= 0) {
      return {};
    }
    out.resize(static_cast<std::size_t>(read_bytes));
    return out;
  }

  void ProcessNextSymbol(std::string_view seq_prefix) {
    const auto pty = dev_.Process(seq_prefix);

    EXPECT_EQ(view_.Process(pty.to_program), hydrolib::ReturnCode::OK);
    view_accum_.append(DrainStream());
    EXPECT_EQ(view_accum_, pty.to_echo);
  }

 private:
  PtyLineDevice dev_;
  hydrolib::streams::mock::MockByteStream stream_;
  Viewer<hydrolib::streams::mock::MockByteStream> view_{stream_};
  std::string view_accum_;
};

INSTANTIATE_TEST_SUITE_P(
    CanonicalEchoMatchesView, TerminalViewTest,
    ::testing::Values("abc", "a\bb", "ab\bc", "\b\b\babc", "bcd\b\b\b", "\br",
                      GenerateRandomString(RandomSeed{0}, 10),
                      GenerateRandomString(RandomSeed{1}, 20),
                      GenerateRandomString(
                          RandomSeed{2}, hydrolib::shell::kMaxCommandLength)));

TEST_P(TerminalViewTest, EchoMatches) {
  const auto& seq = GetParam();

  for (std::size_t i = 0; i < seq.size(); ++i) {
    ProcessNextSymbol(seq.substr(0, i + 1));
  }
}

}  // namespace
