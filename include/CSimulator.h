/** header file for the circuit simulator
 * 
 * This header file defines the function that will be used to evaluate the circuit
*/

#pragma once

struct Simulator_Parameters{
    double tolerance;
    int max_iterations;
    // other parameters for your circuit simulator       
};

double circuit_performance(int vector_size, int *circuit_vector,
    int unit_parameters_size, double *unit_parameters,
    struct Simulator_Parameters simulator_parameters);
double circuit_performance(int vector_size, int *circuit_vector, struct Simulator_Parameters simulator_parameters);
double circuit_performance(int vector_size, int *circuit_vector,
    int unit_parameters_size, double *unit_parameters);
double circuit_performance(int vector_size, int *circuit_vector);
