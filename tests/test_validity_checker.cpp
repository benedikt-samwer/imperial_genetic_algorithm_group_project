// main.cpp 
#include <iostream>
#include <vector>
#include "CCircuit.h" 


namespace {
    int fail_count = 0;

    void report(const std::string& name, bool passed)
    {
        std::cout << name << ": " << (passed ? "pass" : "fail") << '\n';
        if (!passed) ++fail_count;
    }
}


bool test_manual_demo()
{
    std::vector<int> v = {0,3,1,3,2,3,5,4,7,6,3,3,8};
    Circuit c(6);
    c.initialize_from_vector((int)v.size(), v.data());
    return c.check_validity((int)v.size(), v.data()); 
}

// vector length invalid
bool test_invalid_length()
{
    Circuit c(5);
    std::vector<int> v(9, 0);
    c.initialize_from_vector((int)v.size(), v.data());
    return !c.check_validity((int)v.size(), v.data());
}

// feed points to terminal
bool test_feed_points_to_terminal()
{
    Circuit c(5);
    std::vector<int> v(11, 5);
    v[0] = 5; 
    c.initialize_from_vector((int)v.size(), v.data());                                               // feed → terminal
    return !c.check_validity((int)v.size(), v.data());
}

// conc out of range
bool test_conc_out_of_range()
{
    Circuit c(5);
    std::vector<int> v = {0, 1,2, 3,4, 5,6, 7,8, 9,10};
    c.initialize_from_vector((int)v.size(), v.data());
    return !c.check_validity((int)v.size(), v.data());
}

// self-loop
bool test_invalid_self_loop()
{
    Circuit c(5);
    std::vector<int> v = {0,0,5, 1,5, 2,5, 3,5, 4,5};
    c.initialize_from_vector((int)v.size(), v.data());
    return !c.check_validity((int)v.size(), v.data());
}

// conc == tail
bool test_invalid_same_destination()
{
    Circuit c(5);
    std::vector<int> v = {0,1,1, 2,5, 3,5, 4,5, 5,6};
    c.initialize_from_vector((int)v.size(), v.data());
    return !c.check_validity((int)v.size(), v.data());
}

// unreachable unit
bool test_unreachable_unit()
{
    Circuit c(5);
    std::vector<int> v = {0,7,5, 2,5, 3,5, 4,5, 6,5};
    c.initialize_from_vector((int)v.size(), v.data());
    return !c.check_validity((int)v.size(), v.data());
}

// two terminals check
bool test_two_terminals_check()
{
    Circuit c(3);
    std::vector<int> v = {0,
    /* unit0 conc,tail */ 1, 2,
    /* unit1 conc,tail */ 0, 3,
    /* unit2 conc,tail */ 0, 3};
    c.initialize_from_vector((int)v.size(), v.data());
    return !c.check_validity((int)v.size(), v.data());
}

// two-node cycle
bool test_two_node_cycle()
{
    Circuit c(2);
    std::vector<int> v = {   0,
        /* u0 */   1, 2,
        /* u1 */   0, 3};
    c.initialize_from_vector((int)v.size(), v.data());
    return !c.check_validity((int)v.size(), v.data());
}

// basic valid n=10
bool test_basic_valid()
{
    std::vector<int> v = {
        0, 1,10, 2,11, 3,12, 4,10, 5,11,
        6,12, 7,10, 8,11, 9,12, 10,11
    };
    Circuit c(10);
    c.initialize_from_vector((int)v.size(), v.data());
    return c.check_validity((int)v.size(), v.data());
}

// small valid n=4
bool test_small_valid()
{
    Circuit c(4);
    std::vector<int> v = {1,2,3,0,3,4,3,0,6};
    c.initialize_from_vector((int)v.size(), v.data());
    return c.check_validity((int)v.size(), v.data());
}

// small valid n=3
bool test_small_valid_02()
{
    Circuit c(3);
    std::vector<int> v = {0,1,2,3,0,0,5};
    c.initialize_from_vector((int)v.size(), v.data());
    return c.check_validity((int)v.size(), v.data());
}

// small valid n=10
bool test_small_valid_03()
{
    Circuit c(10);
    std::vector<int> v = {
        0,1,3, 2,4, 5,6, 0,7,
        8,9, 10,11, 0,12, 2,12, 10,12, 11,12
    };
    c.initialize_from_vector((int)v.size(), v.data());
    return c.check_validity((int)v.size(), v.data());
}

// basic valid circuit
bool test_basic_circuit_validity()
{
    std::vector<int> v = {
        0, 1,10, 2,11, 3,12, 4,10, 5,11,
        6,12, 7,10, 8,11, 9,12, 10,11
    };
    Circuit c(10);
    c.initialize_from_vector((int)v.size(), v.data());
    return c.check_validity((int)v.size(), v.data());
}

// /* 13 ─ mass balance converges */
// bool test_mass_balance_converges_valid()
// {
//     Circuit c(2);
//     int cv[5] = {0, 1,2, 3,4};
//     if (!c.check_validity(5, cv)) return false;
//     return c.run_mass_balance();
// }

// /* 14 ─ mass balance invalid */
// bool test_mass_balance_not_converge_small_iter()
// {
//     Circuit c(2);
//     int cv[5] = {0, 1,2, 3,4};
//     if (!c.check_validity(5, cv)) return true;          // 本身就无效也算通过
//     return !c.run_mass_balance(Constants::Simulation::DEFAULT_TOLERANCE, 1);
// }

/* ─────────── main ─────────── */
int main()
{
    struct TestItem { const char* name; bool (*fn)(); } tests[] = {
        {"manual demo",                           test_manual_demo},
        {"invalid length",                        test_invalid_length},
        {"feed points to terminal",               test_feed_points_to_terminal},
        {"conc out of range",                     test_conc_out_of_range},
        {"self-loop",                             test_invalid_self_loop},
        {"conc == tail",                          test_invalid_same_destination},
        {"unreachable unit",                      test_unreachable_unit},
        {"two terminals check",                   test_two_terminals_check},
        {"two-node cycle",                        test_two_node_cycle},
        {"basic valid (n=10)",                    test_basic_valid},
        {"small valid (n=4)",                     test_small_valid},
        {"small valid (n=3)",                     test_small_valid_02},
        {"small valid (n=10) #3",                 test_small_valid_03},
        {"basic circuit validity",                test_basic_circuit_validity},
        // {"mass balance converges",                test_mass_balance_converges_valid},
        // {"mass balance NOT converge (1 iter)",    test_mass_balance_not_converge_small_iter},
    };

    for (const auto& t : tests)
    {
        bool ok = t.fn();
        report(t.name, ok);
    }

    std::cout << "\n========== Summary ==========\n";
    if (fail_count == 0)
        std::cout << "All tests passed.\n";
    else
        std::cout << fail_count << " test(s) failed.\n";

    return fail_count == 0 ? 0 : 1;
}
