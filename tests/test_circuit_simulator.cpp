/**
 * @file test_circuit_simulator.cpp
 * @brief Unit tests for the circuit simulator.
 * 
 * This file contains unit tests for the circuit simulator, including
 * performance tests and validity checks.
 * 
 */
#include <cmath>
#include <iostream>

#include "CSimulator.h"

#include <gtest/gtest.h>
#include <string>
#include <utility>
#include <vector>

/**
 * @brief Class for testing the circuit simulator.
 * 
 * This class contains unit tests for the circuit simulator, including
 * performance tests and validity checks.
 */
class CircuitSimulatorTest : public ::testing::Test
{
protected:
    // Helper function to run a test case and check performance
    // We expect a certain performance or at least a non-zero/non-NaN value
    // For specific known circuits, we might have expected values.
    void run_performance_test(const std::vector<int>& circuit_vector, double expected_performance = -1.0,
                              double tolerance = 1e-2)
    {
        double performance =
            circuit_performance(static_cast<int>(circuit_vector.size()), const_cast<int*>(circuit_vector.data()));

        // Basic checks for any circuit performance call
        ASSERT_FALSE(std::isnan(performance)) << "Performance should not be NaN.";
        // Depending on the circuit, performance could be negative (costs exceed
        // revenue) 
        if (expected_performance != -1.0)
        {
            ASSERT_NEAR(performance, expected_performance, tolerance) << "Performance did not match expected value.";
        }
        // For now, we'll just print the performance. Specific assertions can be
        // added if expected values are known.
        std::cout << "Circuit Vector: [";
        for (size_t i = 0; i < circuit_vector.size(); ++i)
        {
            std::cout << circuit_vector[i] << (i == circuit_vector.size() - 1 ? "" : ", ");
        }
        std::cout << "], Performance: " << performance << " £/s" << std::endl;
    }
};

/**
 * @brief Test case for the original vec1 performance.
 * 
 * This test case checks the performance of a circuit defined by the
 * original vec1 vector. The expected performance is not strictly defined,
 * but it should be a valid circuit.
 * 
 */
TEST_F(CircuitSimulatorTest, OriginalVec1Performance)
{
    std::vector<int> vec1 = {0, 3, 1, 3, 2, 3, 5, 4, 7, 6, 3, 3, 8}; // n=6. Terminals: P1=6, P2=7, T=8
                                                                     // Feed to u0.
                                                                     // u0 -> u3, u1
                                                                     // u1 -> u3, u2
                                                                     // u2 -> u3, u5
                                                                     // u3 -> u4, P2(7)
                                                                     // u4 -> u6(T), u3  -- Problematic: u4->T, u3 and
                                                                     // u3->u4 (cycle T-u4-u3) u5 -> u3, u8(T)

    double performance = circuit_performance(static_cast<int>(vec1.size()), const_cast<int*>(vec1.data()));
    ASSERT_FALSE(std::isnan(performance));
    // It's reasonable to expect negative performance if all valuable material is
    // lost. 
    std::cout << "OriginalVec1 Performance: " << performance << std::endl;
}

/**
 * @brief Test case for a minimal valid circuit.
 * 
 * This test case checks the performance of a minimal valid circuit
 * with n=1. The expected performance is not strictly defined,
 * but it should be a valid circuit.
 */
TEST_F(CircuitSimulatorTest, MinimalValidCircuit)
{
    std::vector<int> vec = {0, 1, 3}; // n=1. u0 -> P1, T
    // We don't have a hardcoded expected value, but it should be calculable and
    // positive if valid.
    run_performance_test(vec);
}

/**
 * @brief Test case for a circuit with all products to tailings.
 * 
 * This test case checks the performance of a circuit where all products
 * are routed to tailings. The expected performance is not strictly defined,
 * but it should be a valid circuit.
 */
TEST_F(CircuitSimulatorTest, AllToTailingsN2)
{
    // n=2. Terminals P1=2, P2=3, T=4
    // Feed to u0. u0 -> T, u1. u1 -> T, T. (Effectively all to T)
    std::vector<int> vec = {0, 4, 1, 4, 4}; // size 5. n=(5-1)/2=2
    double performance = circuit_performance(static_cast<int>(vec.size()), const_cast<int*>(vec.data()));
    ASSERT_FALSE(std::isnan(performance));
    // Expect a negative performance value due to costs and no revenue.
    ASSERT_LT(performance, 0.0);
    std::cout << "AllToTailingsN2 Performance: " << performance << std::endl;
}

/**
 * @brief Test case for a circuit with misrouted products.
 * 
 * This test case checks the performance of a circuit with misrouted
 * products. The expected performance is not strictly defined, but it
 * should be a valid circuit.
 */
TEST_F(CircuitSimulatorTest, MisroutedProductsN1)
{
    // n=1. Terminals P1=1, P2=2, T=3
    // Feed u0. u0 -> P2, P1 (Gormanium product, Palusznium product)
    // This should be valid but might have penalties if Gormanium goes to
    // PalProduct stream etc.
    std::vector<int> vec = {0, 2, 1}; // size 3. n=(3-1)/2=1
    run_performance_test(vec);
}

/**
 * @brief Test case for a long chain circuit.
 * 
 * This test case checks the performance of a long chain circuit
 * with 20 units. The expected performance is not strictly defined,
 * but it should be a valid circuit.
 */
TEST_F(CircuitSimulatorTest, LongChainCircuit)
{
    const int n_units = 20;
    std::vector<int> vec;
    vec.push_back(0); // Feed to u0
    for (int i = 0; i < n_units; ++i)
    {
        if (i < n_units - 1)
        {
            vec.push_back(i + 1); // Conc to next unit
        }
        else
        {
            vec.push_back(n_units); // Conc to P1 (Palusznium product)
        }
        vec.push_back(n_units + 2); // Tail to T (Tailings)
    }
    // This circuit should be valid. Performance will depend on volume penalties.
    run_performance_test(vec);
}


/**
 * @brief Test case for a potentially invalid loop circuit.
 * 
 * This test case checks the performance of a circuit that has
 * a potentially invalid loop.
 * 
 */
TEST_F(CircuitSimulatorTest, PotentiallyInvalidLoopNoConvergence)
{
    // n=2. Terminals P1=2,P2=3,T=4
    // u0 -> u0 (invalid self-loop), u1. u1 -> u1 (invalid self-loop), u0
    // This vector is {X, 0,1, 1,0}. If feed is to u0, it's {0, 0,1, 1,0}
    std::vector<int> vec = {0, 0, 1, 1, 0};
    // circuit_performance should handle this, possibly by returning a very low
    // score or a score indicating invalidity if check_validity is integrated.
    double performance = circuit_performance(static_cast<int>(vec.size()), const_cast<int*>(vec.data()));
    ASSERT_FALSE(std::isnan(performance));
    // For an invalid circuit like this, expect a very poor (negative)
    // performance. The exact value might be a specific constant for invalid
    // circuits if implemented.
    std::cout << "PotentiallyInvalidLoopNoConvergence Performance: " << performance << std::endl;
}

/**
 * @brief Test case for a simple valid circuit with n=2.
 */
TEST_F(CircuitSimulatorTest, SimpleValidN2)
{
    std::vector<int> vec = {0, 2, 1, 3, 4};
    run_performance_test(vec);
}
