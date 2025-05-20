#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <random>

#include "constants.h"
#include "circuit_vector.h"
#include "CCircuit.h"
#include "Genetic_Algorithm.h"

// forward declare this
std::vector<std::vector<int>> initialize_population(int population_size, int num_units, int max_attempts);

bool all_true(int int_vector_size, int * int_vector, int real_vector_size, double * real_vector) {
  return true;
}

bool all_true_ints(int int_vector_size, int * int_vector) {
  return true;
}

bool all_true_reals(int real_vector_size, double * real_vector) {
  return true;
}

int optimize(int int_vector_size, int* int_vector,
             std::function<double(int, int*)> func,
             std::function<bool(int, int*)> validity,
             Algorithm_Parameters algorithm_parameters) {

    const int pop_size = algorithm_parameters.population_size;
    const int max_gens = algorithm_parameters.max_iterations;
    const double mutation_rate = algorithm_parameters.mutation_probability;
    const double crossover_rate = algorithm_parameters.crossover_probability;
    const int elite_count = algorithm_parameters.elite_count;

    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<> prob(0.0, 1.0);
    std::uniform_int_distribution<> value_dist(0, 4);  // range of values for test vectors

    // 1. Generate initial population
    std::vector<std::vector<int>> population;
    while ((int)population.size() < pop_size) {
        std::vector<int> candidate(int_vector_size);
        for (int i = 0; i < int_vector_size; ++i)
            candidate[i] = value_dist(rng);

        if (validity(int_vector_size, candidate.data()))
            population.push_back(candidate);
    }

    std::vector<double> fitness(pop_size);

    for (int gen = 0; gen < max_gens; ++gen) {
        // 2. Evaluate fitness
        for (int i = 0; i < pop_size; ++i)
            fitness[i] = -func(int_vector_size, population[i].data()); // negative because we minimize

        // 3. Find elite (best) solution
        auto best_it = std::max_element(fitness.begin(), fitness.end());
        int best_index = std::distance(fitness.begin(), best_it);

        std::vector<std::vector<int>> new_population;
        new_population.push_back(population[best_index]); // elitism

        // 4. Generate new individuals
        while ((int)new_population.size() < pop_size) {
            // Select parents (roulette wheel selection)
            int p1 = std::uniform_int_distribution<>(0, pop_size - 1)(rng);
            int p2 = std::uniform_int_distribution<>(0, pop_size - 1)(rng);

            std::vector<int> child1 = population[p1];
            std::vector<int> child2 = population[p2];

            // Crossover
            if (prob(rng) < crossover_rate) {
                int point = std::uniform_int_distribution<>(1, int_vector_size - 1)(rng);
                for (int i = 0; i < point; ++i)
                    std::swap(child1[i], child2[i]);
            }

            // Mutation
            for (int i = 0; i < int_vector_size; ++i) {
                if (prob(rng) < mutation_rate) {
                    child1[i] = value_dist(rng);
                }
                if (prob(rng) < mutation_rate) {
                    child2[i] = value_dist(rng);
                }
            }

            if (validity(int_vector_size, child1.data()))
                new_population.push_back(child1);
            if ((int)new_population.size() < pop_size && validity(int_vector_size, child2.data()))
                new_population.push_back(child2);
        }

        population = new_population;
    }

    // Copy final best to output vector
    auto best_it = std::max_element(fitness.begin(), fitness.end());
    int best_index = std::distance(fitness.begin(), best_it);
    std::copy(population[best_index].begin(), population[best_index].end(), int_vector);

    return 0;
}


int optimize(int real_vector_size, double* real_vector,
             std::function<double(int, double*)> func,
             std::function<bool(int, double*)> validity,
             Algorithm_Parameters algorithm_parameters) {

    const int pop_size = algorithm_parameters.population_size;
    const int max_gens = algorithm_parameters.max_iterations;
    const double mutation_rate = algorithm_parameters.mutation_probability;
    const double crossover_rate = algorithm_parameters.crossover_probability;

    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<> prob(0.0, 1.0);
    std::uniform_real_distribution<> value_dist(0.0, 5.0);

    // 1. Generate initial population
    std::vector<std::vector<double>> population;
    while ((int)population.size() < pop_size) {
        std::vector<double> candidate(real_vector_size);
        for (int i = 0; i < real_vector_size; ++i)
            candidate[i] = value_dist(rng);

        if (validity(real_vector_size, candidate.data()))
            population.push_back(candidate);
    }

    std::vector<double> fitness(pop_size);

    for (int gen = 0; gen < max_gens; ++gen) {
        for (int i = 0; i < pop_size; ++i)
            fitness[i] = -func(real_vector_size, population[i].data());

        auto best_it = std::max_element(fitness.begin(), fitness.end());
        int best_index = std::distance(fitness.begin(), best_it);

        std::vector<std::vector<double>> new_population;
        new_population.push_back(population[best_index]); // elitism

        while ((int)new_population.size() < pop_size) {
            int p1 = std::uniform_int_distribution<>(0, pop_size - 1)(rng);
            int p2 = std::uniform_int_distribution<>(0, pop_size - 1)(rng);

            std::vector<double> child1 = population[p1];
            std::vector<double> child2 = population[p2];

            // Crossover
            if (prob(rng) < crossover_rate) {
                int point = std::uniform_int_distribution<>(1, real_vector_size - 1)(rng);
                for (int i = 0; i < point; ++i)
                    std::swap(child1[i], child2[i]);
            }

            // Mutation
            for (int i = 0; i < real_vector_size; ++i) {
                if (prob(rng) < mutation_rate) {
                    child1[i] = value_dist(rng);
                }
                if (prob(rng) < mutation_rate) {
                    child2[i] = value_dist(rng);
                }
            }

            if (validity(real_vector_size, child1.data()))
                new_population.push_back(child1);
            if ((int)new_population.size() < pop_size && validity(real_vector_size, child2.data()))
                new_population.push_back(child2);
        }

        population = new_population;
    }

    // Copy final best to output
    auto best_it = std::max_element(fitness.begin(), fitness.end());
    int best_index = std::distance(fitness.begin(), best_it);
    std::copy(population[best_index].begin(), population[best_index].end(), real_vector);

    return 0;
}

/*int optimize(int int_vector_size, int * int_vector, int real_vector_size, double * real_vector,
                  double (&func) (int, int*, int, double*),
                           bool (&validity) (int, int*, int, double*),
                           struct Algorithm_Parameters algorithm_parameters) {

  
  // Combined optimization function for both int and real vectors
  // Run the genetic algorithm process on both the int and real parts

  return 0;

}*/

// overloads (delete if not needed)






// additional variables, classes and functions as needed.

std::vector<std::vector<int>> initialize_population(int population_size, int num_units, int max_attempts) {
    std::vector<std::vector<int>> population;
    int vector_size = 2 * num_units + 1;

    int attempts = 0;

    while ((int)population.size() < population_size && attempts < max_attempts) {
        attempts++;

        // Create a randomized circuit vector
        CircuitVector candidate(num_units);
        candidate.randomize();

        // Convert to raw data
        const std::vector<int>& data = candidate.get_data();
        const int* raw_vector = data.data();

        // Validate it
        if (Circuit::check_validity(vector_size, raw_vector)) {
            population.push_back(data);
        }
    }

    if ((int)population.size() < population_size) {
        std::cerr << "Warning: Only initialized " << population.size()
                  << " valid individuals out of requested " << population_size << std::endl;
    }

    return population;
}