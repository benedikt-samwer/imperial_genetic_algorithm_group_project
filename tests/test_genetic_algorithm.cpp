/**
 * @file test_genetic_algorithm.cpp
 * @brief Unit tests for the Genetic Algorithm module
 *
 * This file contains unit tests for the Genetic Algorithm implementation
 * using the Google Test framework. The tests cover various aspects of the
 * algorithm, including optimization of discrete and continuous variables,
 * validity checks, and performance evaluation.
 */
#include "CCircuit.h"   // For Circuit class and check_validity
#include "CSimulator.h" // For circuit_performance
#include "Genetic_Algorithm.h"
#include <cmath>
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

// Test parameters
static const int int_test_answer[21] = {0, 1, 2, 0, 3, 1, 4, 2, 0, 1, 3, 4, 2, 1, 0, 3, 2, 4, 1, 3, 0};
double last_int_error = 0.0;
double last_real_error = 0.0;
static const std::vector<double> real_test_answer = {0.8, 0.4, 0.4, 0.8, 0.0, 0.8, 0.6, 0.3, 0.5, 0.9, 0.2,
                                                     0.7, 0.8, 0.1, 0.4, 0.6, 0.3, 0.5, 0.8, 0.0, 0.7};

// Initial guess for discrete variables (L=10)
static const std::vector<int> ga_initial_discrete_n10_vec = {0,  10, 12, 10, 12, 10, 12, 10, 12, 10, 12,
                                                             10, 12, 10, 12, 10, 12, 10, 12, 10, 12};

// Initial guess for continuous variables (L=21)
static const std::vector<double> ga_initial_continuous_L21_vec = { // L=21
    0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 0.1, 0.2};

/**
 * @brief Fitness function for circuit performance optimization.
 *
 * This function evaluates the performance of a circuit based on its
 * configuration and returns a fitness score. The fitness score is
 * calculated as the negative of the circuit performance, as the
 * optimization algorithm aims to maximize the fitness score.
 *
 * @param L_discrete The length of the discrete variable array.
 * @param discrete_vars The array of discrete variables representing the
 * circuit configuration.
 *
 * @return The fitness score of the circuit configuration.
 *
 */
double circuit_performance_fitness_adapter(int L_discrete, int* discrete_vars)
{
    return circuit_performance(L_discrete, discrete_vars);
}

/**
 * @brief Fitness function for circuit performance optimization with
 * continuous variables.
 *
 * This function evaluates the performance of a circuit based on its
 * configuration and continuous variables (e.g., beta values) and
 * returns a fitness score. The fitness score is calculated as
 * the negative of the circuit performance, as the optimization
 * algorithm aims to maximize the fitness score.
 *
 * @param L_discrete The length of the discrete variable array.
 * @param discrete_vars The array of discrete variables representing the
 * circuit configuration.
 * @param L_continuous The length of the continuous variable array.
 * @param continuous_vars The array of continuous variables (e.g., beta
 * values).
 *
 * @return The fitness score of the circuit configuration.
 *
 */
double circuit_performance_mixed_fitness_adapter(int L_discrete, int* discrete_vars, int L_continuous,
                                                 double* continuous_vars)
{
    int n_units = (L_discrete - 1) / 2;
    if (L_discrete <= 0 || (2 * n_units + 1) != L_discrete || (L_continuous > 0 && L_continuous != n_units))
    {
        return -1e18; // Invalid parameters
    }
    Circuit temp_circuit(n_units);
    // The initialize_from_vector is crucial for setting up the circuit structure.
    // We must ensure it's called before trying to run mass balance or get
    // economic value. The check_validity function internally calls
    // initialize_from_vector as part of its process if the structure is basic,
    // but for performance, we need a fully initialized and mass-balanced circuit.

    // A robust way: initialize, set betas (if any), run mass balance, then get
    // economic value. The current circuit_performance(L, v) likely does this
    // internally without exposing beta parameters. For a mixed GA,
    // circuit_performance would need to accept beta values. Lacking that, this
    // fitness function cannot easily use the optimized continuous_vars with the
    // existing circuit_performance.

    // Fallback to discrete-only performance if circuit_performance cannot accept
    // continuous_vars: This makes the continuous part of GA useless for *this
    // specific fitness function*. To truly test mixed optimization,
    // circuit_performance needs to use the continuous variables. For the purpose
    // of making the code compile and run with GA, we'll simulate this. In a real
    // scenario, this is where CSimulator.h/CCircuit.h would need modification.

    // SIMULATED: Pretend we use continuous_vars to slightly modify performance.
    // This is NOT how it should be in production but allows testing GA mechanics.
    double base_performance = circuit_performance(L_discrete, discrete_vars);
    double continuous_penalty = 0.0;
    if (L_continuous > 0)
    {
        for (int i = 0; i < L_continuous; ++i)
        {
            // Example: penalize if beta values are too far from 0.5
            continuous_penalty += (continuous_vars[i] - 0.5) * (continuous_vars[i] - 0.5);
        }
    }
    return base_performance - continuous_penalty * 1000; // Apply some penalty
}

