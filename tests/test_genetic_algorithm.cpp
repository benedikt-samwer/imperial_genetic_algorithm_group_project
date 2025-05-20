// tests/test_genetic_algorithm.cpp
 
#include "Genetic_Algorithm.h"
#include <cmath>
#include <iostream>
#include <vector>
 
// Known “answer” vectors for testing
static const int int_test_answer[] = {2, 1, 1, 2, 0, 2, 3, 0, 4, 4, 1};
static const double real_test_answer[] = {2.0, 1.0, 1.0, 2.0, 0.0, 2.0};
 
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
  std::vector<int> vector1 = {0, 1, 1, 2, 2, 3, 3, 0, 1, 0, 4};
  int L1 = vector1.size();
 
  Algorithm_Parameters params;
  params.verbose = true;
  params.max_iterations = 100;
  params.population_size = 100;
  params.tournament_size = 2;
  params.crossover_probability = 0.8;
  params.mutation_probability = 0.2;
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
  std::vector<double> vector2 = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0};
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
  const double eps = 1e-6;
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
 
  return 0;
}
 