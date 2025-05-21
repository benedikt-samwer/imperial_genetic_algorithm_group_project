#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <random>
#include <vector>
#include <set>
#include "CCircuit.h"
#include "Genetic_Algorithm.h"

// Helper RNG
static std::mt19937 &rng() {
  static thread_local std::mt19937 gen(std::random_device{}());
  return gen;
}

// Generate a valid circuit based on a template pattern
std::vector<int> generate_valid_circuit_template(int num_units) {
    const int n = num_units;
    const int vec_size = 2*n + 1;
    std::vector<int> vec(vec_size);
    
    // Set feed to unit 0 (most common valid configuration)
    vec[0] = 0;
    
    // Basic linear flow pattern with some recycling
    for (int i = 0; i < n; i++) {
        // For the concentrate stream (high-grade)
        if (i < n-1) {
            // Forward flow to next unit for most units
            vec[2*i + 1] = i+1;
        } else {
            // Last unit sends concentrate to Palusznium product (n)
            vec[2*i + 1] = n;
        }
        
        // For the tailings stream
        if (i % 3 == 0) {
            // Every 3rd unit sends tailings to Gormanium product
            vec[2*i + 2] = n+1;
        } else if (i % 3 == 1) {
            // Every 3rd+1 unit sends tailings to final tailings
            vec[2*i + 2] = n+2;
        } else {
            // Other units recycle tailings back to unit 0
            vec[2*i + 2] = 0;
        }
    }
    
    return vec;
}

// Function to apply random valid variations to a template
std::vector<int> create_varied_circuit(const std::vector<int>& template_vec, int num_units, 
                                       std::function<bool(int, int*)> validity_check) {
    const int n = num_units;
    std::vector<int> result = template_vec;
    
    // Apply a few random changes
    int num_changes = std::uniform_int_distribution<int>(1, n)(rng());
    
    // Try multiple times to create a valid variation
    for (int attempt = 0; attempt < 20; attempt++) {
        std::vector<int> candidate = template_vec;
        
        for (int i = 0; i < num_changes; i++) {
            // Pick a random position to modify (excluding feed position)
            int pos = std::uniform_int_distribution<int>(1, 2*n)(rng());
            
            // Determine which unit this connection belongs to
            int unit_idx = (pos-1) / 2;
            
            // Pick a valid destination (any unit or terminal except self)
            std::vector<int> valid_dests;
            for (int dest = 0; dest < n+3; dest++) {
                if (dest != unit_idx) { // Avoid self-loop
                    valid_dests.push_back(dest);
                }
            }
            
            // Shuffle possible destinations
            std::shuffle(valid_dests.begin(), valid_dests.end(), rng());
            
            // Try each possible destination until we find one that works
            bool found_valid = false;
            for (int dest : valid_dests) {
                int old_val = candidate[pos];
                candidate[pos] = dest;
                
                // Check if both connections from this unit point to the same place
                int other_conn = (pos % 2 == 1) ? pos+1 : pos-1;
                if (other_conn < candidate.size() && candidate[other_conn] == dest) {
                    candidate[pos] = old_val; // Restore old value
                    continue;  // Can't have both connections to same place
                }
                
                // Check if the circuit is valid with this change
                if (validity_check(candidate.size(), candidate.data())) {
                    found_valid = true;
                    break;
                } else {
                    candidate[pos] = old_val; // Restore old value
                }
            }
            
            if (!found_valid) {
                // If we couldn't find a valid change for this position, try another
                continue;
            }
        }
        
        // Check if the final candidate is valid
        if (validity_check(candidate.size(), candidate.data())) {
            return candidate;
        }
    }
    
    // If all attempts failed, return the original template
    return template_vec;
}