/**
 * @brief Validity function for discrete circuit configurations.
 *
 * This function checks if the given discrete circuit configuration is valid
 * based on the number of units and their connections. It returns true if the
 * configuration is valid, false otherwise.
 *
 * @param L_discrete The length of the discrete variable array.
 * @param discrete_vars The array of discrete variables representing the
 * circuit configuration.
 *
 * @return True if the configuration is valid, false otherwise.
 */
bool actual_validity_discrete_adapter(int L_discrete, int* discrete_vars)
{
    if (L_discrete <= 0)
        return false;
    int n_units = (L_discrete - 1) / 2;
    if ((2 * n_units + 1) != L_discrete)
        return false;
    Circuit temp_circuit(n_units);
    return temp_circuit.check_validity(L_discrete, discrete_vars);
}

/**
 * @brief Validity function for mixed discrete-continuous circuit
 * configurations.
 *
 * This function checks if the given mixed circuit configuration is valid
 * based on the number of units, their connections, and continuous variables.
 * It returns true if the configuration is valid, false otherwise.
 *
 * @param L_discrete The length of the discrete variable array.
 * @param discrete_vars The array of discrete variables representing the
 * circuit configuration.
 * @param L_continuous The length of the continuous variable array.
 * @param continuous_vars The array of continuous variables (e.g., beta
 * values).
 *
 * @return True if the configuration is valid, false otherwise.
 */
bool actual_validity_mixed_adapter(int L_discrete, int* discrete_vars, int L_continuous, double* continuous_vars)
{
    if (L_discrete <= 0)
        return false;
    int n_units = (L_discrete - 1) / 2;
    if ((2 * n_units + 1) != L_discrete)
        return false;

    Circuit temp_circuit(n_units);
    if (L_continuous > 0 && L_continuous == n_units)
    {
        return temp_circuit.check_validity(L_discrete, discrete_vars, L_continuous, continuous_vars);
    }
    else if (L_continuous == 0)
    {
        return temp_circuit.check_validity(L_discrete, discrete_vars);
    }
    return false; // Mismatch in continuous var length if present
}

/**
 * @brief Test fixture for Genetic Algorithm tests.
 *
 * This class sets up the parameters and environment for running the
 * Genetic Algorithm tests. It inherits from the Google Test framework's
 * Test class.
 */
class GeneticAlgorithmTest : public ::testing::Test
{
protected:
    Algorithm_Parameters params;
    const double EPSILON = 0.15; // Relaxed epsilon for GA performance tests

    void SetUp() override
    {
        params = DEFAULT_ALGORITHM_PARAMETERS;
        params.verbose = false;
        params.max_iterations = 50;
        params.population_size = 50;
        params.tournament_size = 2;
        params.crossover_probability = 0.8;
        params.mutation_probability = 0.1;
        params.mutation_step_size = 1;
        params.convergence_threshold = 1e-4;
        params.stall_generations = 20;
    }
};

/**
 * @brief Test for optimizing a simple valid discrete circuit.
 *
 * This test checks if the Genetic Algorithm can successfully optimize
 * a simple valid discrete circuit configuration. It uses a known
 * valid circuit and verifies that the optimization process converges
 * to a valid solution.
 *
 * @return void
 */
TEST_F(GeneticAlgorithmTest, OptimizeSimpleValidDiscreteCircuit)
{
    const int n_units = 1;
    const int L_discrete = 2 * n_units + 1;
    std::vector<int> initial_guess(L_discrete,
                                   0); // Start with all zeros or random
    // Example: initial_guess = {0,0,0}

    // We use the actual_validity_function and circuit_fitness_function
    int status = optimize(L_discrete, initial_guess.data(), circuit_performance_fitness_adapter,
                          actual_validity_discrete_adapter, params);

    ASSERT_EQ(status, 0) << "Optimization failed to complete successfully.";

    OptimizationResult result = get_last_optimization_result();
    ASSERT_GT(result.best_fitness, -1e9) << "Fitness should be better than a very small number for a valid "
                                            "circuit.";

    std::cout << "Discrete GA - Best Fitness: " << result.best_fitness << " after " << result.generations
              << " generations." << std::endl;
    std::cout << "Best discrete genome: ";
    for (int i = 0; i < L_discrete; ++i)
    {
        std::cout << initial_guess[i] << (i == L_discrete - 1 ? "" : ", ");
    }
    std::cout << std::endl;

    // Check if the found solution is valid
    Circuit c_final(n_units);
    ASSERT_TRUE(c_final.check_validity(L_discrete, initial_guess.data())) << "GA found an invalid solution.";
    // Further checks could compare performance to a known optimum if available.
}

