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
    int valid[7] = { 0, 1,3, 2,4, 4,5 };   // n = 3
    Circuit c(3);

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

// 11 small valid n = 2
void test_small_valid()
{
    Circuit c(2);
    std::vector<int> v = {
        0,          // feed
        1,2,        // unit0
        3,4         // unit1
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
    CU_add_test(s, "small valid (n=2)",        test_small_valid);


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
