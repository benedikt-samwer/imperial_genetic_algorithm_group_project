// tests/test_genetic_algorithm.cpp

#include "Genetic_Algorithm.h"
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>

// Known "answer" vectors for testing
static const int int_test_answer_orig[21] = {
    0, 1, 2, 0, 3, 1, 4, 2, 0, 1, 3,
    4, 2, 1, 0, 3, 2, 4, 1, 3, 0
};
static const double real_test_answer[] = {0.8, 0.4, 0.4, 0.8, 0.0, 0.8};

// Answer vector for the first test file
static const int int_test_answer[] = {2, 1, 1, 2, 0, 2, 3, 0, 4, 4, 1};

// Additional test vectors with different characteristics
static const int int_test_answer_long[] = {5, 3, 0, 2, 8, 1, 7, 4, 9, 2, 5, 3, 6, 8, 1, 9, 0, 7, 5, 4};

// Set up standardized parameters for all tests - completely identical for all tests
Algorithm_Parameters setup_standard_parameters() {
    Algorithm_Parameters params;
    params.verbose = true;
    params.population_size = 200;
    params.mutation_probability = 0.05;
    params.crossover_probability = 0.85;
    params.selection_pressure = 1.6;
    params.stall_generations = 100;
    params.elite_count = 2;
    params.max_iterations = 400;  // Same maximum iterations for all tests
    params.tournament_size = 2;
    params.mutation_step_size = 2;
    params.convergence_threshold = 1e-6;
    
    return params;
}

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

// Original test function from the second file
double test_function_orig(int L, int *v) {
  double sumsq = 0;
  for (int i = 0; i < L; ++i) {
    double diff = v[i] - int_test_answer_orig[i];
    sumsq += diff * diff;
  }
  return -sumsq;
}

// Long vector test function
double long_test_function(int L, int *v) {
  double sumsq = 0;
  int max_idx = std::min(L, static_cast<int>(sizeof(int_test_answer_long)/sizeof(int)));
  for (int i = 0; i < max_idx; ++i) {
    double diff = v[i] - int_test_answer_long[i];
    sumsq += diff * diff;
  }
  return -sumsq;
}

// Fitness function with multiple local optima (discrete)
double multimodal_discrete_function(int L, int *v) {
  // Base fitness is negative sum of squared differences
  double base_fitness = test_function(L, v);
  
  // Add "bumps" to create local optima
  double bumps = 0.0;
  for (int i = 0; i < L-1; i++) {
    // Create a local optimum when adjacent values are equal
    if (v[i] == v[i+1]) {
      bumps += 10.0;
    }
  }
  
  return base_fitness + bumps;
}

// Deceptive fitness landscape (discrete)
double deceptive_function(int L, int *v) {
  int hamming_distance = 0;
  for (int i = 0; i < L; i++) {
    if (v[i] != int_test_answer[i]) {
      hamming_distance++;
    }
  }
  
  // If we're very close to the answer, give a high reward
  if (hamming_distance == 0) return 1000.0;
  
  // If we're getting closer, the fitness actually gets worse
  // This is the deceptive part - creates a "valley" before the peak
  if (hamming_distance < 3) return -50.0 * (3 - hamming_distance);
  
  // Otherwise, give a small reward for being somewhat close
  return 10.0 * (L - hamming_distance);
}

