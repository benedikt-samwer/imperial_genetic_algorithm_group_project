/** Header for the Genetic Algorithm library
 * This header defines the interface for the genetic algorithm optimization
 */

#pragma once

#include <functional>
#include <vector>

// Structure to hold genetic algorithm parameters
struct Algorithm_Parameters {
  // General parameters
  int max_iterations = 1000; // Maximum number of generations
  int population_size = 100; // Number of individuals in the population
  int elite_count = 1;       // Number of best individuals to keep unchanged

  // Selection parameters
  double selection_pressure = 1.5; // Linear rank selection pressure parameter
  int tournament_size = 2;         // Number of contenders per tournament

  // Crossover parameters
  double crossover_probability = 0.8; // Probability of crossover
  int crossover_points = 1;           // Number of crossover points (1 or 2)

  // Mutation parameters
  double mutation_probability = 0.01; // Probability of mutation per gene
  int mutation_step_size = 2;         // Maximum change in value during mutation
  bool allow_mutation_wrapping = true; // Allow mutations to wrap around

  // Inversion‐mutation parameters
  bool use_inversion = true;           // turn inversion on/off
  double inversion_probability = 0.05; // chance to invert per child

  // Termination criteria
  double convergence_threshold = 1e-6; // Convergence threshold
  int stall_generations = 50;          // Max generations with no improvement

  // Debug options
  bool verbose = false;                // Print progress information
  bool log_results = false;            // Log results to file
  std::string log_file = "ga_log.txt"; // Log file name
};

// Default algorithm parameters
#define DEFAULT_ALGORITHM_PARAMETERS                                           \
  Algorithm_Parameters {}

// Validity checking functions
bool all_true(int int_vector_size, int *int_vector, int real_vector_size,
              double *real_vector);
bool all_true_ints(int int_vector_size, int *vector);
bool all_true_reals(int real_vector_size, double *vector);

// Optimization function for discrete vector
int optimize(
    int int_vector_size, int *int_vector,
    std::function<double(int, int *)> func,
    std::function<bool(int, int *)> validity = all_true_ints,
    Algorithm_Parameters algorithm_parameters = DEFAULT_ALGORITHM_PARAMETERS);

// Optimization function for continuous vector
int optimize(
    int real_vector_size, double *real_vector,
    std::function<double(int, double *)> func,
    std::function<bool(int, double *)> validity = all_true_reals,
    Algorithm_Parameters algorithm_parameters = DEFAULT_ALGORITHM_PARAMETERS);

// Optimization function for mixed discrete-continuous vector
int optimize(
    int int_vector_size, int *int_vector, int real_vector_size,
    double *real_vector, std::function<double(int, int *, int, double *)> func,
    std::function<bool(int, int *, int, double *)> validity = all_true,
    Algorithm_Parameters algorithm_parameters = DEFAULT_ALGORITHM_PARAMETERS);

// Structure to hold statistics about the optimization process
struct OptimizationResult {
  double best_fitness; // Best fitness value found
  int generations;     // Number of generations run
  double avg_fitness;  // Average fitness of final population
  double std_fitness;  // Standard deviation of final population fitness
  double time_taken;   // Time taken for optimization (seconds)
  bool converged;      // Whether algorithm converged

  // Default constructor
  OptimizationResult()
      : best_fitness(0), generations(0), avg_fitness(0), std_fitness(0),
        time_taken(0), converged(false) {}
};

// Get the last optimization result
OptimizationResult get_last_optimization_result();

// Other functions and variables
