#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>

#include <array>
#include <functional>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#include "hydrolib_shell_parser.hpp"

namespace {
class Parsing
    : public ::testing::Test,
      public ::testing::WithParamInterface<
          std::tuple<std::vector<std::string>,
                     std::function<std::string(std::vector<std::string>&)>>> {};

std::string UniteSimply(std::vector<std::string>& args) {
  std::string result;
  for (std::size_t i = 0; i < args.size() - 1; ++i) {
    result += args[i];
    result += " ";
  }
  result += args[args.size() - 1];
  return result;
}

std::string UniteRandomly(std::vector<std::string>& args, int seed) {
  std::mt19937 gen(seed);
  std::uniform_int_distribution<int> dist(1, 10);
  std::string result;
  for (int j = 0, number = dist(gen); j != number; ++j) {
    result += " ";
  }
  for (const auto& arg : args) {
    result += arg;
    for (int j = 0, number = dist(gen); j != number; ++j) {
      result += " ";
    }
  }
  return result;
}

const std::array test_cases = {
    std::vector<std::string>{"cat"}, std::vector<std::string>{"echo", "asdf"},
    std::vector<std::string>{"process", "-x", "20", "-v"}};
}  // namespace

INSTANTIATE_TEST_CASE_P(
    Test, Parsing,
    ::testing::Combine(::testing::ValuesIn(test_cases),
                       ::testing::Values(
                           UniteSimply,
                           [](std::vector<std::string>& args) {
                             return UniteRandomly(args, 0);
                           },
                           [](std::vector<std::string>& args) {
                             return UniteRandomly(args, 5);
                           },
                           [](std::vector<std::string>& args) {
                             return UniteRandomly(args, 20);
                           })));

TEST_P(Parsing, SimpleTest) {
  const auto& params = GetParam();
  auto data = std::get<0>(params);
  auto former = std::get<1>(params);
  auto str = former(data);
  auto view = hydrolib::shell::parser::SplitToArgs(str);
  auto iter = data.begin();
  for (auto arg : view) {
    ASSERT_NE(iter, data.end());
    EXPECT_EQ(*iter, arg);
    ++iter;
  }
}
