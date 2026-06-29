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
  for (int i = 0; i < N - 1; i++) {
    if (coefficients[i][i] == 0) [[unlikely]] {
      bool found = false;
      for (int j = i + 1; j < N; j++) {
        if (coefficients[j][i] != 0) {
          for (int k = i; k < N; k++) {
            coefficients[i][k] += coefficients[j][k];
          }
          free_terms[i] += free_terms[j];
          found = true;
          break;
        }
      }
      if (!found) {
        return;
      }
    }
    for (int j = i + 1; j < N; j++) {
      coefficients[i][j] /= coefficients[i][i];
    }
    free_terms[i] /= coefficients[i][i];

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