// Generate a diverse population of valid circuits
std::vector<std::vector<int>> generate_initial_population(int population_size, int num_units,
                                                         std::function<bool(int, int*)> validity_check) {
    std::vector<std::vector<int>> population;
    std::set<std::vector<int>> unique_circuits; // To ensure uniqueness
    
    // Create base templates
    std::vector<std::vector<int>> templates;
    
    // Template 1: Linear flow with recycling
    templates.push_back(generate_valid_circuit_template(num_units));
    
    // Template 2: Alternating product outputs
    auto template2 = generate_valid_circuit_template(num_units);
    for (int i = 0; i < num_units; i++) {
        if (i % 2 == 0) {
            template2[2*i + 1] = num_units;     // Even units send concentrate to Palusznium
            template2[2*i + 2] = num_units + 2; // and tailings to final tailings
        } else {
            template2[2*i + 1] = num_units + 1; // Odd units send concentrate to Gormanium
            template2[2*i + 2] = 0;             // and tailings back to first unit
        }
    }
    if (validity_check(template2.size(), template2.data())) {
        templates.push_back(template2);
    }
    
    // Template 3: Butterfly pattern
    auto template3 = generate_valid_circuit_template(num_units);
    for (int i = 0; i < num_units; i++) {
        if (i < num_units/2) {
            template3[2*i + 1] = i + num_units/2; // First half feed to second half
            template3[2*i + 2] = num_units + 2;   // Tailings to final tailings
        } else {
            template3[2*i + 1] = num_units;       // Second half to products
            template3[2*i + 2] = num_units + 1;
        }
    }
    if (validity_check(template3.size(), template3.data())) {
        templates.push_back(template3);
    }
    
    // Add templates directly to population
    for (const auto& tmpl : templates) {
        population.push_back(tmpl);
        unique_circuits.insert(tmpl);
    }
    
    // Generate variations until we have enough unique circuits
    int max_attempts = population_size * 10;
    int attempts = 0;
    
    std::cout << "Generating initial population of valid circuits..." << std::endl;
    
    while (population.size() < population_size && attempts < max_attempts) {
        // Pick a random template
        const auto& tmpl = templates[std::uniform_int_distribution<int>(0, templates.size()-1)(rng())];
        
        // Create a variation
        auto candidate = create_varied_circuit(tmpl, num_units, validity_check);
        
        // Check uniqueness
        if (unique_circuits.find(candidate) == unique_circuits.end()) {
            population.push_back(candidate);
            unique_circuits.insert(candidate);
            
            if (population.size() % 10 == 0) {
                std::cout << "Generated " << population.size() << " valid circuits" << std::endl;
            }
        }
        
        attempts++;
    }
    
    std::cout << "Initial population: " << population.size() << " valid circuits" << std::endl;
    
    return population;
}

// default validity checks
bool all_true(int iv, int *ivs, int rv, double *rvs) { return true; }
bool all_true_ints(int iv, int *ivs) { return true; }
bool all_true_reals(int iv, double *rvs) { return true; }

// (if not in another file) backing storage + accessor:
static OptimizationResult last_result;
OptimizationResult get_last_optimization_result() { return last_result; }

