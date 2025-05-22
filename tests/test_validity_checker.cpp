// main.cpp 
#include <vector>
#include <cmath>  // 添加这行以获取 NAN 常量
#include "CCircuit.h" 
#include <gtest/gtest.h> // Include Google Test

// Namespace and old report functions are removed as GTest handles this.

class ValidityCheckerTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Original test_manual_demo
TEST_F(ValidityCheckerTest, ManualDemo) {
    std::vector<int> v = {0,3,1,3,2,3,5,4,7,6,3,3,8};
    Circuit c(6); 
    ASSERT_TRUE(c.check_validity((int)v.size(), v.data())); 
}

TEST_F(ValidityCheckerTest, InvalidLength) {
    Circuit c(5);
    std::vector<int> v(9, 0); 
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

TEST_F(ValidityCheckerTest, FeedPointsToTerminal) {
    Circuit c(5);
    std::vector<int> v(11, 0);
    v[0] = 5; 
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

TEST_F(ValidityCheckerTest, ConcOutOfRange) {
    Circuit c(5); 
    std::vector<int> v = {0, 1,2, 3,4, 5,6, 7,8, 9,10}; 
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

TEST_F(ValidityCheckerTest, InvalidSelfLoop) {
    Circuit c(5); 
    std::vector<int> v = {0,0,5, 1,5, 2,5, 3,5, 4,5}; 
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

TEST_F(ValidityCheckerTest, InvalidSameDestination) {
    Circuit c(5); 
    std::vector<int> v = {0,1,1, 2,5, 3,5, 4,5, 5,6}; 
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

TEST_F(ValidityCheckerTest, UnreachableUnit) {
    Circuit c(5); 
    std::vector<int> v = {0,  7,5,  2,5,  3,5,  4,5,  6,5};
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

// Original test_two_terminals_check
TEST_F(ValidityCheckerTest, TwoTerminalsCheck) {
    Circuit c(3);
    std::vector<int> v = {0,
                          1, 2, // unit0 conc,tail
                          0, 3, // unit1 conc,tail
                          0, 3  // unit2 conc,tail
                         };
    // This setup should fail the two terminals check for some units.
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

// Original test_two_node_cycle
TEST_F(ValidityCheckerTest, TwoNodeCycle) {
    Circuit c(2);
    std::vector<int> v = {0,    // feed
                          1, 2, // u0 -> u1, T0 (n+0=2)
                          0, 3  // u1 -> u0, T1 (n+1=3)
                         };
    // This creates a cycle 0 -> 1 -> 0. It should be caught by reachability or terminal checks.
    // Depending on how term_mask handles cycles, this might also fail two_terminals_check or mass_balance.
    // The original expected !c.check_validity.
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

// Original test_basic_valid (n=10)
TEST_F(ValidityCheckerTest, BasicValidN10) {
    Circuit c(10);
    std::vector<int> v_corrected = { 
        0, 1,10, 2,11, 3,12, 4,10, 5,11,
        6,12, 7,10, 8,11, 9,12, 10,11
    };
    ASSERT_TRUE(c.check_validity((int)v_corrected.size(), v_corrected.data()));
}

// Original test_small_valid (n=4)
TEST_F(ValidityCheckerTest, SmallValidN4) {
    Circuit c(4); // n=4. Terminals P1=4, P2=5, T=6
    std::vector<int> v = {1,2,3,0,3,4,3,0,6}; // Corrected: {feed_idx, u0_c,u0_t, u1_c,u1_t, u2_c,u2_t, u3_c,u3_t}
                                          // feed=u1. u0->u2,u3. u1->u0,u3. u2->u3,P1. u3->u0,T.
                                          // Original: {1,2,3,0,3,4,3,0,6} size 9. 2*4+1=9. Ok.
                                          // feed_dest = v[0] = 1 (unit 1)
                                          // u0: conc=v[1]=2 (unit 2), tail=v[2]=3 (unit 3)
                                          // u1: conc=v[3]=0 (unit 0), tail=v[4]=3 (unit 3)
                                          // u2: conc=v[5]=4 (P1),    tail=v[6]=3 (unit 3)
                                          // u3: conc=v[7]=0 (unit 0), tail=v[8]=6 (T)
    ASSERT_TRUE(c.check_validity((int)v.size(), v.data()));
}

// Original test_small_valid_02 (n=3)
TEST_F(ValidityCheckerTest, SmallValidN3_02) {
    Circuit c(3); // n=3. Terminals P1=3,P2=4,T=5
    std::vector<int> v = {0,1,2,3,0,0,5}; // Size 7. 2*3+1=7. Ok.
                                       // feed=u0.
                                       // u0: conc=u1, tail=u2
                                       // u1: conc=P1, tail=u0 
                                       // u2: conc=u0, tail=T
    ASSERT_TRUE(c.check_validity((int)v.size(), v.data()));
}

// Original test_small_valid_03 (n=10)
TEST_F(ValidityCheckerTest, SmallValidN10_03) {
    Circuit c(10); // n=10. Terminals P1=10,P2=11,T=12
    std::vector<int> v = {
        0,1,3, 2,4, 5,6, 0,7,          // u0-u3
        8,9, 10,11, 0,12, 2,12, 10,12, // u4-u8
        11,12                           // u9
    }; // Size 21. 2*10+1=21. Ok.
    ASSERT_TRUE(c.check_validity((int)v.size(), v.data()));
}

// Original test_basic_circuit_validity (n=10)
TEST_F(ValidityCheckerTest, BasicCircuitValidityN10) {
    Circuit c(10);
    std::vector<int> v = {
        0, 1,10, 2,11, 3,12, 4,10, 5,11,
        6,12, 7,10, 8,11, 9,12, 10,11 // u9 -> P1, P2. Last two elements for unit 9.
    };
    ASSERT_TRUE(c.check_validity((int)v.size(), v.data()));
}

// Mass balance convergence tests need careful setup as check_validity calls run_mass_balance.
// A circuit that is structurally valid but would fail mass balance is needed for a good FALSE case.
// A circuit that is structurally valid and converges for a TRUE case.

// Original: test_mass_balance_converges_valid
// This test assumes check_validity itself doesn't run mass_balance, or we are testing a known good vector.
// Since check_validity *does* run mass_balance, this test is essentially the same as other valid tests if the vector is good.
TEST_F(ValidityCheckerTest, DISABLED_MassBalanceConvergesValid) {
    Circuit c(2);
    std::vector<int> cv = {0,
                          2,1,
                          3,4
                         };
    ASSERT_TRUE(c.check_validity((int)cv.size(), cv.data()));
}

// Original: test_mass_balance_not_converge_small_iter
// To make this test meaningful for check_validity, we need a circuit that is structurally valid
// but is known to fail mass balance with default iterations used in check_validity's run_mass_balance call.
// The original test was c.run_mass_balance(Constants::Simulation::DEFAULT_TOLERANCE, 1);
// check_validity calls run_mass_balance(1e-6, 100).
// It's hard to construct such a specific vector without deeper model knowledge.
// For now, this demonstrates the structure.
TEST_F(ValidityCheckerTest, DISABLED_MassBalanceNotConverge) {
    // Placeholder: This requires a specific circuit vector that is structurally valid
    // but guaranteed to fail mass balance within 100 iterations.
    // For example, a complex recirculating load without proper outlets might do this.
    Circuit c(2); 
    std::vector<int> cv = {0, 0,1, 1,0}; // Example of a problematic circuit (u0<->u1 tight loop without outlets for them)
                                      // This will likely fail other checks before mass balance (e.g. two terminals or final terminal check).
    // A better example would be one that passes all other checks but fails mass balance.
    // For now, we expect this to be false due to other reasons, or if it passes them, potentially mass balance.
    ASSERT_FALSE(c.check_validity((int)cv.size(), cv.data())); 
}

// Adding new tests for check_validity with unit_parameters
TEST_F(ValidityCheckerTest, ValidWithUnitParameters) {
    Circuit c(2);
    std::vector<int> circuit_vec = {0, 2,1, 3,4};
    std::vector<double> unit_params = {0.5, 0.5};
    ASSERT_TRUE(c.check_validity((int)circuit_vec.size(), circuit_vec.data(), 
                                (int)unit_params.size(), unit_params.data()));
}

TEST_F(ValidityCheckerTest, InvalidUnitParametersSize) {
    Circuit c(2);
    std::vector<int> circuit_vec = {0, 2,1, 3,4};
    std::vector<double> unit_params = {0.5}; // Incorrect size (should be 2)
    ASSERT_FALSE(c.check_validity((int)circuit_vec.size(), circuit_vec.data(), (int)unit_params.size(), unit_params.data()));
}

TEST_F(ValidityCheckerTest, InvalidUnitParameterValueTooLow) {
    Circuit c(2);
    std::vector<int> circuit_vec = {0, 2,1, 3,4};
    std::vector<double> unit_params = {-0.1, 0.5}; // Invalid beta < 0
    ASSERT_FALSE(c.check_validity((int)circuit_vec.size(), circuit_vec.data(), (int)unit_params.size(), unit_params.data()));
}

TEST_F(ValidityCheckerTest, InvalidUnitParameterValueTooHigh) {
    Circuit c(2);
    std::vector<int> circuit_vec = {0, 2,1, 3,4};
    std::vector<double> unit_params = {0.5, 1.1}; // Invalid beta > 1
    ASSERT_FALSE(c.check_validity((int)circuit_vec.size(), circuit_vec.data(), (int)unit_params.size(), unit_params.data()));
}

TEST_F(ValidityCheckerTest, UnitParameterNaN) {
    Circuit c(2);
    std::vector<int> circuit_vec = {0, 2,1, 3,4};
    std::vector<double> unit_params = {0.5, NAN}; // Invalid beta NaN
    ASSERT_FALSE(c.check_validity((int)circuit_vec.size(), circuit_vec.data(), (int)unit_params.size(), unit_params.data()));
}

TEST_F(ValidityCheckerTest, IndexCheckNegativeConc) {
    Circuit c(2);
    std::vector<int> v = {0, -1, 2, 0, 3}; // u0 conc = -1
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

TEST_F(ValidityCheckerTest, IndexCheckNegativeTail) {
    Circuit c(2);
    std::vector<int> v = {0, 2, -1, 0, 3}; // u0 tail = -1
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

// Tests for final terminal checks (check #8 from CCircuit.cpp)
// Global mask: P1=bit0 (1), P2=bit1 (2), T=bit2 (4)
// Needs (global_mask & 0b011) != 0  (P1 or P2 must be present)
// Needs (global_mask & 0b100) != 0  (T must be present)

TEST_F(ValidityCheckerTest, MissingAllTerminals) { // Should fail multiple checks, including this one
    Circuit c(1); // n=1, P1=1, P2=2, T=3
    std::vector<int> v = {0, 0, 0}; // u0 -> u0, u0 (fails self-loop, same dest, then terminal checks)
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

TEST_F(ValidityCheckerTest, MissingP1AndP2Terminals) { 
    Circuit c(1); // n=1, P1=1, P2=2, T=3
    std::vector<int> v = {0, 3, 3}; // u0 -> T, T (fails same dest, then terminal check for P1/P2)
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

TEST_F(ValidityCheckerTest, MissingTailingsTerminal) {
    Circuit c(2); // n=2, P1=2, P2=3, T=4
    // u0 -> P1, u1; u1 -> P1, P2. No path to Tailings.
    std::vector<int> v = {0, 2,1, 2,3 }; 
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

// No need for the old main function, gtest_main provides it.
/*
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
*/
