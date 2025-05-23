/**
 * @file CSimulator.cpp
 * @brief C++ source file for the circuit simulator
 * @author
 *
 * This source file contains the implementation of the function that will be
 * used to evaluate the circuit and the parameters for the simulation.
 *
 */
#include "CSimulator.h"
#include "CCircuit.h"
#include "CUnit.h"
#include <cmath>
#include <limits>

// Default simulation parameters
struct Simulator_Parameters default_simulator_parameters = {1e-6, 100};

/**
 * @brief Evaluate the circuit performance
 *
 * This function evaluates the performance of the circuit based on the
 * circuit vector and the unit parameters. It initializes the circuit,
 * runs the mass balance, and returns the economic value of the circuit.
 *
 * @param vector_size Size of the circuit vector
 * @param circuit_vector Circuit vector
 * @param unit_parameters_size Size of the unit parameters
 * @param unit_parameters Unit parameters
 * @param simulator_parameters Simulation parameters
 * @param testFlag Test flag to indicate whether to use test parameters
 *
 * @return Economic value of the circuit
 *
 */
double circuit_performance(int vector_size, int* circuit_vector,

                           int unit_parameters_size, double* unit_parameters,
                           struct Simulator_Parameters simulator_parameters, bool testFlag)
{

    // Calculate the number of units
    int num_units = (vector_size - 1) / 2;
    // Check if the vector size is valid
    if (vector_size != 2 * num_units + 1 || num_units <= 0)
    {
        // Invalid vector size
        return -1e12;
    }

    // Initialize the circuit
    Circuit circuit(num_units, unit_parameters, testFlag);
    if (!circuit.initialize_from_vector(vector_size, circuit_vector, unit_parameters, testFlag))
    {
        // Invalid structure
        return -1e12;
    }

    // Run the mass balance
    bool converged = circuit.run_mass_balance(simulator_parameters.tolerance, simulator_parameters.max_iterations);
    if (!converged)
    {
        // Not converged, consider invalid
        return -1e12;
    }

    // Return performance
    return circuit.get_economic_value();
}

// Overloads for other input

double circuit_performance(int vector_size, int* circuit_vector, int unit_parameters_size, double* unit_parameters)
{
    return circuit_performance(vector_size, circuit_vector, unit_parameters_size, unit_parameters,
                               default_simulator_parameters, false);
}

double circuit_performance(int vector_size, int* circuit_vector)
{
    int num_parameters = (vector_size - 1) / 2;
    double result =
        circuit_performance(vector_size, circuit_vector, num_parameters, nullptr, default_simulator_parameters, false);

    return result;
}

double circuit_performance(int vector_size, int* circuit_vector, int unit_parameters_size, double* unit_parameters,
                           bool testFlag)
{
    return circuit_performance(vector_size, circuit_vector, unit_parameters_size, unit_parameters,
                               default_simulator_parameters, testFlag);
}

double circuit_performance(int vector_size, int* circuit_vector, bool testFlag)
{
    int num_parameters = (vector_size - 1) / 2;
    double* parameters = new double[num_parameters];
    for (int i = 0; i < num_parameters; i++)
    {
        parameters[i] = 1.0;
    }
    double result = circuit_performance(vector_size, circuit_vector, num_parameters, nullptr,
                                        default_simulator_parameters, testFlag);

    // Clean up
    delete[] parameters;
    return result;
}
