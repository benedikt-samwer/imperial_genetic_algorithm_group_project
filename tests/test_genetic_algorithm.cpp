// tests/test_genetic_algorithm.cpp
 
#include "Genetic_Algorithm.h"
#include <cmath>
#include <iostream>
#include <vector>
 
// Known “answer” vectors for testing
static const int int_test_answer[21] = {
    0, 1, 2, 0, 3, 1, 4, 2, 0, 1, 3,
    4, 2, 1, 0, 3, 2, 4, 1, 3, 0
};
static const double real_test_answer[] = {0.8, 0.4, 0.4, 0.8, 0.0, 0.8};
// Discrete‐only fitness: we want to minimize sum of squared differences,
// so we return its negative (GA maximizes fitness).
double test_function(int L, int *v) {
  double sumsq = 0;
  for (int i = 0; i < L; ++i) {
    double diff = v[i] - int_test_answer[i];
    sumsq += diff * diff;
  }
  return -sumsq;
}
 
// Continuous‐only fitness, same idea
double real_test_function(int L, double *v) {
  double sumsq = 0;
  for (int i = 0; i < L; ++i) {
    double diff = v[i] - real_test_answer[i];
    sumsq += diff * diff;
  }
  return -sumsq;
}
 
// All‐pass validity
bool always_valid_int(int L, int *v) { return true; }
bool always_valid_real(int L, double *v) { return true; }
 
int main() {
  // --- Discrete test ---
  std::vector<int> vector1 = {
      1, 0, 3, 2, 1, 0, 4, 3, 2, 1, 0,
      3, 4, 2, 1, 0, 3, 2, 1, 4, 0
  };
  int L1 = vector1.size();
 
  Algorithm_Parameters params;
  params.verbose = true;
  params.max_iterations = 200;
  params.population_size = 500;
  params.tournament_size = 2;
  params.crossover_probability = 0.8;
  params.mutation_probability = 0.05;
  params.mutation_step_size = 2;
  params.convergence_threshold = 1e-6;
  params.stall_generations = 50;
 
  std::cout << "=== Discrete GA Test ===\n";
  int code =
      optimize(L1, vector1.data(), test_function, always_valid_int, params);
  if (code != 0) {
    std::cerr << "ERROR: optimize() returned code " << code << "\n";
    return code;
  }
 
  // Print out what the GA found
  auto stats1 = get_last_optimization_result();
  std::cout << "Final discrete genome: ";
  for (int x : vector1)
    std::cout << x << ' ';
  std::cout << "\nBest fitness: " << stats1.best_fitness
            << " (generations: " << stats1.generations << ")\n";
 
  // Verify result
  bool ok1 = true;
  for (int i = 0; i < L1; ++i) {
    if (vector1[i] != int_test_answer[i]) {
      std::cerr << "Mismatch at index " << i << ": got " << vector1[i]
                << " expected " << int_test_answer[i] << "\n";
      ok1 = false;
    }
  }
  if (!ok1) {
    std::cerr << "Discrete GA test FAILED\n";
    return 1;
  }
  std::cout << "Discrete GA test PASSED\n\n";
 
  // --- Continuous test ---
  std::vector<double> vector2 = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5};
  int L2 = vector2.size();
 
  std::cout << "=== Continuous GA Test ===\n";
  code = optimize(L2, vector2.data(), real_test_function, always_valid_real,
                  params);
  if (code != 0) {
    std::cerr << "ERROR: optimize() (real) returned code " << code << "\n";
    return code;
  }
 
  // Print out what the GA found
  auto stats2 = get_last_optimization_result();
  std::cout << "Final real genome: ";
  for (double x : vector2)
    std::cout << x << ' ';
  std::cout << "\nBest fitness: " << stats2.best_fitness
            << " (generations: " << stats2.generations << ")\n";
 
  // Verify result (with a small epsilon)
  bool ok2 = true;
  const double eps = 1e-2;
  for (int i = 0; i < L2; ++i) {
    if (std::abs(vector2[i] - real_test_answer[i]) > eps) {
      std::cerr << "Mismatch (real) at index " << i << ": got " << vector2[i]
                << " expected " << real_test_answer[i] << "\n";
      ok2 = false;
    }
  }
  if (!ok2) {
    std::cerr << "Continuous GA test FAILED\n";
    return 1;
  }
  std::cout << "Continuous GA test PASSED\n";

  // --- Mixed discrete+continuous test ---
  std::cout << "\n=== Mixed GA Test ===\n";

  std::vector<int> vector3 = {
      1, 0, 3, 2, 1, 0, 4, 3, 2, 1, 0,
      3, 4, 2, 1, 0, 3, 2, 1, 4, 0
  };
  std::vector<double> vector4 = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5};
  int L3 = vector3.size();
  int L4 = vector4.size();

  auto mixed_test_function = [](int Li, int* iv, int Lr, double* rv) -> double {
    double int_error = 0.0, real_error = 0.0;
    for (int i = 0; i < Li; ++i) {
      double diff = iv[i] - int_test_answer[i];
      int_error += diff * diff;
    }
    for (int i = 0; i < Lr; ++i) {
      double diff = rv[i] - real_test_answer[i];
      real_error += diff * diff;
    }
    return -(int_error + real_error);
  };

  auto always_valid_mixed = [](int, int*, int, double*) { return true; };

  code = optimize(L3, vector3.data(), L4, vector4.data(),
                      mixed_test_function, always_valid_mixed, params);
  if (code != 0) {
    std::cerr << "ERROR: optimize() (mixed) returned code " << code << "\n";
    return code;
  }

  auto stats3 = get_last_optimization_result();
  std::cout << "Final mixed int vector: ";
  for (int x : vector3) std::cout << x << ' ';
  std::cout << "\nFinal mixed real vector: ";
  for (double x : vector4) std::cout << x << ' ';
  std::cout << "\nBest fitness: " << stats3.best_fitness
            << " (generations: " << stats3.generations << ")\n";

  // === Separate validation ===
  bool ok3_int = true;
  bool ok3_real = true;

  std::cout << "\nValidating integer genome:\n";
  for (int i = 0; i < L3; ++i) {
    if (vector3[i] != int_test_answer[i]) {
      std::cerr << "Mismatch (int) at index " << i
                << ": got " << vector3[i]
                << ", expected " << int_test_answer[i] << "\n";
      ok3_int = false;
    } else {
      std::cout << "Match at index " << i << ": " << vector3[i] << "\n";
    }

  const double float_eps = 1e-2;
  std::cout << "\nValidating real genome:\n";
  for (int i = 0; i < L4; ++i) {
    if (std::abs(vector4[i] - real_test_answer[i]) > float_eps) {
      std::cerr << "Mismatch (real) at index " << i
                << ": got " << vector4[i]
                << ", expected " << real_test_answer[i] << "\n";
      ok3_real = false;
    }
  }

  if (!ok3_int && !ok3_real) {
    std::cerr << "Mixed GA test FAILED: both int and real components incorrect.\n";
    return 1;
  }
  if (!ok3_int) {
    std::cerr << "Mixed GA test FAILED: int genome mismatch.\n";
    return 1;
  }
  if (!ok3_real) {
    std::cerr << "Mixed GA test PASSED (int correct, real approximate).\n";
  } else {
    std::cout << "Mixed GA test PASSED (both int and real correct).\n";
  }
  return 0;
}
}