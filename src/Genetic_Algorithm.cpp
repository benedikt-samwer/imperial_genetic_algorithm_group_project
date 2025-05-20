// src/genetic_algorithm.cpp

#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <random>
#include <vector>
// src/genetic_algorithm.cpp

#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <random>
#include <vector>

#include "Genetic_Algorithm.h"

// Helper RNG
static std::mt19937 &rng() {
  static thread_local std::mt19937 gen(std::random_device{}());
  return gen;
}

// default validity checks
bool all_true(int iv, int *ivs, int rv, double *rvs) { return true; }
bool all_true_ints(int iv, int *ivs) { return true; }
bool all_true_reals(int rv, double *rvs) { return true; }

// (if not in another file) backing storage + accessor:
static OptimizationResult last_result;
OptimizationResult get_last_optimization_result() { return last_result; }

// ********************************************************************
// 1) Discrete-only optimize
// ********************************************************************
int optimize(int int_vector_size, int *int_vector,
             std::function<double(int, int *)> func,
             std::function<bool(int, int *)> validity,
             Algorithm_Parameters params) {
  using Clock = std::chrono::high_resolution_clock;
  auto t0 = Clock::now();

  // --- 1. Wrap C array to std::vector
  // std::vector<int> population_member(int_vector_size); CURRENTLY UNUSED
  // We'll grow a population of these:
  std::vector<std::vector<int>> population;

  // --- 1. Initialise population
  int n_units = (int_vector_size - 1) / 2;
  int min_gene = -3;
  int max_gene = n_units + 2;
  std::uniform_int_distribution<int> gene_dist(min_gene, max_gene);

  population.clear();
  population.reserve(params.population_size);

  while (population.size() < params.population_size) {
    std::vector<int> genome(int_vector_size);
    for (int &g : genome)
      g = gene_dist(rng());
    if (validity(int_vector_size, genome.data()))
      population.push_back(std::move(genome));
  }

  double best_overall = -1e300;              // best seen so far
  int stall_count = 0;                       // gens since last improvement
  double eps = params.convergence_threshold; // “meaningful” fitness delta
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
          int max_points = std::min(5, int_vector_size / 2); // limit excessive cuts
          int num_cuts = static_cast<int>((1.0 - progress) * max_points);
          num_cuts = std::max(1, num_cuts); // always at least 1 point

          std::vector<bool> crossover_mask(int_vector_size, false);
          for (int i = 0; i < num_cuts; ++i) {
              int cut = std::uniform_int_distribution<int>(0, int_vector_size - 1)(rng());
              crossover_mask[cut] = true;
          }

          bool flip = false;
          for (int j = 0; j < int_vector_size; ++j) {
              if (crossover_mask[j]) flip = !flip;
              if (flip) std::swap(c1[j], c2[j]);
          }
      }
      // – Mutation (step‐size ± params.mutation_step_size, wrapping)
      {
        int range = max_gene - min_gene + 1;
        std::uniform_int_distribution<int> step_dist(-params.mutation_step_size,
                                                     params.mutation_step_size);

        for (int j = 0; j < int_vector_size; ++j) {
          if (u01(rng()) < params.mutation_probability) {
            // pick a random step in [–step_size…+step_size]
            int step = step_dist(rng());
            int val = c1[j] + step;
            // wrap into [min_gene…max_gene]
            val = min_gene + ((val - min_gene) % range + range) % range;
            c1[j] = val;
          }
        }

        // repeat for the second child, c2:
        for (int j = 0; j < int_vector_size; ++j) {
          if (u01(rng()) < params.mutation_probability) {
            int step = step_dist(rng());
            int val = c2[j] + step;
            val = min_gene + ((val - min_gene) % range + range) % range;
            c2[j] = val;
          }
        }
      }

      next_gen.push_back(std::move(c1));
      if (next_gen.size() < population.size())
        next_gen.push_back(std::move(c2));
    }

    // 2d) Replace population
    population.swap(next_gen);

    if (params.verbose && gen % (params.max_iterations / 10) == 0) {
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

  // --- 4. (Optional) store statistics in a global/result struct...

  auto t1 = Clock::now();
  if (params.verbose) {
    double secs = std::chrono::duration<double>(t1 - t0).count();
    std::cout << "[GA] Completed in " << secs << "s, best_fitness=" << best_fit
              << "\n";
  }

  return 0;
}

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
      g = dist01(rng());  // all β_i in [0,1]
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
      fitnesses[i] = validity(real_vector_size, population[i].data()) ?
        func(real_vector_size, population[i].data()) : -1e9;
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
        std::cout << "[GA-Real] No improvement for " << stall_count << " generations — stopping.\n";
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
          if (dist01(rng()) < 0.5) std::swap(c1[j], c2[j]);
        }
      }

      // Mutation
      for (int j = 0; j < real_vector_size; ++j) {
        if (dist01(rng()) < params.mutation_probability) {
          double step = dist01(rng()) * params.mutation_step_size;
          c1[j] = std::clamp(c1[j] + step * (dist01(rng()) < 0.5 ? -1 : 1), 0.0, 1.0);
        }
        if (dist01(rng()) < params.mutation_probability) {
          double step = dist01(rng()) * params.mutation_step_size;
          c2[j] = std::clamp(c2[j] + step * (dist01(rng()) < 0.5 ? -1 : 1), 0.0, 1.0);
        }
      }

      next_gen.push_back(std::move(c1));
      if (next_gen.size() < population.size())
        next_gen.push_back(std::move(c2));
    }

    population.swap(next_gen);

    if (params.verbose && gen % (params.max_iterations / 10) == 0) {
      std::cout << "[GA-Real] Gen " << gen << " best fitness " << gen_best << "\n";
    }
  }

  // Copy best solution
  double best_fit = -1e12;
  std::vector<double>* best_genome = nullptr;
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

  auto t1 = Clock::now();
  if (params.verbose) {
    double secs = std::chrono::duration<double>(t1 - t0).count();
    std::cout << "[GA-Real] Completed in " << secs << "s, best_fitness=" << best_fit << "\n";
  }

  return 0;
}
// ********************************************************************
// 3) Mixed discrete-continuous optimize (stub)
// ********************************************************************
int optimize(int int_vector_size, int *int_vector, int real_vector_size,
             double *real_vector,
             std::function<double(int, int *, int, double *)> func,
             std::function<bool(int, int *, int, double *)> validity,
             Algorithm_Parameters params) {
  // TODO: implement combined GA
  return -1;
}