// ********************************************************************
// 1) Discrete-only optimize with improved initialization
// ********************************************************************
int optimize(int int_vector_size, int *int_vector,
             std::function<double(int, int *)> func,
             std::function<bool(int, int *)> validity,
             Algorithm_Parameters params) {
    using Clock = std::chrono::high_resolution_clock;
    auto t0 = Clock::now();

    // --- 1. Improved population initialization
    int n_units = (int_vector_size - 1) / 2;
    std::cout << "Initializing population for " << n_units << " units..." << std::endl;
    
    // Generate valid initial population
    std::vector<std::vector<int>> population = 
        generate_initial_population(params.population_size, n_units, validity);
    
    // If we couldn't generate enough valid circuits, adjust population size
    if (population.size() < params.population_size) {
        std::cout << "Warning: Could only generate " << population.size() 
                  << " valid circuits, adjusting population size" << std::endl;
        params.population_size = population.size();
    }

    double best_overall = -1e300;              // best seen so far
    int stall_count = 0;                       // gens since last improvement
    double eps = params.convergence_threshold; // "meaningful" fitness delta
    int max_stall = params.stall_generations;  // allowed idle generations

    // --- 2. Main GA loop
    for (int gen = 0; gen < params.max_iterations; ++gen) {
        // 2a) Evaluate fitness of each genome
        std::vector<double> fitnesses(population.size());
        for (size_t i = 0; i < population.size(); ++i) {
            int *gdata = population[i].data();
            if (!validity(int_vector_size, gdata)) {
                fitnesses[i] = -1e9; // heavy penalty
            } else {
                fitnesses[i] = func(int_vector_size, gdata);
            }
        }

        double gen_best = *std::max_element(fitnesses.begin(), fitnesses.end());
        if (gen_best > best_overall + eps) {
            best_overall = gen_best;
            stall_count = 0; // reset when we see new best
        } else {
            stall_count++;
        }
        if (stall_count >= max_stall) {
            if (params.verbose) {
                std::cout << "[GA] No improvement for " << stall_count
                        << " generations—stopping early.\n";
            }
            break; // exit the generation loop
        }

        // 2b) Elitism: copy best genome to next generation
        std::vector<std::vector<int>> next_gen;
        {
            auto best_it = std::max_element(fitnesses.begin(), fitnesses.end());
            size_t best_idx = std::distance(fitnesses.begin(), best_it);
            next_gen.push_back(population[best_idx]);
        }

        // ----- TOURNAMENT SETUP -----
        int k = params.tournament_size > 0 ? params.tournament_size : 2;
        std::uniform_int_distribution<size_t> pop_dist(0, population.size() - 1);
        auto pick_parent = [&]() {
            size_t best = pop_dist(rng());
            double best_fit = fitnesses[best];
            for (int i = 1; i < k; ++i) {
                size_t idx = pop_dist(rng());
                if (fitnesses[idx] > best_fit) {
                    best = idx;
                    best_fit = fitnesses[idx];
                }
            }
            return population[best];
        };

        // 2c) Fill rest via selection, crossover, mutation
        std::uniform_real_distribution<double> u01(0.0, 1.0);
        while (next_gen.size() < population.size()) {
            // – Selection via k-way tournament
            auto p1 = pick_parent();
            auto p2 = pick_parent();

            // – Crossover
            std::vector<int> c1 = p1, c2 = p2;
            if (u01(rng()) < params.crossover_probability) {
                // Adaptive crossover points: more early on, fewer later
                double progress = static_cast<double>(gen) / params.max_iterations;
                int max_points =
                    std::min(5, int_vector_size / 2); // limit excessive cuts
                int num_cuts = static_cast<int>((1.0 - progress) * max_points);
                num_cuts = std::max(1, num_cuts); // always at least 1 point

                std::vector<bool> crossover_mask(int_vector_size, false);
                for (int i = 0; i < num_cuts; ++i) {
                    int cut =
                        std::uniform_int_distribution<int>(0, int_vector_size - 1)(rng());
                    crossover_mask[cut] = true;
                }

                bool flip = false;
                for (int j = 0; j < int_vector_size; ++j) {
                    if (crossover_mask[j])
                        flip = !flip;
                    if (flip)
                        std::swap(c1[j], c2[j]);
                }
            }
            
            // – Mutation (creep + optional inversion)
            {
                // 1) Substitution ("creep") mutation on both children
                int min_gene = 0;
                int max_gene = n_units + 2;
                int range = max_gene - min_gene + 1;
                std::uniform_int_distribution<int> step_dist(-params.mutation_step_size,
                                                            params.mutation_step_size);
                for (auto *child : {&c1, &c2}) {
                    for (int j = 0; j < int_vector_size; ++j) {
                        if (u01(rng()) < params.mutation_probability) {
                            int step = step_dist(rng());
                            int val = (*child)[j] + step;
                            (*child)[j] =
                                min_gene + ((val - min_gene) % range + range) % range;
                        }
                    }
                }

                // 2) Inversion mutation, if enabled
                if (params.use_inversion) {
                    // pick two indices a < b
                    std::uniform_int_distribution<int> a_dist(0, int_vector_size - 2);
                    int a = a_dist(rng());
                    std::uniform_int_distribution<int> b_dist(a + 1, int_vector_size - 1);
                    int b = b_dist(rng());

                    // reverse that slice in each child with its own probability
                    if (u01(rng()) < params.inversion_probability) {
                        std::reverse(c1.begin() + a, c1.begin() + b + 1);
                    }
                    if (u01(rng()) < params.inversion_probability) {
                        std::reverse(c2.begin() + a, c2.begin() + b + 1);
                    }
                }
            }

            // Check validity of children and add valid ones
            if (validity(int_vector_size, c1.data())) {
                next_gen.push_back(std::move(c1));
            }
            
            if (next_gen.size() < population.size() && validity(int_vector_size, c2.data())) {
                next_gen.push_back(std::move(c2));
            }
        }

        // 2d) Replace population
        population.swap(next_gen);

        if (params.verbose && gen % 10 == 0) {
            std::cout << "[GA] Gen " << gen << " best fitness "
                    << *std::max_element(fitnesses.begin(), fitnesses.end())
                    << std::endl;
        }
    }

    // --- 3. Write best genome back into int_vector[]
    // (Re-evaluate final fitness to find the winner)
    double best_fit = -1e12;
    std::vector<int> *best_genome = nullptr;
    for (auto &g : population) {
        double fit = func(int_vector_size, g.data());
        if (fit > best_fit) {
            best_fit = fit;
            best_genome = &g;
        }
    }
    if (best_genome) {
        for (int i = 0; i < int_vector_size; ++i)
            int_vector[i] = (*best_genome)[i];
    }

    // Store optimization results
    last_result.best_fitness = best_fit;
    last_result.generations = params.max_iterations;
    
    auto t1 = Clock::now();
    if (params.verbose) {
        double secs = std::chrono::duration<double>(t1 - t0).count();
        std::cout << "[GA] Completed in " << secs << "s, best_fitness=" << best_fit
                << "\n";
    }

    return 0;
}

