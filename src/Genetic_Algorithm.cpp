#include "Genetic_Algorithm.h"
#include "CCircuit.h"
#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <omp.h> // Add OpenMP header
#include <random>
#include <set>
#include <vector>

// Add at the top of the file
static int g_random_seed = -1; // -1 means use random seed

// Add this function to set the seed
void set_random_seed(int seed)
{
    g_random_seed = seed;
}

// Modified rng() function - now properly thread-safe
static std::mt19937& rng()
{
    if (g_random_seed >= 0)
    {
        // Deterministic mode - use thread ID to ensure different seeds per thread
        static thread_local std::mt19937 gen(g_random_seed + omp_get_thread_num());
        return gen;
    }
    else
    {
        // Non-deterministic mode
        static thread_local std::mt19937 gen(std::random_device{}());
        return gen;
    }
}

// Generate a valid circuit based on a template pattern
std::vector<int> generate_valid_circuit_template(int num_units)
{
    const int n = num_units;
    const int vec_size = 2 * n + 1;
    std::vector<int> vec(vec_size);

    // Set feed to unit 0 (most common valid configuration)
    vec[0] = 0;

    // Basic linear flow pattern with some recycling
    for (int i = 0; i < n; i++)
    {
        // For the concentrate stream (high-grade)
        if (i < n - 1)
        {
            // Forward flow to next unit for most units
            vec[2 * i + 1] = i + 1;
        }
        else
        {
            // Last unit sends concentrate to Palusznium product (n)
            vec[2 * i + 1] = n;
        }

        // For the tailings stream
        if (i % 3 == 0)
        {
            // Every 3rd unit sends tailings to Gormanium product
            vec[2 * i + 2] = n + 1;
        }
        else if (i % 3 == 1)
        {
            // Every 3rd+1 unit sends tailings to final tailings
            vec[2 * i + 2] = n + 2;
        }
        else
        {
            // Other units recycle tailings back to unit 0
            vec[2 * i + 2] = 0;
        }
    }

    return vec;
}

// Function to apply random valid variations to a template
std::vector<int> create_varied_circuit(const std::vector<int>& template_vec, int num_units,
                                       std::function<bool(int, int*)> validity_check)
{
    const int n = num_units;
    std::vector<int> result = template_vec;

    // Apply a few random changes
    int num_changes = std::uniform_int_distribution<int>(1, n)(rng());

    // Try multiple times to create a valid variation
    for (int attempt = 0; attempt < 20; attempt++)
    {
        std::vector<int> candidate = template_vec;

        for (int i = 0; i < num_changes; i++)
        {
            // Pick a random position to modify (excluding feed position)
            int pos = std::uniform_int_distribution<int>(1, 2 * n)(rng());

            // Determine which unit this connection belongs to
            int unit_idx = (pos - 1) / 2;

            // Pick a valid destination (any unit or terminal except self)
            std::vector<int> valid_dests;
            for (int dest = 0; dest < n + 3; dest++)
            {
                if (dest != unit_idx)
                { // Avoid self-loop
                    valid_dests.push_back(dest);
                }
            }

            // Shuffle possible destinations
            std::shuffle(valid_dests.begin(), valid_dests.end(), rng());

            // Try each possible destination until we find one that works
            bool found_valid = false;
            for (int dest : valid_dests)
            {
                int old_val = candidate[pos];
                candidate[pos] = dest;

                // Check if both connections from this unit point to the same place
                int other_conn = (pos % 2 == 1) ? pos + 1 : pos - 1;
                if (other_conn < candidate.size() && candidate[other_conn] == dest)
                {
                    candidate[pos] = old_val; // Restore old value
                    continue;                 // Can't have both connections to same place
                }

                // Check if the circuit is valid with this change
                if (validity_check(candidate.size(), candidate.data()))
                {
                    found_valid = true;
                    break;
                }
                else
                {
                    candidate[pos] = old_val; // Restore old value
                }
            }

            if (!found_valid)
            {
                // If we couldn't find a valid change for this position, try another
                continue;
            }
        }

        // Check if the final candidate is valid
        if (validity_check(candidate.size(), candidate.data()))
        {
            return candidate;
        }
    }

    // If all attempts failed, return the original template
    return template_vec;
}

