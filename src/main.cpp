#include <iostream>

#include "CUnit.h"
#include "CCircuit.h"
#include "CSimulator.h"
#include "Genetic_Algorithm.h"


int main(int argc, char * argv[])
{

    // set things up
    int vector[11] = {0, 1, 2, 3, 3, 0, 2, 7, 5, 0, 6};
    double parameters[5] = {1.0, 0.0, 1.0, 1.0, 0.5};

    // run your code

    int return_code=optimize(11, vector, 5, parameters, circuit_performance);
    if (return_code==0) {
        std::cout << "Optimization successful!" << std::endl;
    } else {
        std::cout << "Optimization failed!" << std::endl;
    }
    // or
    // optimize(11, vector, circuit_performance, Circuit::check_validity)
    // etc.

    // generate final output, save to file, etc.
    std::cout << circuit_performance(11, vector, 5, parameters) << std::endl;

    for (int i = 0; i < sizeof(vector)/sizeof(int); i++) {
        std::cout << vector[i] << " ";
    }
    std::cout << std::endl;

    return return_code;
}