// Test GA with continuous variables (e.g., beta values for unit volumes)
// We need a fitness function that primarily depends on these continuous values.
// For simplicity, let's use a placeholder fitness until a clear example is
// available. The real_test_answer and real_test_function from original can be
// adapted.
static const std::vector<double> target_beta_values_for_cont_test = {0.5, 0.6}; // For n=2 example

// This fitness function is a simple sum of squares error to a target vector.
double simple_continuous_fitness_adapter(int L_continuous, double* continuous_vars)
{
    if (L_continuous != static_cast<int>(target_beta_values_for_cont_test.size()))
        return -1e18;
    double sum_sq_error = 0;
    for (int i = 0; i < L_continuous; ++i)
    {
        double diff = continuous_vars[i] - target_beta_values_for_cont_test[i];
        sum_sq_error += diff * diff;
    }
    return -sum_sq_error; // GA maximizes, so negative error
}
// This validity function is for the continuous-only GA optimize signature
bool dummy_validity_continuous_adapter(int L_continuous, double* cv)
{
    // Add any specific checks for continuous variables if needed, e.g. range
    // [0,1]
    for (int i = 0; i < L_continuous; ++i)
    {
        if (cv[i] < 0.0 || cv[i] > 1.0)
            return false;
    }
    return true;
}

/**
 * @brief Test for optimizing simple continuous variables using the
 * Genetic Algorithm.
 */
TEST_F(GeneticAlgorithmTest, OptimizeSimpleContinuousVariables)
{
    const int L_continuous = target_beta_values_for_cont_test.size();
    std::vector<double> initial_continuous_guess(L_continuous, 0.1);

    int status = optimize(L_continuous, initial_continuous_guess.data(), simple_continuous_fitness_adapter,
                          dummy_validity_continuous_adapter, params);

    ASSERT_EQ(status, 0) << "Optimization failed (continuous variables).";
    OptimizationResult result = get_last_optimization_result();
    ASSERT_NEAR(result.best_fitness, 0.0, EPSILON * EPSILON * L_continuous)
        << "Continuous GA did not converge to target fitness.";

    std::cout << "Continuous GA - Best Fitness: " << result.best_fitness << " after " << result.generations
              << " generations." << std::endl;
    std::cout << "Best continuous genome: ";
    for (int i = 0; i < L_continuous; ++i)
    {
        std::cout << initial_continuous_guess[i] << (i == L_continuous - 1 ? "" : ", ");
        ASSERT_NEAR(initial_continuous_guess[i], target_beta_values_for_cont_test[i], EPSILON);
    }
    std::cout << std::endl;
}

/**
 * @brief Fitness function for matching the real test answer.
 *
 * This function evaluates the performance of a circuit based on its
 * configuration and returns a fitness score. The fitness score is
 * calculated as the negative of the squared error from the real test
 * answer, as the optimization algorithm aims to minimize this error.
 *
 * @param L_continuous The length of the continuous variable array.
 * @param continuous_vars The array of continuous variables representing the
 * circuit configuration.
 *
 * @return The fitness score of the circuit configuration.
 */
double match_real_test_answer_fitness_adapter(int L_continuous, double* continuous_vars)
{
    if (L_continuous != static_cast<int>(real_test_answer.size()))
        return -1e18; // Ensure correct length
    double sum_sq_error = 0;
    for (int i = 0; i < L_continuous; ++i)
    {
        double diff = continuous_vars[i] - real_test_answer[i];
        sum_sq_error += diff * diff;
    }
    return -sum_sq_error; // GA maximizes, so negative error
}

/**
 * @brief Test for optimizing continuous variables to match the real test
 * answer.
 */
TEST_F(GeneticAlgorithmTest, OptimizeContinuousToMatchRealTestAnswerL21)
{
    const int L_continuous = real_test_answer.size();                  // Should be 21
    std::vector<double> initial_guess = ga_initial_continuous_L21_vec; // Use the L21 initial vector
    ASSERT_EQ(initial_guess.size(), L_continuous);

    Algorithm_Parameters current_params = params;
    current_params.max_iterations = 1000;
    current_params.population_size = 500;
    current_params.mutation_probability = 0.02;
    current_params.mutation_step_size = 0.05;
    current_params.convergence_threshold = 1e-6;
    current_params.stall_generations = 50;

    int status = optimize(L_continuous, initial_guess.data(), match_real_test_answer_fitness_adapter,
                          dummy_validity_continuous_adapter, // Basic 0-1 range check
                          current_params);

    ASSERT_EQ(status, 0) << "Optimization failed (continuous variables to match "
                            "real_test_answer).";
    OptimizationResult result = get_last_optimization_result();
    ASSERT_NEAR(result.best_fitness, 0.0, EPSILON * EPSILON * L_continuous)
        << "Continuous GA did not converge to target fitness for "
           "real_test_answer.";

    std::cout << "Continuous GA (match real_test_answer L21) - Best Fitness: " << result.best_fitness << " after "
              << result.generations << " generations." << std::endl;
    std::cout << "Best continuous genome: ";
    for (int i = 0; i < L_continuous; ++i)
    {
        std::cout << initial_guess[i] << (i == L_continuous - 1 ? "" : ", ");
        ASSERT_NEAR(initial_guess[i], real_test_answer[i], EPSILON);
    }
    std::cout << std::endl;
}