// Generate a diverse population of valid circuits
std::vector<std::vector<int>> generate_initial_population(int population_size, int num_units,
                                                          std::function<bool(int, int*)> validity_check)
{
    std::vector<std::vector<int>> population;
    std::set<std::vector<int>> unique_circuits; // To ensure uniqueness

    // Create base templates
    std::vector<std::vector<int>> templates;

    // Template 1: Linear flow with recycling
    templates.push_back(generate_valid_circuit_template(num_units));

    // Template 2: Alternating product outputs
    auto template2 = generate_valid_circuit_template(num_units);
    for (int i = 0; i < num_units; i++)
    {
        if (i % 2 == 0)
        {
            template2[2 * i + 1] = num_units;     // Even units send concentrate to Palusznium
            template2[2 * i + 2] = num_units + 2; // and tailings to final tailings
        }
        else
        {
            template2[2 * i + 1] = num_units + 1; // Odd units send concentrate to Gormanium
            template2[2 * i + 2] = 0;             // and tailings back to first unit
        }
    }
    if (validity_check(template2.size(), template2.data()))
    {
        templates.push_back(template2);
    }

    // Template 3: Butterfly pattern
    auto template3 = generate_valid_circuit_template(num_units);
    for (int i = 0; i < num_units; i++)
    {
        if (i < num_units / 2)
        {
            template3[2 * i + 1] = i + num_units / 2; // First half feed to second half
            template3[2 * i + 2] = num_units + 2;     // Tailings to final tailings
        }
        else
        {
            template3[2 * i + 1] = num_units; // Second half to products
            template3[2 * i + 2] = num_units + 1;
        }
    }
    if (validity_check(template3.size(), template3.data()))
    {
        templates.push_back(template3);
    }

    // Add templates directly to population
    for (const auto& tmpl : templates)
    {
        population.push_back(tmpl);
        unique_circuits.insert(tmpl);
    }

    // Generate variations until we have enough unique circuits
    int max_attempts = population_size * 10;
    int attempts = 0;

    std::cout << "Generating initial population of valid circuits..." << std::endl;

    while (population.size() < population_size && attempts < max_attempts)
    {
        // Pick a random template
        const auto& tmpl = templates[std::uniform_int_distribution<int>(0, templates.size() - 1)(rng())];

        // Create a variation
        auto candidate = create_varied_circuit(tmpl, num_units, validity_check);

        // Check uniqueness
        if (unique_circuits.find(candidate) == unique_circuits.end())
        {
            population.push_back(candidate);
            unique_circuits.insert(candidate);

            if (population.size() % 10 == 0)
            {
                std::cout << "Generated " << population.size() << " valid circuits" << std::endl;
            }
        }

        attempts++;
    }

    std::cout << "Initial population: " << population.size() << " valid circuits" << std::endl;

    return population;
}

// default validity checks
bool all_true(int iv, int* ivs, int rv, double* rvs)
{
    return true;
}
bool all_true_ints(int iv, int* ivs)
{
    return true;
}
bool all_true_reals(int iv, double* rvs)
{
    return true;
}

// (if not in another file) backing storage + accessor:
static OptimizationResult last_result;
OptimizationResult get_last_optimization_result()
{
    return last_result;
}

