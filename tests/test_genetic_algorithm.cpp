// tests/test_genetic_algorithm.cpp

#include "CCircuit.h"   // For Circuit class and check_validity
#include "CSimulator.h" // For circuit_performance
#include "Genetic_Algorithm.h"
#include <cmath>
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

// Known "answer" vectors for testing
static const int int_test_answer[21] = {0, 1, 2, 0, 3, 1, 4, 2, 0, 1, 3, 4, 2, 1, 0, 3, 2, 4, 1, 3, 0};
double last_int_error = 0.0;
double last_real_error = 0.0;
static const std::vector<double> real_test_answer = {0.8, 0.4, 0.4, 0.8, 0.0, 0.8, 0.6, 0.3, 0.5, 0.9, 0.2,
                                                     0.7, 0.8, 0.1, 0.4, 0.6, 0.3, 0.5, 0.8, 0.0, 0.7};

// Previous initial vectors from old main()
static const std::vector<int> ga_initial_discrete_n10_vec =
    { // L=20, so n should be (20-1)/2 -> not integer. This was for L=21 if
      // int_test_answer was target
      // The old main had L1 = vector1.size() for this, where vector1 had 20
      // elements.
      // This seems to be a mismatch with int_test_answer that has 21
      // elements.
      // For n=10, L must be 21. The old vector1 had 20 elements.
      // Let's use a slice of int_test_answer or a new proper n=10 vector.
      // For now, let's define it as it was in the old main's vector1 (L=20)
      // and adjust if needed or make a new L=21 vector.
      // Old vector1 (L=20):
      //  1, 0, 3, 2, 1, 0, 4, 3, 2, 1, 0,
      //  3, 4, 2, 1, 0, 3, 2, 1, 4 // (last element 0 was missing to make it
      //  20 from 21 of int_test_answer's pair)
      // Let's use a valid n=10 initial guess (L=21) for consistency in tests
      // aiming for actual circuit performance.
      // Example: feed to u0, u0->P1,T, u1->P1,T ... u9->P1,T
        0, 10, 12, 10, 12, 10, 12, 10, 12, 10, 12, 10, 12, 10, 12, 10, 12, 10, 12, 10, 12};

static const std::vector<double> ga_initial_continuous_L21_vec = { // L=21
    0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 0.1, 0.2};

// Fitness function using actual circuit_performance
// The GA aims to maximize this.
// Note: circuit_performance itself might return negative values (costs >
// revenue).
double circuit_performance_fitness_adapter(int L_discrete, int* discrete_vars)
{
    // This adapter is for the GA's discrete optimization function signature.
    // It doesn't use continuous variables directly in this signature.
    // If your circuit_performance or underlying CCircuit needs beta values that
    // are NOT optimized by GA here, they would need to be handled differently
    // (e.g. fixed default betas, or betas set externally).
    return circuit_performance(L_discrete, discrete_vars);
}

double circuit_performance_mixed_fitness_adapter(int L_discrete, int* discrete_vars, int L_continuous,
                                                 double* continuous_vars)
{
    // This adapter is for the GA's mixed optimization function signature.
    // We need to ensure circuit_performance or the underlying Circuit object can
    // use these continuous_vars (betas). This is a placeholder for how one
    // *might* integrate it. The actual CSimulator/CCircuit API will dictate.

    // Option 1: If circuit_performance can take beta values (not shown in
    // CSimulator.h example) return circuit_performance(L_discrete, discrete_vars,
    // L_continuous, continuous_vars);

    // Option 2: Create a temporary Circuit object, set betas, then calculate
    // performance. This assumes CCircuit has a way to set betas and then a way to
    // get performance that CSimulator typically wraps.
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

// Validity function using CCircuit::check_validity
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
        // params.continuous_mutation_step_size = 0.1; // This field does not exist
        // in Algorithm_Parameters
        params.convergence_threshold = 1e-4;
        params.stall_generations = 20;
    }
};

// Test GA with discrete variables trying to find a known simple valid circuit
// for n=1 Circuit: {0, 1, 3} (Feed u0, u0->P1,T). n=1, P1=1, P2=2, T=3.
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
// This fitness function is for the continuous-only GA optimize signature
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

// New fitness function to match the global real_test_answer (L=21)
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
