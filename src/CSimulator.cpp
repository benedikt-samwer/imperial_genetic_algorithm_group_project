#include "CUnit.h"
#include "CCircuit.h"
#include "CSimulator.h"
#include <cmath>
#include <limits>

struct Simulator_Parameters default_simulator_parameters = {1e-6, 100};

// Compute performance of a circuit vector
double circuit_performance(int vector_size, int* circuit_vector,

                        int unit_parameters_size, double *unit_parameters,
                        struct Simulator_Parameters simulator_parameters) {

    // Calculate the number of units
    int num_units = (vector_size - 1) / 2;
    // Check if the vector size is valid
    if (vector_size != 2 * num_units + 1 || num_units <= 0) {
        // Invalid vector size
        return -1e12;
    }

    // Initialize the circuit
    Circuit circuit(num_units, unit_parameters);
    if (!circuit.initialize_from_vector(vector_size, circuit_vector, unit_parameters)) {
        // Invalid structure
        return -1e12;
    }

    // Run the mass balance
    bool converged = circuit.run_mass_balance(simulator_parameters.tolerance, simulator_parameters.max_iterations);
    if (!converged) {
        // Not converged, consider invalid
        return -1e12;
    }

    // Return performance
    return circuit.get_economic_value();
}

// Overloads for other input

double circuit_performance(int vector_size, int* circuit_vector,
                        int unit_parameters_size, double *unit_parameters){
    return circuit_performance(vector_size, circuit_vector,
                          unit_parameters_size, unit_parameters,
                          default_simulator_parameters);
}
double circuit_performance(int vector_size, int* circuit_vector){
    int num_parameters = (vector_size-1)/2;
    double *parameters = new double[num_parameters];
    for (int i=0; i<num_parameters; i++) {
        parameters[i] = 1.0;
    }
    double result = circuit_performance(vector_size, circuit_vector, 
                            num_parameters, parameters,
                            default_simulator_parameters);

    delete[] parameters;
    return result;
}