// ********************************************************************
// 2) Continuous-only optimize
// ********************************************************************
int optimize(int real_vector_size, double *real_vector,
             std::function<double(int, double *)> func,
             std::function<bool(int, double *)> validity,
             Algorithm_Parameters params) {
  using Clock = std::chrono::high_resolution_clock;
  auto t0 = Clock::now();

  std::uniform_real_distribution<double> dist01(0.0, 1.0);
  std::vector<std::vector<double>> population;

  // --- 1. Initialise population
  population.reserve(params.population_size);
  while (population.size() < params.population_size) {
    std::vector<double> genome(real_vector_size);
    for (auto &g : genome)
      g = dist01(rng()); // all β_i in [0,1]
    if (validity(real_vector_size, genome.data()))
      population.push_back(std::move(genome));
  }

  double best_overall = -1e300;
  int stall_count = 0;
  double eps = params.convergence_threshold;
  int max_stall = params.stall_generations;

  for (int gen = 0; gen < params.max_iterations; ++gen) {
    // Evaluate fitness
    std::vector<double> fitnesses(population.size());
    for (size_t i = 0; i < population.size(); ++i) {
      fitnesses[i] = validity(real_vector_size, population[i].data())
                         ? func(real_vector_size, population[i].data())
                         : -1e9;
    }

    double gen_best = *std::max_element(fitnesses.begin(), fitnesses.end());
    if (gen_best > best_overall + eps) {
      best_overall = gen_best;
      stall_count = 0;
    } else {
      stall_count++;
    }

    if (stall_count >= max_stall) {
      if (params.verbose)
        std::cout << "[GA-Real] No improvement for " << stall_count
                  << " generations — stopping.\n";
      break;
    }

    // Elitism
    std::vector<std::vector<double>> next_gen;
    {
      auto best_it = std::max_element(fitnesses.begin(), fitnesses.end());
      size_t best_idx = std::distance(fitnesses.begin(), best_it);
      next_gen.push_back(population[best_idx]);
    }

    // Tournament selection
    int k = params.tournament_size > 0 ? params.tournament_size : 2;
    std::uniform_int_distribution<size_t> pop_dist(0, population.size() - 1);
    auto pick_parent = [&]() {
      size_t best = pop_dist(rng());
      double best_fit = fitnesses[best];
      for (int i = 1; i < k; ++i) {
        size_t idx = pop_dist(rng());
        if (fitnesses[idx] > best_fit) {
          best = idx;
          best_fit = fitnesses[idx];
        }
      }
      return population[best];
    };

    // Crossover + Mutation
    while (next_gen.size() < population.size()) {
      auto p1 = pick_parent();
      auto p2 = pick_parent();
      std::vector<double> c1 = p1, c2 = p2;

      if (dist01(rng()) < params.crossover_probability) {
        for (int j = 0; j < real_vector_size; ++j) {
          if (dist01(rng()) < 0.5)
            std::swap(c1[j], c2[j]);
        }
      }

      // Mutation
      for (int j = 0; j < real_vector_size; ++j) {
        if (dist01(rng()) < params.mutation_probability) {
          double step = dist01(rng()) * params.mutation_step_size;
          c1[j] = std::clamp(c1[j] + step * (dist01(rng()) < 0.5 ? -1 : 1), 0.0,
                             1.0);
        }
        if (dist01(rng()) < params.mutation_probability) {
          double step = dist01(rng()) * params.mutation_step_size;
          c2[j] = std::clamp(c2[j] + step * (dist01(rng()) < 0.5 ? -1 : 1), 0.0,
                             1.0);
        }
      }

      next_gen.push_back(std::move(c1));
      if (next_gen.size() < population.size())
        next_gen.push_back(std::move(c2));
    }

    population.swap(next_gen);

    if (params.verbose && gen % (params.max_iterations / 10) == 0) {
      std::cout << "[GA-Real] Gen " << gen << " best fitness " << gen_best
                << "\n";
    }
  }

  // Copy best solution
  double best_fit = -1e12;
  std::vector<double> *best_genome = nullptr;
  for (auto &g : population) {
    double fit = func(real_vector_size, g.data());
    if (fit > best_fit) {
      best_fit = fit;
      best_genome = &g;
    }
  }
  if (best_genome) {
    for (int i = 0; i < real_vector_size; ++i)
      real_vector[i] = (*best_genome)[i];
  }

  // Store optimization results
  last_result.best_fitness = best_fit;
  last_result.generations = params.max_iterations - stall_count;

  auto t1 = Clock::now();
  if (params.verbose) {
    double secs = std::chrono::duration<double>(t1 - t0).count();
    std::cout << "[GA-Real] Completed in " << secs
              << "s, best_fitness=" << best_fit << "\n";
  }

  return 0;
}

