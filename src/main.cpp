#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

#include "CCircuit.h"
#include "CSimulator.h"
#include "Config.h" // <— your new loader
#include "Genetic_Algorithm.h"

int main() {
  std::cout << "=== Palusznium Rush Circuit Optimizer ===\n\n";

  // load GA & random‐seed settings from parameters.txt
  Algorithm_Parameters params;
  load_parameters("parameters.txt", params);

  // Optionally fix the RNG for reproducibility
  if (params.random_seed >= 0) {
    set_random_seed(params.random_seed);
    std::cout << "* Using fixed seed: " << params.random_seed << "\n";
  }

  // Print to see
  std::cout << "GA parameters:\n"
            << "  population_size        = " << params.population_size << "\n"
            << "  max_iterations         = " << params.max_iterations << "\n"
            << "  tournament_size        = " << params.tournament_size << "\n"
            << "  crossover_probability  = " << params.crossover_probability
            << "\n"
            << "  mutation_probability   = " << params.mutation_probability
            << "\n"
            << "  mutation_step_size     = " << params.mutation_step_size
            << "\n"
            << "  use_inversion          = " << std::boolalpha
            << params.use_inversion << "\n"
            << "  inversion_probability  = " << params.inversion_probability
            << "\n"
            << "  convergence_threshold  = " << params.convergence_threshold
            << "\n"
            << "  stall_generations      = " << params.stall_generations
            << "\n\n";

  // Problem setup
  constexpr int num_units = 10;
  constexpr int vector_size = 2 * num_units + 1;
  int circuit_vector[vector_size] = {0};

  // Build our fitness & validity wrappers
  auto fitness = [&](int L, int *v) {
    // simulator returns “economic value”
    return circuit_performance(L, v);
  };
  auto validity = [num_units](int L, int *v) {
    Circuit c(num_units);
    c.initialize_from_vector(L, v);
    return c.check_validity(L, v);
  };

  std::cout << "Starting GA on a " << num_units << "-unit circuit…\n\n";

  // Run it
  int code = optimize(vector_size, circuit_vector, fitness, validity, params);
  if (code != 0) {
    std::cerr << "ERROR: GA returned code " << code << "\n";
    return code;
  }

  // Report results
  double best_val = circuit_performance(vector_size, circuit_vector);
  std::cout << "\nOptimization complete!\n"
            << "  Best economic value: £" << std::fixed << std::setprecision(2)
            << best_val << " /s\n";

  std::cout << "Optimized circuit vector:\n  ";
  for (int i = 0; i < vector_size; ++i)
    std::cout << circuit_vector[i] << (i + 1 < vector_size ? ' ' : '\n');

  // (Optional) re-run simulator for detailed debug output
  std::cout << "\nDetailed circuit metrics:\n";
  circuit_performance(vector_size, circuit_vector);

  return 0;
}