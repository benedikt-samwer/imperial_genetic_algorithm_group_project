#include <iostream>
#include <iomanip>
#include "CCircuit.h"
#include "CSimulator.h"
#include "CUnit.h"
#include <vector>
#include "Genetic_Algorithm.h"
#include "circuit_vector.h"
#include "constants.h"
#include <iostream>
#include <functional>


int main() {
    constexpr int num_units = 10;
    constexpr int vector_size = 2 * num_units + 1;
    int vector[vector_size] = {0};
    double dummy_params[5] = {1.0, 1.0, 1.0, 1.0, 1.0};


    auto fitness = [](int a, int* b, int c, double* d) {
        return 100.0;  // Dummy fitness value for testing
    };

    auto validity = [](int a, int* b, int c, double* d) {
        return true;   // Always valid for test
    };

    optimize(vector_size, vector, 5, dummy_params, fitness, validity);

    std::cout << "Tested vector: ";
    for (int i = 0; i < vector_size; ++i)
        std::cout << vector[i] << " ";
    std::cout << std::endl;

    return 0;
}
