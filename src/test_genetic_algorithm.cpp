#include <iostream>

#include "Genetic_Algorithm.h"


// This answer vector is used in the test function
int int_test_answer[11] = {2, 1, 1, 2, 0, 2, 3, 0, 4, 4, 1};

double real_test_answer[6] = {2.0, 1.0, 1.0, 2.0, 0.0, 2.0};

// mock test function for genetic algorithm
double test_function(int vector_size, int * vector) {

    double result = 0;
    for (int i = 0; i < vector_size; i++) {
        result += (vector[i]-int_test_answer[i])*(vector[i]-int_test_answer[i]);
        }

    return result;
    
    }

double real_test_function(int vector_size, double * vector) {

    double result = 0;
    for (int i = 0; i < vector_size; i++) {
        result += (vector[i]-real_test_answer[i])*(vector[i]-real_test_answer[i]);
        }

    return result;
    
    }


int main() {
    // set things up
    int vector1[] = {0,1,1,2,2,3,3,0,1, 0,4};

    double vector2[] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0};

    int return_code = 0;

    std::cout << "Checking optimum without validity checker. Vector length 11" << std::endl;

    // run your code
    return_code = optimize(sizeof(vector1), vector1, test_function);
    // you should test the quality of your answer here.

    if (return_code != 0) {
        std::cout << "Discrete optimization failed!" << std::endl;
        return return_code;
    }

    std::cout << "Optimum integer vector found." << std::endl;

    /*// Can test other versions

    return_code = optimize(sizeof(vector2), vector2, real_test_function);

    if (return_code != 0) {
        std::cout << "Discrete optimization failed!" << std::endl;
        return return_code;
    }
*/

    int optimize(int int_vector_size, int* int_vector,
             std::function<double(int, int*)> func) {
    return optimize(int_vector_size, int_vector, func, all_true_ints, DEFAULT_ALGORITHM_PARAMETERS);
}
    return 0;
}