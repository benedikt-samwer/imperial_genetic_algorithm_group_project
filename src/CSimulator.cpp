#include "CUnit.h"
#include "CCircuit.h"
#include "CSimulator.h"
#include "constants.h"

#include <cmath>
#include <iostream>

Simulator_Parameters default_simulator_parameters = {};

// Compute performance of a circuit vector
double circuit_performance(int vector_size, int* circuit_vector,
                           int unit_parameters_size, double* unit_parameters,
                           Simulator_Parameters simulator_parameters) {
    
    int num_units = (vector_size - 1) / 2;
    Circuit circuit(num_units);

    // Step 1: Initialize the circuit from the vector
    if (!circuit.initialize_from_vector(vector_size, circuit_vector)) {
        return -1e9;  // Large negative penalty for invalid circuit
    }

    // Step 2: Run mass balance simulation
    if (!circuit.run_mass_balance(simulator_parameters.tolerance,
                                  simulator_parameters.max_iterations)) {
        return -1e9;  // Non-converging circuits are penalized heavily
    }

    // Step 3: Get the economic value of the circuit
    double value = circuit.get_economic_value();

    return value;
}


// overloads (delete if not needed)
double circuit_performance(int vector_size, int* circuit_vector,
                        int unit_parameters_size, double *unit_parameters){
  return circuit_performance(vector_size, circuit_vector,
                          unit_parameters_size, unit_parameters,
                          default_simulator_parameters);
};
double circuit_performance(int vector_size, int* circuit_vector){
    int num_parameters = (vector_size-1)/2;
    double *parameters;
    parameters = new double[num_parameters];
    for (int i=0; i<num_parameters; i++) {
        parameters[i] = 1.0;
    }
    return circuit_performance(vector_size, circuit_vector, 
                            num_parameters, parameters,
                            default_simulator_parameters);
};

// Other functions and variables to evaluate a real circuit.


