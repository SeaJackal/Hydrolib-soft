#include <gtest/gtest.h>

#include <array>
#include <tuple>

#include "hydrolib_linear_equations.hpp"

using hydrolib::math::SolveLinear;

constexpr int kLinearSize = 3;
constexpr int kLinearSize6 = 6;

using LinearParam =
    std::tuple<std::array<std::array<double, kLinearSize>, kLinearSize>,
               std::array<double, kLinearSize>>;
using LinearParam6 =
    std::tuple<std::array<std::array<double, kLinearSize6>, kLinearSize6>,
               std::array<double, kLinearSize6>>;

static constexpr std::array<LinearParam, 4> kLinearSolveCases{{
    LinearParam{std::array<std::array<double, kLinearSize>, kLinearSize>{{
                    std::array<double, kLinearSize>{2.0, 1.0, -1.0},
                    std::array<double, kLinearSize>{-3.0, -1.0, 2.0},
                    std::array<double, kLinearSize>{-2.0, 1.0, 2.0},
                }},
                std::array<double, kLinearSize>{2.0, 3.0, -1.0}},
    LinearParam{std::array<std::array<double, kLinearSize>, kLinearSize>{{
                    std::array<double, kLinearSize>{4.0, 2.0, 0.0},
                    std::array<double, kLinearSize>{2.0, 4.0, 2.0},
                    std::array<double, kLinearSize>{0.0, 2.0, 4.0},
                }},
                std::array<double, kLinearSize>{1.0, 2.0, 3.0}},
    LinearParam{std::array<std::array<double, kLinearSize>, kLinearSize>{{
                    std::array<double, kLinearSize>{1.5, -0.5, 2.0},
                    std::array<double, kLinearSize>{0.5, 3.0, -1.0},
                    std::array<double, kLinearSize>{2.0, -1.0, 1.0},
                }},
                std::array<double, kLinearSize>{1.2, -0.4, 0.5}},
    LinearParam{std::array<std::array<double, kLinearSize>, kLinearSize>{{
                    std::array<double, kLinearSize>{3.0, 1.0, 1.0},
                    std::array<double, kLinearSize>{1.0, 2.0, 3.0},
                    std::array<double, kLinearSize>{2.0, 1.0, 2.0},
                }},
                std::array<double, kLinearSize>{2.0, -1.0, 1.0}},
}};

static constexpr std::array<std::array<double, kLinearSize6>, kLinearSize6>
    kLinearSolveCoefficients6{{
        std::array<double, kLinearSize6>{+0.8192, +0.0000, +0.8192, +0.8192,
                                         +0.0000, +0.8192},
        std::array<double, kLinearSize6>{+0.0000, +1.0000, +0.0000, +0.0000,
                                         +1.0000, +0.0000},
        std::array<double, kLinearSize6>{-0.1806, -0.0520, +0.1806, +0.1806,
                                         -0.0520, -0.1806},
        std::array<double, kLinearSize6>{+0.0983, +0.0000, +0.0983, -0.0983,
                                         +0.0000, -0.0983},
        std::array<double, kLinearSize6>{-0.1265, -0.1230, -0.1265, +0.1265,
                                         +0.1230, +0.1265},
        std::array<double, kLinearSize6>{-0.5736, -0.0000, +0.5736, -0.5736,
                                         +0.0000, +0.5736},
    }};

static constexpr std::array kLinearSolveCases6{
    LinearParam6{
        kLinearSolveCoefficients6,
        std::array<double, kLinearSize6>{1.0, -0.5, 0.25, -0.75, 0.5, -1.0}},
    LinearParam6{
        kLinearSolveCoefficients6,
        std::array<double, kLinearSize6>{0.1, 0.2, 0.3, 0.4, 0.5, 0.6}},
    LinearParam6{kLinearSolveCoefficients6,
                 std::array<double, kLinearSize6>{1, 0, 0, 0, 0, 0}},
    LinearParam6{kLinearSolveCoefficients6,
                 std::array<double, kLinearSize6>{0, 1, 0, 0, 0, 0}},
    LinearParam6{kLinearSolveCoefficients6,
                 std::array<double, kLinearSize6>{0, 0, 1, 0, 0, 0}}};

template <int N>
void RunLinearSolveCase(
    const std::array<std::array<double, N>, N>& coefficients,
    const std::array<double, N>& free_terms) {
  std::array<double, N> results{};
  SolveLinear<double, N>(coefficients, free_terms, results);

  for (int i = 0; i < N; i++) {
    double expected = 0.0;
    for (int j = 0; j < N; j++) {
      expected += coefficients[i][j] * results[j];
    }
    EXPECT_DOUBLE_EQ(free_terms[i], expected);
  }
}

class TestLinearEquationsSolve
    : public ::testing::Test,
      public ::testing::WithParamInterface<LinearParam> {};

INSTANTIATE_TEST_CASE_P(Test, TestLinearEquationsSolve,
                        ::testing::ValuesIn(kLinearSolveCases));

class TestLinearEquationsSolve6
    : public ::testing::Test,
      public ::testing::WithParamInterface<LinearParam6> {};

INSTANTIATE_TEST_CASE_P(Test, TestLinearEquationsSolve6,
                        ::testing::ValuesIn(kLinearSolveCases6));

TEST_P(TestLinearEquationsSolve, Basic) {
  auto values = GetParam();
  auto coefficients = std::get<0>(values);
  auto expected = std::get<1>(values);
  RunLinearSolveCase<kLinearSize>(coefficients, expected);
}

TEST_P(TestLinearEquationsSolve6, Basic) {
  auto values = GetParam();
  auto coefficients = std::get<0>(values);
  auto expected = std::get<1>(values);
  RunLinearSolveCase<kLinearSize6>(coefficients, expected);
}
