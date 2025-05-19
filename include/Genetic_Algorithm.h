/** Header for the Genetic Algorithm library
 *
*/

#pragma once

struct Algorithm_Parameters{
    int max_iterations;
    // other parameters for your algorithm       
};

#define DEFAULT_ALGORITHM_PARAMETERS Algorithm_Parameters{1000}

bool all_true(int int_vector_size, int* int_vector,
              int real_vector_size, double* real_vector);
bool all_true_ints(int int_vector_size, int* vector);
bool all_true_reals(int real_vector_size, double* vector);

int optimize(int int_vector_size, int* int_vector,
                int real_vector_size, double* real_vector,
                double (&func) (int, int*, int, double*),
                bool (&validity) (int, int*, int, double*)=all_true,
                struct Algorithm_Parameters algorithm_parameters=DEFAULT_ALGORITHM_PARAMETERS);

int optimize(int int_vector_size, int* int_vector,
                double (&func) (int, int*),
                bool (&validity) (int, int*)=all_true_ints,
                struct Algorithm_Parameters algorithm_parameters=DEFAULT_ALGORITHM_PARAMETERS);

int optimize(int real_vector_size, double* real_vector,
             double (&func) (int, double*),
             bool (&validity) (int, double*)=all_true_reals,
             struct Algorithm_Parameters algorithm_parameters=DEFAULT_ALGORITHM_PARAMETERS);

// Other functions and variables