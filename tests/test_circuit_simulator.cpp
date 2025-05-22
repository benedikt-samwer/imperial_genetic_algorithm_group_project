
#include <cmath>
#include <iostream>

#include "CSimulator.h"

#include <vector>
#include <string>
#include <utility>

void run_test_case(const std::string& label, const std::vector<int>& circuit_vector) {
    std::cout << "\n=== Test Case: " << label << " ===\n";
    std::cout << "Circuit Vector: ";
    // double result = circuit_performance(static_cast<int>(circuit_vector.size()), const_cast<int*>(circuit_vector.data()));

    double result = circuit_performance(static_cast<int>(circuit_vector.size()), const_cast<int*>(circuit_vector.data()), false);
    std::cout << "Performance: " << result << " £/s\n";
    std::cout << "============================\n";
}
 
int main(int argc, char * argv[]) {
    Simulator_Parameters params = default_simulator_parameters;
 
    params.generate_visualization = false;
 
    std::vector<std::pair<std::string, std::vector<int>>> test_cases = {
        { "Minimal Working Circuit", {0, 1, 5, 2, 4, 3, 5} },
        { "All to Tailings (0% Recovery)", {0,3,1,3,2,3,5,4,7,6,3,3,8} },
        { "Misrouted Products", {0, 1, 4, 2, 5, 3, 4} },
        { "Long Circuit with Volume Penalty", {0, 1, 21, 2, 21, 3, 21, 4, 21, 5, 21, 6, 21, 7, 21, 8, 21, 9, 21, 10, 21, 11, 21, 12, 21, 13, 21, 14, 21, 15, 21, 16, 21, 17, 21, 18, 21, 19, 21, 20, 21} },
        { "No Convergence (Invalid loop)", {0, 0, 0, 0, 0, 0, 0} }
    };
 
    for (const auto& [label, vector] : test_cases) {
        run_test_case(label, vector);
    }
 
    return 0;
}