// ********************************************************************
// 3) Hybrid optimize (both discrete and continuous)
// ********************************************************************
int optimize(int int_vector_size, int *int_vector, int real_vector_size,
             double *real_vector,
             std::function<double(int, int *, int, double *)> func,
             std::function<bool(int, int *, int, double *)> validity,
             Algorithm_Parameters params) {
  using Clock = std::chrono::high_resolution_clock;
  auto t0 = Clock::now();

  std::uniform_real_distribution<double> u01(0.0, 1.0);
  std::uniform_real_distribution<double> mutation_step_dist(-params.mutation_step_size,
                                                            params.mutation_step_size);
  std::uniform_int_distribution<int> int_step_dist(-params.mutation_step_size,
                                                   params.mutation_step_size);

  int n_units = (int_vector_size - 1) / 2;
  int min_gene = 0; // Changed from -3 to match circuit vector layout
  int max_gene = n_units + 2;
  int gene_range = max_gene - min_gene + 1;
  std::uniform_int_distribution<int> gene_dist(min_gene, max_gene);

  // --- Initialize population using improved initialization
  std::vector<std::pair<std::vector<int>, std::vector<double>>> population;
  
  // We'll use the improved discrete initialization for the integer part
  auto validity_int_only = [&](int size, int* ivec) {
      // Create a dummy volume vector for validity check
      std::vector<double> dummy_vol(real_vector_size, 0.5);
      return validity(size, ivec, real_vector_size, dummy_vol.data());
  };
  
  // Generate valid circuit templates
  auto int_templates = generate_initial_population(params.population_size, n_units, validity_int_only);
  
  // Now create population with both int and real parts
  while (population.size() < params.population_size && !int_templates.empty()) {
      // Get a valid integer vector
      std::vector<int> int_part = int_templates.back();
      int_templates.pop_back();
      
      // Create random real vector
      std::vector<double> real_part(real_vector_size);
      for (double &x : real_part) x = u01(rng());
      
      // Check combined validity
      if (validity(int_vector_size, int_part.data(), 
                  real_vector_size, real_part.data())) {
          population.emplace_back(std::move(int_part), std::move(real_part));
      }
  }
  
  // If we couldn't generate enough valid individuals, adjust population size
  if (population.size() < params.population_size) {
      std::cout << "Warning: Could only generate " << population.size() 
                << " valid combined circuits, adjusting population size" << std::endl;
      params.population_size = population.size();
  }

  double best_overall = -1e300;
  int stall_count = 0;

  for (int gen = 0; gen < params.max_iterations; ++gen) {
    // Evaluate fitness
    std::vector<double> fitnesses(population.size());
    for (size_t i = 0; i < population.size(); ++i) {
      auto &[i_part, r_part] = population[i];
      fitnesses[i] = validity(int_vector_size, i_part.data(),
                              real_vector_size, r_part.data())
                         ? func(int_vector_size, i_part.data(),
                                real_vector_size, r_part.data())
                         : -1e9;
    }

    double gen_best = *std::max_element(fitnesses.begin(), fitnesses.end());
    if (gen_best > best_overall + params.convergence_threshold) {
      best_overall = gen_best;
      stall_count = 0;
    } else {
      stall_count++;
    }
    if (stall_count >= params.stall_generations) break;

    // Elitism
    std::vector<std::pair<std::vector<int>, std::vector<double>>> next_gen;
    {
      auto best_it = std::max_element(fitnesses.begin(), fitnesses.end());
      size_t best_idx = std::distance(fitnesses.begin(), best_it);
      next_gen.push_back(population[best_idx]);
    }

    // Tournament selection
    int k = params.tournament_size > 0 ? params.tournament_size : 2;
    std::uniform_int_distribution<size_t> pop_dist(0, population.size() - 1);
    auto pick_parent = [&]() {
      size_t best = pop_dist(rng());
      double best_fit = fitnesses[best];
      for (int i = 1; i < k; ++i) {
        size_t idx = pop_dist(rng());
        if (fitnesses[idx] > best_fit) {
          best = idx;
          best_fit = fitnesses[idx];
        }
      }
      return population[best];
    };

    // Fill rest of population
    while (next_gen.size() < population.size()) {
      auto [p1_int, p1_real] = pick_parent();
      auto [p2_int, p2_real] = pick_parent();

      std::vector<int> c1_int = p1_int, c2_int = p2_int;
      std::vector<double> c1_real = p1_real, c2_real = p2_real;

      // Crossover (discrete)
      if (u01(rng()) < params.crossover_probability) {
        int cut = std::uniform_int_distribution<int>(1, int_vector_size - 1)(rng());
        for (int j = cut; j < int_vector_size; ++j)
          std::swap(c1_int[j], c2_int[j]);
      }

      // Crossover (real-valued uniform)
      if (u01(rng()) < params.crossover_probability) {
        for (int j = 0; j < real_vector_size; ++j) {
          if (u01(rng()) < 0.5)
            std::swap(c1_real[j], c2_real[j]);
        }
      }

      // Mutation (discrete with wrapping)
      for (int j = 0; j < int_vector_size; ++j) {
        if (u01(rng()) < params.mutation_probability) {
          int step = int_step_dist(rng());
          int val = c1_int[j] + step;
          val = min_gene + ((val - min_gene) % gene_range + gene_range) % gene_range;
          c1_int[j] = val;
        }
        if (u01(rng()) < params.mutation_probability) {
          int step = int_step_dist(rng());
          int val = c2_int[j] + step;
          val = min_gene + ((val - min_gene) % gene_range + gene_range) % gene_range;
          c2_int[j] = val;
        }
      }

      // Mutation (real bounded in [0, 1])
      for (int j = 0; j < real_vector_size; ++j) {
        if (u01(rng()) < params.mutation_probability) {
          c1_real[j] = std::clamp(c1_real[j] + mutation_step_dist(rng()), 0.0, 1.0);
        }
        if (u01(rng()) < params.mutation_probability) {
          c2_real[j] = std::clamp(c2_real[j] + mutation_step_dist(rng()), 0.0, 1.0);
        }
      }

      // Check validity of both children
      if (validity(int_vector_size, c1_int.data(), 
                  real_vector_size, c1_real.data())) {
          next_gen.emplace_back(std::move(c1_int), std::move(c1_real));
      }
      
      if (next_gen.size() < population.size() && 
          validity(int_vector_size, c2_int.data(), 
                  real_vector_size, c2_real.data())) {
          next_gen.emplace_back(std::move(c2_int), std::move(c2_real));
      }
    }

    population.swap(next_gen);

    if (params.verbose && gen % 10 == 0) {
      std::cout << "[GA-Mixed] Gen " << gen << " best fitness " << gen_best << "\n";
    }
  }

  // Copy best solution
  double best_fit = -1e12;
  const std::pair<std::vector<int>, std::vector<double>>* best = nullptr;
  for (auto &g : population) {
    double fit = func(int_vector_size, g.first.data(),
                      real_vector_size, g.second.data());
    if (fit > best_fit) {
      best_fit = fit;
      best = &g;
    }
  }

  if (best) {
    std::copy(best->first.begin(), best->first.end(), int_vector);
    std::copy(best->second.begin(), best->second.end(), real_vector);
  }

  // Store optimization results
  last_result.best_fitness = best_fit;
  last_result.generations = params.max_iterations - stall_count;

  auto t1 = Clock::now();
  if (params.verbose) {
    double secs = std::chrono::duration<double>(t1 - t0).count();
    std::cout << "[GA-Mixed] Completed in " << secs
              << "s, best_fitness=" << best_fit << "\n";
  }

  return 0;
}