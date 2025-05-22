#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "CCircuit.h"
#include "CSimulator.h"
#include "Config.h" // <— your new loader
#include "Genetic_Algorithm.h"

int main() {
  // Save original cout buffer before we start
  std::streambuf *original_cout_buffer = std::cout.rdbuf();

  // Create null stream to discard output
  std::ofstream null_stream("/dev/null");

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
  std::cout
      << "GA parameters:\n"
      << "  mode                        = " << params.mode << "\n"
      << "  random_seed                 = " << params.random_seed << "\n\n"

      << "  population_size             = " << params.population_size << "\n"
      << "  elite_count                 = " << params.elite_count << "\n"
      << "  max_iterations              = " << params.max_iterations << "\n\n"

      << "  tournament_size             = " << params.tournament_size << "\n"
      << "  selection_pressure          = " << params.selection_pressure
      << "\n\n"

      << "  crossover_probability       = " << params.crossover_probability
      << "\n"
      << "  crossover_points            = " << params.crossover_points << "\n\n"

      << "  mutation_probability        = " << params.mutation_probability
      << "\n"
      << "  mutation_step_size          = " << params.mutation_step_size << "\n"
      << "  allow_mutation_wrapping     = " << std::boolalpha
      << params.allow_mutation_wrapping << "\n\n"

      << "  use_inversion               = " << std::boolalpha
      << params.use_inversion << "\n"
      << "  inversion_probability       = " << params.inversion_probability
      << "\n\n"

      << "  use_scaling_mutation        = " << std::boolalpha
      << params.use_scaling_mutation << "\n"
      << "  scaling_mutation_prob       = " << params.scaling_mutation_prob
      << "\n"
      << "  scaling_mutation_min        = " << params.scaling_mutation_min
      << "\n"
      << "  scaling_mutation_max        = " << params.scaling_mutation_max
      << "\n\n"

      << "  convergence_threshold       = " << params.convergence_threshold
      << "\n"
      << "  stall_generations           = " << params.stall_generations
      << "\n\n"

      << "  verbose                     = " << std::boolalpha << params.verbose
      << "\n"
      << "  log_results                 = " << std::boolalpha
      << params.log_results << "\n"
      << "  log_file                    = " << params.log_file << "\n\n";

  // Optimisation mode
  auto mode = params.mode; // “d”, “c” or “h” from parameters.txt
  std::cout << "Mode: " << mode << "\n";

  // Set number of units
  constexpr int num_units = 10;
  constexpr int vector_size = 2 * num_units + 1;

  // Create vectors to hold the optimization results
  int circuit_vector[vector_size] = {0};
  double volume_params[num_units];
  for (int i = 0; i < num_units; i++) {
    volume_params[i] = 0.5; // Initialize at middle of range
  }
  if (mode == "d") {
    std::cout << "Running DISCRETE optimization...\n";
    std::cout.rdbuf(null_stream.rdbuf()); // silence

    auto discrete_fitness = [](int size, int *vec) -> double {
      // Discrete-only overload
      return circuit_performance(size, vec);
    };

    auto discrete_validity = [](int size, int *vec) -> bool {
      // Discrete-only constructor and validity
      Circuit c(size / 2); // (2n + 1 → n units)
      c.initialize_from_vector(size, vec);
      return c.check_validity(size, vec);
    };

    optimize(vector_size, circuit_vector, discrete_fitness, discrete_validity,
             params);
  }

  else if (mode == "c") {
    std::cout << "Running CONTINUOUS optimization...\n";
    std::cout.rdbuf(null_stream.rdbuf()); // silence

    // Known-valid discrete circuit (hardcoded)
    const int fixed_circuit[vector_size] = {1, 2, 4, 3,  5, 3, 0, 8, 11, 7, 12,
                                            7, 0, 7, 11, 8, 6, 9, 7, 10, 3};
    std::copy(fixed_circuit, fixed_circuit + vector_size, circuit_vector);

    auto cont_fitness = [&](int r_size, double *rvec) -> double {
      return circuit_performance(vector_size, circuit_vector, r_size, rvec);
    };

    auto cont_validity = [&](int r_size, double *rvec) -> bool {
      Circuit c(num_units, rvec); // use volume constructor!
      c.initialize_from_vector(vector_size, circuit_vector, rvec);
      return c.check_validity(vector_size, circuit_vector, r_size, rvec);
    };

    optimize(num_units, volume_params, cont_fitness, cont_validity, params);
  }

  else {
    std::cout << "Running hybrid optimization (connections + volumes)...\n";

    // Redirect cout to null stream to silence debug output
    std::cout.rdbuf(null_stream.rdbuf());

    // Define hybrid fitness and validity functions
    auto hybrid_fitness = [](int i_size, int *i_vec, int r_size,
                             double *r_vec) -> double {
      return circuit_performance(i_size, i_vec, r_size, r_vec);
    };

    auto hybrid_validity = [num_units](int i_size, int *i_vec, int r_size,
                                       double *r_vec) -> bool {
      Circuit c(num_units);
      c.initialize_from_vector(i_size, i_vec);
      return c.check_validity(i_size, i_vec, r_size, r_vec);
    };

    // Run hybrid optimization (cout is redirected, so no debug output)
    optimize(vector_size, circuit_vector, num_units, volume_params,
             hybrid_fitness, hybrid_validity, params);
  }

  // Calculate performance with optimized values (still silent)
  double performance = circuit_performance(vector_size, circuit_vector,
                                           num_units, volume_params);

  // Create a circuit object for detailed analysis, still silent
  Circuit circuit(num_units, volume_params);
  circuit.initialize_from_vector(vector_size, circuit_vector, volume_params);
  circuit.run_mass_balance();

  // Extract important metrics before restoring cout
  double palusznium_recovery = circuit.get_palusznium_recovery() * 100;
  double palusznium_grade = circuit.get_palusznium_grade() * 100;
  double gormanium_recovery = circuit.get_gormanium_recovery() * 100;
  double gormanium_grade = circuit.get_gormanium_grade() * 100;

  // Calculate volumes and costs while still silent
  double total_volume = 0.0;
  double unit_volumes[num_units];
  for (int i = 0; i < num_units; i++) {
    if (mode == "h" || mode == "c") {
      // Use scaled volumes
      double min_volume = 2.5;
      double max_volume = 20.0;
      unit_volumes[i] =
          min_volume + (max_volume - min_volume) * volume_params[i];
    } else {
      // Discrete mode: use fixed volume
      unit_volumes[i] = 10.0;
    }
    total_volume += unit_volumes[i];
  }

  double operating_cost = 5.0 * std::pow(total_volume, 2.0 / 3.0);
  if (total_volume >= 150.0) {
    operating_cost += 1000.0 * std::pow(total_volume - 150.0, 2.0);
  }

  // Now RESTORE cout to print results
  std::cout.rdbuf(original_cout_buffer);

  // Print final results after optimization
  std::cout << "\nOptimization complete!\n";
  std::cout << "Final circuit economic value: £" << std::fixed
            << std::setprecision(2) << performance << " per second\n\n";

  // Display the optimized circuit vector
  std::cout << "Optimized circuit vector: ";
  for (int i = 0; i < vector_size; ++i)
    std::cout << circuit_vector[i] << " ";
  std::cout << std::endl;

  // Display the optimized volumes
  std::cout << "Optimized volume parameters: ";
  for (int i = 0; i < num_units; ++i)
    std::cout << std::fixed << std::setprecision(5) << volume_params[i] << " ";
  std::cout << std::endl;

  // Display circuit performance metrics
  std::cout << "\nCircuit Performance:\n";
  std::cout << "- Palusznium recovery: " << std::fixed << std::setprecision(2)
            << palusznium_recovery << "%\n";
  std::cout << "- Palusznium grade: " << std::fixed << std::setprecision(2)
            << palusznium_grade << "%\n";
  std::cout << "- Gormanium recovery: " << std::fixed << std::setprecision(2)
            << gormanium_recovery << "%\n";
  std::cout << "- Gormanium grade: " << std::fixed << std::setprecision(2)
            << gormanium_grade << "%\n";

  // Circuit configuration analysis
  std::cout << "\nCircuit Configuration Analysis:\n";
  int direct_to_p = 0, direct_to_g = 0, direct_to_t = 0, recycles = 0;
  for (int i = 0; i < num_units; i++) {
    // Check concentrate connections
    int conc_dest = circuit_vector[1 + 2 * i];
    if (conc_dest == num_units)
      direct_to_p++;
    else if (conc_dest == num_units + 1)
      direct_to_g++;
    else if (conc_dest == num_units + 2)
      direct_to_t++;
    else if (conc_dest < i)
      recycles++;

    // Check tailing connections
    int tail_dest = circuit_vector[2 + 2 * i];
    if (tail_dest == num_units)
      direct_to_p++;
    else if (tail_dest == num_units + 1)
      direct_to_g++;
    else if (tail_dest == num_units + 2)
      direct_to_t++;
    else if (tail_dest < i)
      recycles++;
  }

  std::cout << "- Units sending to Palusznium product: " << direct_to_p << "\n";
  std::cout << "- Units sending to Gormanium product: " << direct_to_g << "\n";
  std::cout << "- Units sending to Tailings: " << direct_to_t << "\n";
  std::cout << "- Recycle connections: " << recycles << "\n";

  // Unit volume analysis
  std::cout << "\nUnit Volumes (m³):\n";
  for (int i = 0; i < num_units; i++) {
    std::cout << "Unit " << i << ": " << std::fixed << std::setprecision(2)
              << unit_volumes[i] << " m³\n";
  }
  std::cout << "Total volume: " << std::fixed << std::setprecision(2)
            << total_volume << " m³\n";

  // Economic analysis
  std::cout << "\nEconomic Analysis:\n";
  double palusznium_value = circuit.get_palusznium_recovery() * 8 * 120;
  double gormanium_value = circuit.get_gormanium_recovery() * 12 * 80;
  std::cout << "- Palusznium revenue: £" << std::fixed << std::setprecision(2)
            << palusznium_value << "/s\n";
  std::cout << "- Gormanium revenue: £" << std::fixed << std::setprecision(2)
            << gormanium_value << "/s\n";
  std::cout << "- Total revenue: £" << std::fixed << std::setprecision(2)
            << (palusznium_value + gormanium_value) << "/s\n";
  std::cout << "- Operating cost: £" << std::fixed << std::setprecision(2)
            << operating_cost << "/s\n";
  std::cout << "- Net profit: £" << std::fixed << std::setprecision(2)
            << performance << "/s\n";

  // Save raw circuit data into a CSV:
  const std::string out_csv = "circuit_results.csv";
  if (circuit.save_output_info(out_csv)) {
    std::cout << "\n Saved detailed circuit info to " << out_csv << "\n";
  } else {
    std::cerr << "\n  Failed to write circuit info to " << out_csv << "\n";
  }

  return 0;
}