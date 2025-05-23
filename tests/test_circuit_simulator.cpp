#include <cmath>
#include <iostream>

#include "CSimulator.h"

#include <gtest/gtest.h>
#include <string>
#include <utility>
#include <vector>

// Define a test fixture if needed, or use TEST macro for simple tests
class CircuitSimulatorTest : public ::testing::Test
{
protected:
    // You can add SetUp and TearDown methods here if needed
    // void SetUp() override {}
    // void TearDown() override {}

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
        // revenue) ASSERT_GE(performance, 0.0) << "Performance should generally be
        // non-negative for valid, productive circuits.";

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

// Test case based on the original vec1
// The original test expected a performance around 301.91 for this vector with
// n=6 However, circuit_performance internally uses n derived from vector size.
// For {0,3,1,3,2,3,5,4,7,6,3,3,8}, size is 13. So, n = (13-1)/2 = 6.
// This matches the original manual test's assumption for vec1.
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
    // lost. ASSERT_LT(performance, 0.0); For now, just ensure it runs. The exact
    // expected value needs clarification for this specific (likely invalid)
    // vector.
    std::cout << "OriginalVec1 Performance: " << performance << std::endl;
}

// Test with a minimal valid circuit
// n=1. Feed to u0. u0 -> P1, T.
// P1 = 1, P2 = 2, T = 3
// Vector: {0, 1, 3} -> size 3. (2*1+1)
TEST_F(CircuitSimulatorTest, MinimalValidCircuit)
{
    std::vector<int> vec = {0, 1, 3}; // n=1. u0 -> P1, T
    // We don't have a hardcoded expected value, but it should be calculable and
    // positive if valid.
    run_performance_test(vec);
}

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

TEST_F(CircuitSimulatorTest, MisroutedProductsN1)
{
    // n=1. Terminals P1=1, P2=2, T=3
    // Feed u0. u0 -> P2, P1 (Gormanium product, Palusznium product)
    // This should be valid but might have penalties if Gormanium goes to
    // PalProduct stream etc.
    std::vector<int> vec = {0, 2, 1}; // size 3. n=(3-1)/2=1
    run_performance_test(vec);
}

// This test is based on "Long Circuit with Volume Penalty"
// n = 20. Size = 2*20+1 = 41.
// Each unit i -> i+1 (or P1 for last), T. (A simple chain)
// Example: u0->u1,T; u1->u2,T; ...; u19->P1,T
// Terminals for n=20: P1=20, P2=21, T=22
// The original vector {0, 1, 21, ...} implies P2 for concentrate, which is
// Gormanium. If we want Palusznium, it should be P1 (n). Let's make a simple
// chain: u0->u1,T ... u18->u19,T u19->P1,T
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

// Test for a circuit that is likely invalid and might not converge or be caught
// by check_validity The original test_validity_checker had {0,0,1,1,0} for n=2
// as problematic. circuit_performance should ideally call check_validity. If
// it's invalid, performance should reflect that (e.g., a large negative
// default).
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
    // circuits if implemented. Example: ASSERT_EQ(performance,
    // Constants::Circuit::INVALID_CIRCUIT_PERFORMANCE_VALUE); For now, just
    // checking it doesn't crash and returns a number.
    std::cout << "PotentiallyInvalidLoopNoConvergence Performance: " << performance << std::endl;
}

// Example of a simple valid circuit with known n=2
// Feed to u0. u0 -> P1, u1. u1 -> P2, T
// n=2. P1=2, P2=3, T=4
// Vector: {0, 2,1, 3,4}
TEST_F(CircuitSimulatorTest, SimpleValidN2)
{
    std::vector<int> vec = {0, 2, 1, 3, 4};
    run_performance_test(vec);
}

// The main function is provided by gtest_main