/**
 * @brief Test for optimizing discrete variables for a circuit with N=10.
 *
 * This test checks if the Genetic Algorithm can successfully optimize
 * a discrete circuit configuration with 10 units. It verifies that the
 * optimization process converges to a valid solution.
 */
TEST_F(GeneticAlgorithmTest, OptimizeDiscreteWithN10Circuit)
{
    const int n_units = 10;
    const int L_discrete = 2 * n_units + 1; // Should be 21
    std::vector<int> initial_guess = ga_initial_discrete_n10_vec;
    ASSERT_EQ(initial_guess.size(), L_discrete);

    params.max_iterations = 150; // n=10 is more complex
    params.population_size = 150;

    int status = optimize(L_discrete, initial_guess.data(), circuit_performance_fitness_adapter,
                          actual_validity_discrete_adapter, params);

    ASSERT_EQ(status, 0) << "Optimization failed for N10 discrete circuit.";

    OptimizationResult result = get_last_optimization_result();
    ASSERT_GT(result.best_fitness, -1e9) << "Fitness for N10 circuit too low.";

    std::cout << "Discrete GA (N=10) - Best Fitness: " << result.best_fitness << " after " << result.generations
              << " generations." << std::endl;
    std::cout << "Best discrete genome (N=10): ";
    for (int i = 0; i < L_discrete; ++i)
    {
        std::cout << initial_guess[i] << (i == L_discrete - 1 ? "" : ", ");
    }
    std::cout << std::endl;

    Circuit c_final(n_units);
    ASSERT_TRUE(c_final.check_validity(L_discrete, initial_guess.data())) << "GA found an invalid N10 solution.";
}

/**
 * @brief Test for optimizing mixed discrete-continuous variables for a
 * circuit with N=10.
 *
 * This test checks if the Genetic Algorithm can successfully optimize
 * a mixed circuit configuration with 10 units. It verifies that the
 * optimization process converges to a valid solution.
 */
TEST_F(GeneticAlgorithmTest, OptimizeMixedWithN10CircuitAndL10Betas)
{
    const int n_units = 10;
    const int L_discrete = 2 * n_units + 1; // 21
    const int L_continuous = n_units;       // 10 beta values

    std::vector<int> initial_discrete_guess = ga_initial_discrete_n10_vec;
    ASSERT_EQ(initial_discrete_guess.size(), L_discrete);
    std::vector<double> initial_continuous_guess(L_continuous,
                                                 0.5); // Initial betas at 0.5

    params.max_iterations = 200;
    params.population_size = 200;

    int status = optimize(L_discrete, initial_discrete_guess.data(), L_continuous, initial_continuous_guess.data(),
                          circuit_performance_mixed_fitness_adapter, actual_validity_mixed_adapter, params);

    ASSERT_EQ(status, 0) << "Mixed N10 optimization failed.";
    OptimizationResult result = get_last_optimization_result();
    ASSERT_GT(result.best_fitness, -1e9) << "Mixed N10 GA fitness too low.";

    std::cout << "Mixed GA (N=10) - Best Fitness: " << result.best_fitness << " after " << result.generations
              << " generations." << std::endl;
    std::cout << "Best discrete genome (N=10): ";
    for (int i = 0; i < L_discrete; ++i)
    {
        std::cout << initial_discrete_guess[i] << (i == L_discrete - 1 ? "" : ", ");
    }
    std::cout << "\nBest continuous genome (N=10 Betas): ";
    for (int i = 0; i < L_continuous; ++i)
    {
        std::cout << initial_continuous_guess[i] << (i == L_continuous - 1 ? "" : ", ");
        ASSERT_GE(initial_continuous_guess[i], 0.0);
        ASSERT_LE(initial_continuous_guess[i], 1.0);
    }
    std::cout << std::endl;

    Circuit final_circuit(n_units);
    bool is_valid = final_circuit.check_validity(L_discrete, initial_discrete_guess.data(), L_continuous,
                                                 initial_continuous_guess.data());
    ASSERT_TRUE(is_valid) << "Mixed GA N10 found an invalid final solution.";
}
