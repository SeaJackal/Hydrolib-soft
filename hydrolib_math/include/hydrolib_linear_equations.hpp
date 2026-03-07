#pragma once

#include "hydrolib_fixed_point.hpp"

namespace hydrolib::math {

template <ArithmeticConcept Number, int N>
constexpr void SolveLinear(std::array<std::array<Number, N>, N> coefficients,
                           std::array<Number, N> free_terms,
                           std::array<Number, N>& results);

template <ArithmeticConcept Number, int N>
constexpr void SolveLinear(std::array<std::array<Number, N>, N> coefficients,
                           std::array<Number, N> free_terms,
                           std::array<Number, N>& results) {
  if (coefficients[0][0] == 0) [[unlikely]] {
    for (int i = 1; i < N; i++) {
      if (coefficients[i][0] != 0) {
        for (int j = 0; j < N; j++) {
          coefficients[0][j] += coefficients[i][j];
        }
        break;
      }
    }
  }
  for (int i = 0; i < N - 1; i++) {
    if (coefficients[i][i] == 0) [[unlikely]] {
      for (int j = i + 1; j < N; j++) {
        coefficients[i][j] += coefficients[i - 1][j];
      }
      free_terms[i] += free_terms[i - 1];
    } else {
      for (int j = i + 1; j < N; j++) {
        coefficients[i][j] /= coefficients[i][i];
      }
      free_terms[i] /= coefficients[i][i];
    }
    for (int k = i + 1; k < N; k++) {
      for (int j = i + 1; j < N; j++) {
        coefficients[k][j] -= coefficients[k][i] * coefficients[i][j];
      }
      free_terms[k] -= free_terms[i] * coefficients[k][i];
    }
  }
  free_terms[N - 1] /= coefficients[N - 1][N - 1];
  for (int i = N - 2; i >= 0; i--) {
    for (int j = i + 1; j < N; j++) {
      free_terms[i] -= free_terms[j] * coefficients[i][j];
    }
  }
  results = free_terms;
}
}  // namespace hydrolib::math
