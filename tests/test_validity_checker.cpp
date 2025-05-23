/**
 * @file test_validity_checker.cpp
 * @brief Unit tests for the CCircuit class, specifically focusing on
 * validity checking of circuit configurations.
 *
 */
#include "CCircuit.h"
#include <cmath>
#include <gtest/gtest.h>
#include <vector>

/**
 * @brief Test fixture for the ValidityCheckerTest class.
 *
 * This class sets up the test environment for testing the validity of
 * circuit configurations.
 */
class ValidityCheckerTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test for a manually created circuit vector.
 *
 * This test checks the validity of a circuit configuration using a manually
 * created vector.
 */
TEST_F(ValidityCheckerTest, ManualDemo)
{
    std::vector<int> v = {0, 3, 1, 3, 2, 3, 5, 4, 7, 6, 3, 3, 8};
    Circuit c(6);
    ASSERT_TRUE(c.check_validity((int)v.size(), v.data()));
}

/**
 * @brief Test for an empty circuit vector.
 *
 * This test checks the validity of an empty circuit vector.
 */
TEST_F(ValidityCheckerTest, InvalidLength)
{
    Circuit c(5);
    std::vector<int> v(9, 0);
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

/**
 * @brief Test for a circuit vector with a feed unit pointing to a terminal.
 *
 * This test checks the validity of a circuit configuration where the feed unit
 * points to a terminal.
 */
TEST_F(ValidityCheckerTest, FeedPointsToTerminal)
{
    Circuit c(5);
    std::vector<int> v(11, 0);
    v[0] = 5;
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

/**
 * @brief Test for a circuit vector with an out-of-range concentration value.
 *
 * This test checks the validity of a circuit configuration where the
 * concentration value is out of range.
 */
TEST_F(ValidityCheckerTest, ConcOutOfRange)
{
    Circuit c(5);
    std::vector<int> v = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

/**
 * @brief Test for a circuit vector with an out-of-range tail value.
 *
 * This test checks the validity of a circuit configuration where the tail
 * value is out of range.
 */
TEST_F(ValidityCheckerTest, InvalidSelfLoop)
{
    Circuit c(5);
    std::vector<int> v = {0, 0, 5, 1, 5, 2, 5, 3, 5, 4, 5};
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

/**
 * @brief Test for a circuit vector with an invalid destination.
 *
 * This test checks the validity of a circuit configuration where the
 * destination is invalid.
 */
TEST_F(ValidityCheckerTest, InvalidSameDestination)
{
    Circuit c(5);
    std::vector<int> v = {0, 1, 1, 2, 5, 3, 5, 4, 5, 5, 6};
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

/**
 * @brief Test for a circuit vector with an unreachable unit.
 *
 * This test checks the validity of a circuit configuration where a unit is
 * unreachable.
 */
TEST_F(ValidityCheckerTest, UnreachableUnit)
{
    Circuit c(5);
    std::vector<int> v = {0, 7, 5, 2, 5, 3, 5, 4, 5, 6, 5};
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

/**
 * @brief Test for a circuit vector with two terminals.
 *
 * This test checks the validity of a circuit configuration where there are
 * two terminals.
 */
TEST_F(ValidityCheckerTest, TwoTerminalsCheck)
{
    Circuit c(3);
    std::vector<int> v = {
        0, 1, 2, // unit0 conc,tail
        0, 3,    // unit1 conc,tail
        0, 3     // unit2 conc,tail
    };
    // This setup should fail the two terminals check for some units.
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

/**
 * @brief Test for a circuit vector with a cycle.
 *
 * This test checks the validity of a circuit configuration where there is a
 * cycle.
 */
TEST_F(ValidityCheckerTest, TwoNodeCycle)
{
    Circuit c(2);
    std::vector<int> v = {
        0,    // feed
        1, 2, // u0 -> u1, T0 (n+0=2)
        0, 3  // u1 -> u0, T1 (n+1=3)
    };
    // This creates a cycle 0 -> 1 -> 0. It should be caught by reachability or
    // terminal checks. Depending on how term_mask handles cycles, this might also
    // fail two_terminals_check or mass_balance. The original expected
    // !c.check_validity.
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

/**
 * @brief Test for a basic valid circuit vector.
 *
 * This test checks the validity of a basic circuit configuration.
 */
TEST_F(ValidityCheckerTest, BasicValidN10)
{
    Circuit c(10);
    std::vector<int> v_corrected = {0, 1, 10, 2, 11, 3, 12, 4, 10, 5, 11, 6, 12, 7, 10, 8, 11, 9, 12, 10, 11};
    ASSERT_TRUE(c.check_validity((int)v_corrected.size(), v_corrected.data()));
}

/**
 * @brief Test for a small valid circuit vector with n=4.
 *
 * This test checks the validity of a small circuit configuration with n=4.
 */
TEST_F(ValidityCheckerTest, SmallValidN4)
{
    Circuit c(4); // n=4. Terminals P1=4, P2=5, T=6
    std::vector<int> v = {1, 2, 3, 0, 3, 4, 3, 0, 6};
    ASSERT_TRUE(c.check_validity((int)v.size(), v.data()));
}

/**
 * @brief Test for a small valid circuit vector with n=3.
 *
 * This test checks the validity of a small circuit configuration with n=3.
 */
TEST_F(ValidityCheckerTest, SmallValidN3_02)
{
    Circuit c(3);                               // n=3. Terminals P1=3,P2=4,T=5
    std::vector<int> v = {0, 1, 2, 3, 0, 0, 5}; // Size 7. 2*3+1=7. Ok.
                                                // feed=u0.
                                                // u0: conc=u1, tail=u2
                                                // u1: conc=P1, tail=u0
                                                // u2: conc=u0, tail=T
    ASSERT_TRUE(c.check_validity((int)v.size(), v.data()));
}

/**
 * @brief Test for a small valid circuit vector with n=10.
 *
 * This test checks the validity of a small circuit configuration with n=10.
 */
TEST_F(ValidityCheckerTest, SmallValidN10_03)
{
    Circuit c(10); // n=10. Terminals P1=10,P2=11,T=12
    std::vector<int> v = {
        0,  1, 3,  2,  4, 5,  6, 0,  7,      // u0-u3
        8,  9, 10, 11, 0, 12, 2, 12, 10, 12, // u4-u8
        11, 12                               // u9
    }; // Size 21. 2*10+1=21. Ok.
    ASSERT_TRUE(c.check_validity((int)v.size(), v.data()));
}

/**
 * @brief Test for a basic circuit vector with n=10.
 *
 * This test checks the validity of a basic circuit configuration with n=10.
 */
TEST_F(ValidityCheckerTest, BasicCircuitValidityN10)
{
    Circuit c(10);
    std::vector<int> v = {
        0, 1,  10, 2,  11, 3,  12, 4,  10, 5, 11,
        6, 12, 7,  10, 8,  11, 9,  12, 10, 11 // u9 -> P1, P2. Last two elements
                                              // for unit 9.
    };
    ASSERT_TRUE(c.check_validity((int)v.size(), v.data()));
}

// Mass balance convergence tests need careful setup as check_validity calls
// run_mass_balance. A circuit that is structurally valid but would fail mass
// balance is needed for a good FALSE case. A circuit that is structurally valid
// and converges for a TRUE case.

/**
 * @brief Test for a mass balance convergence with a valid circuit vector.
 *
 * This test checks the validity of a circuit configuration that is known to
 * converge for mass balance.
 * This test assumes check_validity itself doesn't run mass_balance, or we are
 * testing a known good vector. Since check_validity *does* run mass_balance,
 * this test is essentially the same as other valid tests if the vector is good.
 *
 */
TEST_F(ValidityCheckerTest, DISABLED_MassBalanceConvergesValid)
{
    Circuit c(2);
    std::vector<int> cv = {0, 2, 1, 3, 4};
    ASSERT_TRUE(c.check_validity((int)cv.size(), cv.data()));
}

/**
 * @brief Test for a mass balance convergence failure with a valid circuit
 * vector.
 *
 * This test checks the validity of a circuit configuration that is known to
 * fail mass balance.
 * This test assumes check_validity itself doesn't run mass_balance, or we are
 * testing a known good vector. Since check_validity *does* run mass_balance,
 * this test is essentially the same as other valid tests if the vector is good.
 *
 */
TEST_F(ValidityCheckerTest, DISABLED_MassBalanceNotConverge)
{
    // Placeholder: This requires a specific circuit vector that is structurally
    // valid but guaranteed to fail mass balance within 100 iterations. For
    // example, a complex recirculating load without proper outlets might do this.
    Circuit c(2);
    std::vector<int> cv = {0, 0, 1, 1, 0}; // Example of a problematic circuit (u0<->u1 tight loop without
                                           // outlets for them) This will likely fail other checks before mass
                                           // balance (e.g. two terminals or final terminal check).
    // A better example would be one that passes all other checks but fails mass
    // balance. For now, we expect this to be false due to other reasons, or if it
    // passes them, potentially mass balance.
    ASSERT_FALSE(c.check_validity((int)cv.size(), cv.data()));
}

/**
 * @brief Test for a valid circuit vector with unit parameters.
 *
 * This test checks the validity of a circuit configuration with unit
 * parameters.
 */
TEST_F(ValidityCheckerTest, ValidWithUnitParameters)
{
    Circuit c(2);
    std::vector<int> circuit_vec = {0, 2, 1, 3, 4};
    std::vector<double> unit_params = {0.5, 0.5};
    ASSERT_TRUE(
        c.check_validity((int)circuit_vec.size(), circuit_vec.data(), (int)unit_params.size(), unit_params.data()));
}

/**
 * @brief Test for an invalid circuit vector with unit parameters size.
 *
 * This test checks the validity of a circuit configuration with an incorrect
 * size of unit parameters.
 */
TEST_F(ValidityCheckerTest, InvalidUnitParametersSize)
{
    Circuit c(2);
    std::vector<int> circuit_vec = {0, 2, 1, 3, 4};
    std::vector<double> unit_params = {0.5}; // Incorrect size (should be 2)
    ASSERT_FALSE(
        c.check_validity((int)circuit_vec.size(), circuit_vec.data(), (int)unit_params.size(), unit_params.data()));
}

/**
 * @brief Test for an invalid circuit vector with unit parameters values.
 *
 * This test checks the validity of a circuit configuration with invalid unit
 * parameters values.
 */
TEST_F(ValidityCheckerTest, InvalidUnitParameterValueTooLow)
{
    Circuit c(2);
    std::vector<int> circuit_vec = {0, 2, 1, 3, 4};
    std::vector<double> unit_params = {-0.1, 0.5}; // Invalid beta < 0
    ASSERT_FALSE(
        c.check_validity((int)circuit_vec.size(), circuit_vec.data(), (int)unit_params.size(), unit_params.data()));
}

/**
 * @brief Test for an invalid circuit vector with unit parameters values.
 *
 * This test checks the validity of a circuit configuration with invalid unit
 * parameters values.
 */
TEST_F(ValidityCheckerTest, InvalidUnitParameterValueTooHigh)
{
    Circuit c(2);
    std::vector<int> circuit_vec = {0, 2, 1, 3, 4};
    std::vector<double> unit_params = {0.5, 1.1}; // Invalid beta > 1
    ASSERT_FALSE(
        c.check_validity((int)circuit_vec.size(), circuit_vec.data(), (int)unit_params.size(), unit_params.data()));
}

/**
 * @brief Test for an invalid circuit vector with unit parameters NaN.
 *
 * This test checks the validity of a circuit configuration with invalid unit
 * parameters values (NaN).
 */
TEST_F(ValidityCheckerTest, UnitParameterNaN)
{
    Circuit c(2);
    std::vector<int> circuit_vec = {0, 2, 1, 3, 4};
    std::vector<double> unit_params = {0.5, NAN}; // Invalid beta NaN
    ASSERT_FALSE(
        c.check_validity((int)circuit_vec.size(), circuit_vec.data(), (int)unit_params.size(), unit_params.data()));
}

/**
 * @brief Test for a circuit vector with an invalid index.
 *
 * This test checks the validity of a circuit configuration with an invalid
 * index.
 */
TEST_F(ValidityCheckerTest, IndexCheckNegativeConc)
{
    Circuit c(2);
    std::vector<int> v = {0, -1, 2, 0, 3}; // u0 conc = -1
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

/**
 * @brief Test for a circuit vector with an invalid tail index.
 *
 * This test checks the validity of a circuit configuration with an invalid
 * tail index.
 */
TEST_F(ValidityCheckerTest, IndexCheckNegativeTail)
{
    Circuit c(2);
    std::vector<int> v = {0, 2, -1, 0, 3}; // u0 tail = -1
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

/**
 * @brief Test for a circuit vector with an invalid tail index.
 *
 * This test checks the validity of a circuit configuration with an invalid
 * tail index.
 */
TEST_F(ValidityCheckerTest, MissingAllTerminals)
{                                   // Should fail multiple checks, including this one
    Circuit c(1);                   // n=1, P1=1, P2=2, T=3
    std::vector<int> v = {0, 0, 0}; // u0 -> u0, u0 (fails self-loop, same dest, then terminal checks)
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

/**
 * @brief Test for a circuit vector with missing P1 and P2 terminals.
 *
 * This test checks the validity of a circuit configuration with missing P1 and
 * P2 terminals.
 */
TEST_F(ValidityCheckerTest, MissingP1AndP2Terminals)
{
    Circuit c(1);                   // n=1, P1=1, P2=2, T=3
    std::vector<int> v = {0, 3, 3}; // u0 -> T, T (fails same dest, then terminal check for P1/P2)
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

/**
 * @brief Test for a circuit vector with missing tailings terminal.
 *
 * This test checks the validity of a circuit configuration with missing
 * tailings terminal.
 */
TEST_F(ValidityCheckerTest, MissingTailingsTerminal)
{
    Circuit c(2); // n=2, P1=2, P2=3, T=4
    // u0 -> P1, u1; u1 -> P1, P2. No path to Tailings.
    std::vector<int> v = {0, 2, 1, 2, 3};
    ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}
