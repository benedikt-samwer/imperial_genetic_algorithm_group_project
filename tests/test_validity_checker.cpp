#include <CUnit/Basic.h>
#include <CCircuit.h>
#include <iostream>
#include <vector>
#include <iterator>

/*───────────────────────────────────────────────*
 * original demo
 *───────────────────────────────────────────────*/
void manual_demo()
{
    int valid[13] = { 0,3,1,3,2,3,5,4,7,6,3,3,8 };   // n = 3
    Circuit c(6);

    std::cout << "check_validity demo: ";
    bool ok = c.check_validity(std::size(valid), valid);
    std::cout << (ok ? "pass\n" : "fail\n");
}

/*=====================  test cases  ===========================*/

// 00 vector size is not valid
void test_invalid_length()
{
    Circuit c(5);                    
    std::vector<int> v(9, 0);        
    CU_ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

// 01 feed points to terminal
void test_feed_points_to_terminal()
{
    Circuit c(5);                    
    std::vector<int> v(11, 5);
    v[0] = 5;   // feed points to terminal
    CU_ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

// 02 / 03 conc out of range
void test_conc_out_of_range()
{
    Circuit c(4);
    std::vector<int> v = {
        0, 7,5, 1,5, 2,5, 3,5, 4,5  // unit0 conc out of range
    };
    CU_ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

// 04 self-loop
void test_invalid_self_loop()
{
    Circuit c(4);
    std::vector<int> v = {
        0, 0,5,  1,5,  2,5,  3,5,  4,5
    };                               // unit0 self-loop
    CU_ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

// 05 conc == tail
void test_invalid_same_destination()
{
    Circuit c(4);
    std::vector<int> v = {
        0, 1,1,  2,5,  3,5,  4,5,  5,6 
    }; 
    CU_ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

// 06 unreachable unit
void test_unreachable_unit()
{
    Circuit c(5);                    // unit4 unreachable
    std::vector<int> v = {
        0, 1,5,  5,6,  5,6,  5,6,  5,6,  5,6
    };
    CU_ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

// 07 only one terminal
void test_only_one_terminal()
{
    Circuit c(3);                   
    std::vector<int> v = {
        0,
        1,3,            // 0 → 1,P1
        2,3,            // 1 → 2,P1
        3,3             // 2 → P1,P1
    };
    CU_ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

void test_R2_condition() {
    Circuit c(10);  // 10 units
    
    // 构造一个无效电路：Unit 0的conc和tail都指向P1（只能到达一个终端流）
    std::vector<int> v = {
        0,           // feed -> Unit 0
        10, 10,      // Unit 0: conc -> P1, tail -> P1 (违反R2)
        10, 11,      // Unit 1: conc -> P1, tail -> P2
        10, 11,      // Unit 2: conc -> P1, tail -> P2
        10, 11,      // Unit 3: conc -> P1, tail -> P2
        10, 11,      // Unit 4: conc -> P1, tail -> P2
        10, 11,      // Unit 5: conc -> P1, tail -> P2
        10, 11,      // Unit 6: conc -> P1, tail -> P2
        10, 11,      // Unit 7: conc -> P1, tail -> P2
        10, 11,      // Unit 8: conc -> P1, tail -> P2
        10, 11       // Unit 9: conc -> P1, tail -> P2
    };
    
    CU_ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

// 08 / 09 / 13 two-node cycle
void test_two_node_cycle()
{
    Circuit c(3);
    std::vector<int> v = {
        0,
        1,3,            // 0 → 1
        0,4,            // 1 → 0  (cycle)
        3,4             // 2 → terminal
    };
    CU_ASSERT_FALSE(c.check_validity((int)v.size(), v.data()));
}

// 10 valid n = 10
void test_basic_valid()
{
    std::vector<int> v = {
        0,
        1,10,  2,11,  3,12,  4,10,  5,11,
        6,12,  7,10,  8,11,  9,12, 10,11
    };                              
    Circuit c(10);
    CU_ASSERT_TRUE(c.check_validity((int)v.size(), v.data()));
}

// 11 small valid n = 4
void test_small_valid()
{
    Circuit c(4);
    std::vector<int> v = {
        1,2,3,0,3,4,3,0,6
    };
    CU_ASSERT_TRUE(c.check_validity((int)v.size(), v.data()));
}

void test_small_valid_02()
{
    Circuit c(3);
    std::vector<int> v = {
        0,1,2,3,0,0,5
    };
    CU_ASSERT_TRUE(c.check_validity((int)v.size(), v.data()));
}

void test_small_valid_03()
{
    Circuit c(10);
    std::vector<int> v = {
    /* Feed destination */ 0,  // Input flow enters unit 0
    /* Unit 0 */ 1, 3,   // Concentrate to unit 1, tailings to unit 3
    /* Unit 1 */ 2, 4,   // Concentrate to unit 2, tailings to unit 4  
    /* Unit 2 */ 5, 6,   // Concentrate to unit 5, tailings to unit 6
    /* Unit 3 */ 0, 7,   // Concentrate recycled to unit 0, tailings to unit 7
    /* Unit 4 */ 8, 9,   // Concentrate to unit 8, tailings to unit 9
    /* Unit 5 */ 10, 11, // Concentrate to palusznium product (10), tailings to gormanium product (11)
    /* Unit 6 */ 0, 12,  // Concentrate recycled to unit 0, tailings to tailings product (12)
    /* Unit 7 */ 2, 12,  // Concentrate to unit 2, tailings to tailings product
    /* Unit 8 */ 10, 12, // Concentrate to palusznium product, tailings to tailings product
    /* Unit 9 */ 11, 12  // Concentrate to gormanium product, tailings to tailings product
    };
    CU_ASSERT_TRUE(c.check_validity((int)v.size(), v.data()));
}

// 12 basic valid circuit
void test_basic_circuit_validity()
{
    std::vector<int> v = {
        0,
        1,10,  2,11,  3,12,  4,10,  5,11,
        6,12,  7,10,  8,11,  9,12,  10,11
    };
    Circuit c(10);
    CU_ASSERT_TRUE(c.check_validity(static_cast<int>(v.size()), v.data()));
}

// 13 mass balance converges
void test_mass_balance_converges_valid(void)
{
    Circuit c(2);

    int cv[5] = { 0,   // feed_dest = unit0
                  1, 2,// unit0: conc→unit1, tail→P1
                  3, 4 // unit1: conc→P2,   tail→TA
                };

    CU_ASSERT_TRUE( c.check_validity(5, cv) );

    CU_ASSERT_TRUE( c.mass_balance_converges() );
}

// 14 mass balance not converge
static void test_mass_balance_not_converge_small_iter(void)
{
    Circuit c(2);

    int cv[5] = { 0,        // feed → unit0
                  1, 2,     // unit0: conc→unit1, tail→P1
                  3, 4      // unit1: conc→P2,   tail→TA
                };

    CU_ASSERT_TRUE( c.check_validity(5, cv) );

    CU_ASSERT_FALSE( c.mass_balance_converges(Constants::Simulation::DEFAULT_TOLERANCE,
                                              1) );          // ← only 1 iteration
}

/*───────────────────────────────────────────────*
 * main                                          *
 *───────────────────────────────────────────────*/
int main()
{
    /*----------- CUnit initialization ------------------*/
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    CU_pSuite s = CU_add_suite("Circuit-Suite", nullptr, nullptr);
    if (!s) { CU_cleanup_registry(); return CU_get_error(); }

    CU_add_test(s, "basic valid",      test_basic_circuit_validity);
    CU_add_test(s, "manual demo", manual_demo);
    CU_add_test(s, "invalid length",           test_invalid_length);
    CU_add_test(s, "feed points to terminal",  test_feed_points_to_terminal);
    CU_add_test(s, "conc out of range",        test_conc_out_of_range);
    CU_add_test(s, "self-loop",                test_invalid_self_loop);
    CU_add_test(s, "conc==tail",               test_invalid_same_destination);
    CU_add_test(s, "unreachable unit",         test_unreachable_unit);
    CU_add_test(s, "single terminal only",     test_only_one_terminal);
    CU_add_test(s, "two-node cycle",           test_two_node_cycle);
    CU_add_test(s, "basic valid (n=10)",       test_basic_valid);
    CU_add_test(s, "small valid (n=4)",        test_small_valid);
    CU_add_test(s, "valid circuit should converge", test_mass_balance_converges_valid);
    CU_add_test(s, "not converge with 1 iteration", test_mass_balance_not_converge_small_iter);
    CU_add_test(s, "R2 condition", test_R2_condition);
    CU_add_test(s, "small valid (n=3)",        test_small_valid_02);
    CU_add_test(s, "small valid (n=10)",       test_small_valid_03);



    /*----------- run tests ----------------------*/
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();

    /*-----------  manual demo ------------------*/
    manual_demo();

    /*----------- return code: if any test fails, exit non-0 ----*/
    unsigned fails = CU_get_number_of_failures();
    CU_cleanup_registry();
    return (fails == 0) ? 0 : 1;
}