// Function with multiple optima of equal value (discrete)
double multi_optimum_function(int L, int *v) {
  // Two different optimal solutions
  const int solution1[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  const int solution2[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
  
  double dist1 = 0, dist2 = 0;
  for (int i = 0; i < std::min(L, 10); i++) {
    double diff1 = v[i] - solution1[i];
    double diff2 = v[i] - solution2[i];
    dist1 += diff1 * diff1;
    dist2 += diff2 * diff2;
  }
  
  // Return the negative of the minimum distance (closer to either solution is better)
  return -std::min(dist1, dist2);
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

// Test for long discrete vector
void test_long_vector(const Algorithm_Parameters& params) {
  std::cout << "=== Long Vector Discrete GA Test ===\n";
  
  std::vector<int> vector(20, 0);  // Initialize with 20 zeros
  int L = vector.size();
  
  int code = optimize(L, vector.data(), long_test_function, always_valid_int, params);
  if (code != 0) {
    std::cerr << "ERROR: long vector optimize() returned code " << code << "\n";
    return;
  }
  
  auto stats = get_last_optimization_result();
  std::cout << "Final long vector genome: ";
  for (int x : vector)
    std::cout << x << ' ';
  std::cout << "\nBest fitness: " << stats.best_fitness
            << " (generations: " << stats.generations << ")\n";
  
  // Check if we found values close to optimal
  bool ok = true;
  for (int i = 0; i < L && i < static_cast<int>(sizeof(int_test_answer_long)/sizeof(int)); ++i) {
    if (vector[i] != int_test_answer_long[i]) {
      std::cerr << "Mismatch at index " << i << ": got " << vector[i]
                << " expected " << int_test_answer_long[i] << "\n";
      ok = false;
    }
  }
  
  if (ok) {
    std::cout << "Long vector test PASSED\n\n";
  } else {
    std::cout << "Long vector test FAILED\n\n";
  }
}

// Test for multimodal fitness landscape
void test_multimodal(const Algorithm_Parameters& params) {
  std::cout << "=== Multimodal Discrete GA Test ===\n";
  
  std::vector<int> vector = {0, 1, 1, 2, 2, 3, 3, 0, 1, 0, 4};
  int L = vector.size();
  
  int code = optimize(L, vector.data(), multimodal_discrete_function, always_valid_int, params);
  if (code != 0) {
    std::cerr << "ERROR: multimodal optimize() returned code " << code << "\n";
    return;
  }
  
  auto stats = get_last_optimization_result();
  std::cout << "Final multimodal genome: ";
  for (int x : vector)
    std::cout << x << ' ';
  std::cout << "\nBest fitness: " << stats.best_fitness
            << " (generations: " << stats.generations << ")\n";
  
  // Calculate reference fitness using the known answer
  std::vector<int> reference(int_test_answer, int_test_answer + L);
  double reference_fitness = multimodal_discrete_function(L, reference.data());
  
  std::cout << "Reference fitness: " << reference_fitness << "\n";
  std::cout << "Final fitness: " << multimodal_discrete_function(L, vector.data()) << "\n";
  
  // Success criteria: either exactly match the answer or achieve a fitness within 90% of optimal
  bool exact_match = true;
  for (int i = 0; i < L; ++i) {
    if (vector[i] != int_test_answer[i]) {
      exact_match = false;
      break;
    }
  }
  
  double fitness_ratio = multimodal_discrete_function(L, vector.data()) / reference_fitness;
  
  if (exact_match || fitness_ratio >= 0.9) {
    std::cout << "Multimodal test PASSED" << (exact_match ? " (exact match)" : " (close enough)") << "\n\n";
  } else {
    std::cout << "Multimodal test FAILED (fitness not close enough to optimal)\n\n";
  }
}

// Test for deceptive fitness landscape
void test_deceptive(const Algorithm_Parameters& params) {
  std::cout << "=== Deceptive Fitness Test ===\n";
  
  std::vector<int> vector = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}; // Start away from solution
  int L = vector.size();
  
  int code = optimize(L, vector.data(), deceptive_function, always_valid_int, params);
  if (code != 0) {
    std::cerr << "ERROR: deceptive optimize() returned code " << code << "\n";
    return;
  }
  
  auto stats = get_last_optimization_result();
  std::cout << "Final deceptive solution: ";
  for (int x : vector)
    std::cout << x << ' ';
  std::cout << "\nBest fitness: " << stats.best_fitness
            << " (generations: " << stats.generations << ")\n";
  
  // For deceptive function, success is finding the exact answer
  bool exact_match = true;
  for (int i = 0; i < L; ++i) {
    if (vector[i] != int_test_answer[i]) {
      exact_match = false;
      break;
    }
  }
  
  if (exact_match) {
    std::cout << "Deceptive test PASSED (found global optimum despite deception)\n\n";
  } else {
    std::cout << "Deceptive test FAILED (trapped in local optimum)\n\n";
  }
}

// Test for multiple global optima
void test_multi_optimum(const Algorithm_Parameters& params) {
  std::cout << "=== Multiple Optima Test ===\n";
  
  std::vector<int> vector = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5}; // Start in middle
  int L = vector.size();
  
  int code = optimize(L, vector.data(), multi_optimum_function, always_valid_int, params);
  if (code != 0) {
    std::cerr << "ERROR: multi-optimum optimize() returned code " << code << "\n";
    return;
  }
  
  auto stats = get_last_optimization_result();
  std::cout << "Final multi-optimum solution: ";
  for (int x : vector)
    std::cout << x << ' ';
  std::cout << "\nBest fitness: " << stats.best_fitness
            << " (generations: " << stats.generations << ")\n";
  
  // Check if we converged to either of the two optima
  bool is_solution1 = true, is_solution2 = true;
  for (int i = 0; i < L; ++i) {
    if (vector[i] != 1) is_solution1 = false;
    if (vector[i] != 9) is_solution2 = false;
  }
  
  if (is_solution1 || is_solution2) {
    std::cout << "Multiple Optima test PASSED (found " 
              << (is_solution1 ? "solution 1" : "solution 2") << ")\n\n";
  } else {
    std::cout << "Multiple Optima test FAILED (didn't find either optimum)\n\n";
  }
}

int main() {
  // Create a single set of standard parameters for all tests
  Algorithm_Parameters standard_params = setup_standard_parameters();
  
  // Print the parameters being used for all tests
  std::cout << "=== Running All Tests with These Parameters ===\n"
            << "- Population size: " << standard_params.population_size << "\n"
            << "- Mutation rate: " << standard_params.mutation_probability << "\n"
            << "- Crossover rate: " << standard_params.crossover_probability << "\n"
            << "- Selection pressure: " << standard_params.selection_pressure << "\n"
            << "- Stall generations: " << standard_params.stall_generations << "\n" 
            << "- Elite count: " << standard_params.elite_count << "\n"
            << "- Max iterations: " << standard_params.max_iterations << "\n"
            << "- Tournament size: " << standard_params.tournament_size << "\n"
            << "- Mutation step size: " << standard_params.mutation_step_size << "\n\n";
 
  // --- Basic Discrete test ---
  std::vector<int> vector1 = {0, 1, 1, 2, 2, 3, 3, 0, 1, 0, 4};
  int L1 = vector1.size();
            
  std::cout << "=== Basic Discrete GA Test ===\n";
  int code = optimize(L1, vector1.data(), test_function, always_valid_int, standard_params);
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
    std::cerr << "Basic Discrete GA test FAILED\n";
    return 1;
  }
  std::cout << "Basic Discrete GA test PASSED\n\n";

  // Run additional challenging tests using the same standardized parameters
  test_long_vector(standard_params);
  test_multimodal(standard_params);
  test_deceptive(standard_params);
  test_multi_optimum(standard_params);
  
  // --- From the second file: Original Discrete test with different test vector ---
  std::vector<int> vector_orig = {
      1, 0, 3, 2, 1, 0, 4, 3, 2, 1, 0,
      3, 4, 2, 1, 0, 3, 2, 1, 4, 0
  };
  int L_orig = vector_orig.size();
  
  std::cout << "=== Original Discrete GA Test ===\n";
  code = optimize(L_orig, vector_orig.data(), test_function_orig, always_valid_int, standard_params);
  if (code != 0) {
    std::cerr << "ERROR: optimize() returned code " << code << "\n";
    return code;
  }
  
  // Print out what the GA found
  auto stats_orig = get_last_optimization_result();
  std::cout << "Final original discrete genome: ";
  for (int x : vector_orig)
    std::cout << x << ' ';
  std::cout << "\nBest fitness: " << stats_orig.best_fitness
            << " (generations: " << stats_orig.generations << ")\n";
  
  // Verify result
  bool ok_orig = true;
  for (int i = 0; i < L_orig; ++i) {
    if (vector_orig[i] != int_test_answer_orig[i]) {
      std::cerr << "Mismatch at index " << i << ": got " << vector_orig[i]
                << " expected " << int_test_answer_orig[i] << "\n";
      ok_orig = false;
    }
  }
  if (!ok_orig) {
    std::cerr << "Original Discrete GA test FAILED\n";
    return 1;
  }
  std::cout << "Original Discrete GA test PASSED\n\n";
 
  // --- Continuous test ---
  std::vector<double> vector2 = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5};
  int L2 = vector2.size();
 
  std::cout << "=== Continuous GA Test ===\n";
  code = optimize(L2, vector2.data(), real_test_function, always_valid_real,
                  standard_params);
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
  const double eps = 1e-4;
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