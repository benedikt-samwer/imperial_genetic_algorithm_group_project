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
  std::vector<int> population_member(int_vector_size);
  // We'll grow a population of these:
  std::vector<std::vector<int>> population;

  // --- 1. Initialize population
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

    // TODO: check for convergence/stall_generations here

    // 2b) Elitism: copy best genome to next generation
    std::vector<std::vector<int>> next_gen;
    {
      auto best_it = std::max_element(fitnesses.begin(), fitnesses.end());
      size_t best_idx = std::distance(fitnesses.begin(), best_it);
      next_gen.push_back(population[best_idx]);
    }

    // 2c) Fill rest via selection, crossover, mutation
    std::uniform_real_distribution<double> u01(0.0, 1.0);
    while (next_gen.size() < population.size()) {
      // – Selection (e.g., tournament or roulette) → parents p1, p2
      std::vector<int> p1 = population[0]; // TODO
      std::vector<int> p2 = population[1]; // TODO

      // – Crossover
      std::vector<int> c1 = p1, c2 = p2;
      if (u01(rng()) < params.crossover_probability) {
        int cut =
            std::uniform_int_distribution<int>(1, int_vector_size - 1)(rng());
        for (int j = cut; j < int_vector_size; ++j)
          std::swap(c1[j], c2[j]);
      }

      // – Mutation
      for (int j = 0; j < int_vector_size; ++j) {
        if (u01(rng()) < params.mutation_probability) {
          // TODO: apply step +/- params.mutation_step_size, wrap if needed
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

// ********************************************************************
// 2) Continuous-only optimize (stub)
// ********************************************************************
int optimize(int real_vector_size, double *real_vector,
             std::function<double(int, double *)> func,
             std::function<bool(int, double *)> validity,
             Algorithm_Parameters params) {
  // TODO: implement real-valued GA (or templatize above)
  return -1;
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