// ********************************************************************
// 1) Discrete-only optimize with PARALLEL fitness evaluation
// ********************************************************************
int optimize(int int_vector_size, int* int_vector, std::function<double(int, int*)> func,
             std::function<bool(int, int*)> validity, Algorithm_Parameters params)
{
    using Clock = std::chrono::high_resolution_clock;
    auto t0 = Clock::now();

    // Print OpenMP info
    std::cout << "OpenMP: Using " << omp_get_max_threads() << " threads for parallel fitness evaluation" << std::endl;

    // --- 1. Improved population initialization
    int n_units = (int_vector_size - 1) / 2;
    std::cout << "Initializing population for " << n_units << " units..." << std::endl;

    // Generate valid initial population
    std::vector<std::vector<int>> population = generate_initial_population(params.population_size, n_units, validity);

    // If we couldn't generate enough valid circuits, adjust population size
    if (population.size() < params.population_size)
    {
        std::cout << "Warning: Could only generate " << population.size()
                  << " valid circuits, adjusting population size" << std::endl;
        params.population_size = population.size();
    }

    double best_overall = -1e300;              // best seen so far
    int stall_count = 0;                       // gens since last improvement
    double eps = params.convergence_threshold; // "meaningful" fitness delta
    int max_stall = params.stall_generations;  // allowed idle generations

    // --- 2. Main GA loop
    for (int gen = 0; gen < params.max_iterations; ++gen)
    {
        // 2a) PARALLEL fitness evaluation - THIS IS THE KEY OPTIMIZATION!
        std::vector<double> fitnesses(population.size());

// Parallel fitness evaluation using OpenMP
#pragma omp parallel for schedule(dynamic)
        for (size_t i = 0; i < population.size(); ++i)
        {
            int* gdata = population[i].data();
            if (!validity(int_vector_size, gdata))
            {
                fitnesses[i] = -1e9; // heavy penalty
            }
            else
            {
                fitnesses[i] = func(int_vector_size, gdata);
            }
        }

        double gen_best = *std::max_element(fitnesses.begin(), fitnesses.end());
        if (gen_best > best_overall + eps)
        {
            best_overall = gen_best;
            stall_count = 0; // reset when we see new best
        }
        else
        {
            stall_count++;
        }
        if (stall_count >= max_stall)
        {
            if (params.verbose)
            {
                std::cout << "[GA] No improvement for " << stall_count << " generations—stopping early.\n";
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
        auto pick_parent = [&]()
        {
            size_t best = pop_dist(rng());
            double best_fit = fitnesses[best];
            for (int i = 1; i < k; ++i)
            {
                size_t idx = pop_dist(rng());
                if (fitnesses[idx] > best_fit)
                {
                    best = idx;
                    best_fit = fitnesses[idx];
                }
            }
            return population[best];
        };

        // 2c) Fill rest via selection, crossover, mutation
        std::uniform_real_distribution<double> u01(0.0, 1.0);
        while (next_gen.size() < population.size())
        {
            // – Selection via k-way tournament
            auto p1 = pick_parent();
            auto p2 = pick_parent();

            // – Crossover
            std::vector<int> c1 = p1, c2 = p2;
            if (u01(rng()) < params.crossover_probability)
            {
                // Adaptive crossover points: more early on, fewer later
                double progress = static_cast<double>(gen) / params.max_iterations;
                int max_points = std::min(5, int_vector_size / 2); // limit excessive cuts
                int num_cuts = static_cast<int>((1.0 - progress) * max_points);
                num_cuts = std::max(1, num_cuts); // always at least 1 point

                std::vector<bool> crossover_mask(int_vector_size, false);
                for (int i = 0; i < num_cuts; ++i)
                {
                    int cut = std::uniform_int_distribution<int>(0, int_vector_size - 1)(rng());
                    crossover_mask[cut] = true;
                }

                bool flip = false;
                for (int j = 0; j < int_vector_size; ++j)
                {
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
                std::uniform_int_distribution<int> step_dist(-params.mutation_step_size, params.mutation_step_size);
                for (auto* child : {&c1, &c2})
                {
                    for (int j = 0; j < int_vector_size; ++j)
                    {
                        if (u01(rng()) < params.mutation_probability)
                        {
                            int step = step_dist(rng());
                            int val = (*child)[j] + step;
                            (*child)[j] = min_gene + ((val - min_gene) % range + range) % range;
                        }
                    }
                }

                // 2) Inversion mutation, if enabled
                if (params.use_inversion)
                {
                    // pick two indices a < b
                    std::uniform_int_distribution<int> a_dist(0, int_vector_size - 2);
                    int a = a_dist(rng());
                    std::uniform_int_distribution<int> b_dist(a + 1, int_vector_size - 1);
                    int b = b_dist(rng());

                    // reverse that slice in each child with its own probability
                    if (u01(rng()) < params.inversion_probability)
                    {
                        std::reverse(c1.begin() + a, c1.begin() + b + 1);
                    }
                    if (u01(rng()) < params.inversion_probability)
                    {
                        std::reverse(c2.begin() + a, c2.begin() + b + 1);
                    }
                }
            }

            // Check validity of children and add valid ones
            if (validity(int_vector_size, c1.data()))
            {
                next_gen.push_back(std::move(c1));
            }

            if (next_gen.size() < population.size() && validity(int_vector_size, c2.data()))
            {
                next_gen.push_back(std::move(c2));
            }
        }

        // 2d) Replace population
        population.swap(next_gen);

        if (params.verbose && gen % 10 == 0)
        {
            std::cout << "[GA] Gen " << gen << " best fitness " << *std::max_element(fitnesses.begin(), fitnesses.end())
                      << " (thread utilization: " << omp_get_max_threads() << " cores)" << "\n";
        }
    }

    // --- 3. Write best genome back into int_vector[]
    // (Re-evaluate final fitness to find the winner) - Also parallel!
    double best_fit = -1e12;
    size_t best_idx = 0;
    std::vector<double> final_fitnesses(population.size());

#pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < population.size(); ++i)
    {
        final_fitnesses[i] = func(int_vector_size, population[i].data());
    }

    // Find best (sequential)
    for (size_t i = 0; i < population.size(); ++i)
    {
        if (final_fitnesses[i] > best_fit)
        {
            best_fit = final_fitnesses[i];
            best_idx = i;
        }
    }

    // Copy best solution
    for (int i = 0; i < int_vector_size; ++i)
    {
        int_vector[i] = population[best_idx][i];
    }

    // Store optimization results
    last_result.best_fitness = best_fit;
    last_result.generations = params.max_iterations;

    auto t1 = Clock::now();
    if (params.verbose)
    {
        double secs = std::chrono::duration<double>(t1 - t0).count();
        std::cout << "[GA] Completed in " << secs << "s, best_fitness=" << best_fit << " (using "
                  << omp_get_max_threads() << " parallel threads)" << "\n";
    }

    return 0;
}

// ********************************************************************
// 2) Continuous-only optimize with PARALLEL fitness evaluation
// ********************************************************************

int optimize(int real_vector_size, double* real_vector, std::function<double(int, double*)> func,
             std::function<bool(int, double*)> validity, Algorithm_Parameters params)
{
    using Clock = std::chrono::high_resolution_clock;
    auto t0 = Clock::now();

    std::cout << "OpenMP: Using " << omp_get_max_threads() << " threads for continuous optimization" << std::endl;

    std::uniform_real_distribution<double> dist01(0.0, 1.0);
    std::vector<std::vector<double>> population;

    // --- 1. Initialise population
    population.reserve(params.population_size);
    while (population.size() < params.population_size)
    {
        std::vector<double> genome(real_vector_size);
        for (auto& g : genome)
            g = dist01(rng()); // all β_i in [0,1]
        if (validity(real_vector_size, genome.data()))
            population.push_back(std::move(genome));
    }

    double best_overall = -1e300;
    int stall_count = 0;
    double eps = params.convergence_threshold;
    int max_stall = params.stall_generations;

    for (int gen = 0; gen < params.max_iterations; ++gen)
    {
        // PARALLEL fitness evaluation
        std::vector<double> fitnesses(population.size());

#pragma omp parallel for schedule(dynamic)
        for (size_t i = 0; i < population.size(); ++i)
        {
            fitnesses[i] =
                validity(real_vector_size, population[i].data()) ? func(real_vector_size, population[i].data()) : -1e9;
        }

        double gen_best = *std::max_element(fitnesses.begin(), fitnesses.end());
        if (gen_best > best_overall + eps)
        {
            best_overall = gen_best;
            stall_count = 0;
        }
        else
        {
            stall_count++;
        }

        if (stall_count >= max_stall)
        {
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
        auto pick_parent = [&]()
        {
            size_t best = pop_dist(rng());
            double best_fit = fitnesses[best];
            for (int i = 1; i < k; ++i)
            {
                size_t idx = pop_dist(rng());
                if (fitnesses[idx] > best_fit)
                {
                    best = idx;
                    best_fit = fitnesses[idx];
                }
            }
            return population[best];
        };

        // Crossover + Mutation
        while (next_gen.size() < population.size())
        {
            auto p1 = pick_parent();
            auto p2 = pick_parent();
            std::vector<double> c1 = p1, c2 = p2;

            if (dist01(rng()) < params.crossover_probability)
            {
                for (int j = 0; j < real_vector_size; ++j)
                {
                    if (dist01(rng()) < 0.5)
                        std::swap(c1[j], c2[j]);
                }
            }

            // Mutation
            for (int j = 0; j < real_vector_size; ++j)
            {
                if (dist01(rng()) < params.mutation_probability)
                {
                    double step = dist01(rng()) * params.mutation_step_size;
                    c1[j] = std::clamp(c1[j] + step * (dist01(rng()) < 0.5 ? -1 : 1), 0.0, 1.0);
                }
                if (dist01(rng()) < params.mutation_probability)
                {
                    double step = dist01(rng()) * params.mutation_step_size;
                    c2[j] = std::clamp(c2[j] + step * (dist01(rng()) < 0.5 ? -1 : 1), 0.0, 1.0);
                }
            }

            // Optional: scaling mutation
            if (params.use_scaling_mutation)
            {
                std::uniform_int_distribution<int> idx_dist(0, real_vector_size - 1);
                std::uniform_real_distribution<double> scale_dist(params.scaling_mutation_min,
                                                                  params.scaling_mutation_max);

                // Child 1
                if (dist01(rng()) < params.scaling_mutation_prob)
                {
                    int idx = idx_dist(rng());
                    double factor = scale_dist(rng());
                    c1[idx] = std::clamp(c1[idx] * factor, 0.0, 1.0);
                }

                // Child 2
                if (dist01(rng()) < params.scaling_mutation_prob)
                {
                    int idx = idx_dist(rng());
                    double factor = scale_dist(rng());
                    c2[idx] = std::clamp(c2[idx] * factor, 0.0, 1.0);
                }
            }

            next_gen.push_back(std::move(c1));
            if (next_gen.size() < population.size())
                next_gen.push_back(std::move(c2));
        }

        population.swap(next_gen);

        if (params.verbose && gen % (params.max_iterations / 10) == 0)
        {
            std::cout << "[GA-Real] Gen " << gen << " best fitness " << gen_best
                      << " (parallel threads: " << omp_get_max_threads() << ")" << "\n";
        }
    }

    // PARALLEL final evaluation
    double best_fit = -1e12;
    size_t best_idx = 0;
    std::vector<double> final_fitnesses(population.size());

#pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < population.size(); ++i)
    {
        final_fitnesses[i] = func(real_vector_size, population[i].data());
    }

    // Find best (sequential)
    for (size_t i = 0; i < population.size(); ++i)
    {
        if (final_fitnesses[i] > best_fit)
        {
            best_fit = final_fitnesses[i];
            best_idx = i;
        }
    }

    // Copy best solution
    for (int i = 0; i < real_vector_size; ++i)
    {
        real_vector[i] = population[best_idx][i];
    }

    // Store optimization results
    last_result.best_fitness = best_fit;
    last_result.generations = params.max_iterations - stall_count;

    auto t1 = Clock::now();
    if (params.verbose)
    {
        double secs = std::chrono::duration<double>(t1 - t0).count();
        std::cout << "[GA-Real] Completed in " << secs << "s, best_fitness=" << best_fit << " (using "
                  << omp_get_max_threads() << " parallel threads)" << "\n";
    }

    return 0;
}

// ********************************************************************
// 3) Hybrid optimize with sequential approach but parallel evaluations
// ********************************************************************
int optimize(int int_vector_size, int* int_vector, int real_vector_size, double* real_vector,
             std::function<double(int, int*, int, double*)> hybrid_func,
             std::function<bool(int, int*, int, double*)> hybrid_validity, Algorithm_Parameters params)
{

    std::cout << "OpenMP: Using " << omp_get_max_threads() << " threads for hybrid optimization" << std::endl;

    // Discrete step: optimize only int vector
    auto wrapped_func_int = [&](int n, int* v)
    {
        return hybrid_func(n, v, real_vector_size,
                           real_vector); // current real_vector
    };
    auto wrapped_valid_int = [&](int n, int* v) { return hybrid_validity(n, v, real_vector_size, real_vector); };

    optimize(int_vector_size, int_vector, wrapped_func_int, wrapped_valid_int, params);

    // Continuous step: optimize only real vector
    auto wrapped_func_real = [&](int n, double* r)
    {
        return hybrid_func(int_vector_size, int_vector, n, r); // fixed int_vector
    };
    auto wrapped_valid_real = [&](int n, double* r) { return hybrid_validity(int_vector_size, int_vector, n, r); };

    optimize(real_vector_size, real_vector, wrapped_func_real, wrapped_valid_real, params);

    return 0;
}