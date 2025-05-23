/** 
 * @file CSimulator.h
 * @brief C++ header file for the circuit simulator
 *
 * This header file defines the function that will be used to evaluate the
 * circuit and the parameters for the simulation
 */
#pragma once

#include <string>

// Structure to hold the simulation parameters
struct Simulator_Parameters
{
    // Convergence parameters
    double tolerance = 1e-6;
    int max_iterations = 1000;

    // Material properties
    double material_density = 3000.0; // kg/m³, density of all solid materials
    double solids_content = 0.1;      // Fraction of solids by volume

    // Rate constants (s⁻¹)
    double k_palusznium_high = 0.008;
    double k_palusznium_inter = 0.004;
    double k_gormanium_high = 0.004;
    double k_gormanium_inter = 0.002;
    double k_waste_high = 0.0005;
    double k_waste_inter = 0.00025;

    // Feed rates (kg/s)
    double feed_palusznium = 8.0;
    double feed_gormanium = 12.0;
    double feed_waste = 80.0;

    // Economic parameters (£/kg)
    double palusznium_value_in_palusznium_stream = 120.0;
    double gormanium_value_in_palusznium_stream = -20.0;
    double waste_penalty_in_palusznium_stream = -300.0;

    double palusznium_value_in_gormanium_stream = 0.0;
    double gormanium_value_in_gormanium_stream = 80.0;
    double waste_penalty_in_gormanium_stream = -25.0;

    // Unit volume parameters
    double fixed_unit_volume = 10.0;   // m³
    double min_unit_volume = 2.5;      // m³ (for variable case)
    double max_unit_volume = 20.0;     // m³ (for variable case)
    double max_circuit_volume = 150.0; // m³

    // Circuit operating cost parameters
    double cost_coefficient = 5.0;
    double volume_penalty_coefficient = 1000.0;

    // Visualization options
    bool generate_visualization = false;
    std::string visualization_file = "circuit.dot";
};

// Default simulation parameters
extern Simulator_Parameters default_simulator_parameters;

// Main circuit performance evaluation function
double circuit_performance(int vector_size, int* circuit_vector, int unit_parameters_size, double* unit_parameters,
                           Simulator_Parameters simulator_parameters);

// Overloaded functions for convenience
double circuit_performance(int vector_size, int* circuit_vector, int unit_parameters_size, double* unit_parameters);
double circuit_performance(int vector_size, int* circuit_vector, Simulator_Parameters simulator_parameters);
double circuit_performance(int vector_size, int* circuit_vector);

double circuit_performance(int vector_size, int* circuit_vector, bool testFlag);

double circuit_performance(int vector_size, int* circuit_vector, int unit_parameters_size, double* unit_parameters,
                           bool testFlag);