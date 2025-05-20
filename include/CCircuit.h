/**
 * @file CCircuit.h
 * @brief Declares the Circuit class – a mineral‑processing circuit
 * 
 * A circuit consists of a number of separation units (CUnit) connected
 * together. Each unit receives a mixed feed stream and produces two
 * output streams: a concentrate and a tailings stream.
 * 
 * The class stores:
 *  • The array of units in the circuit
 *  • The feed unit number and feed rates
 *  • The final product stream flow rates
 *  • Economic parameters
 * 
 */

#pragma once
#include "CUnit.h"
#include "constants.h"

#include "CCircuit.h"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <iomanip>

#include <vector>
#include <string>

// Constants for the circuit outlet destinations
// These values will be used in the circuit vector to indicate the final product streams
enum CircuitDestination {
    PALUSZNIUM_PRODUCT = -1,   // Final Palusznium concentrate product
    GORMANIUM_PRODUCT = -2,    // Final Gormanium concentrate product
    TAILINGS_OUTPUT = -3       // Final tailings output
};


/* ------------------------------------------------------------------ */
/*                         Circuit class                       */
/* ------------------------------------------------------------------ */

class Circuit {
  public:

    // The array of units in the circuit
    std::vector<CUnit> units;

    /* --------- volume information --------- */
    double *beta; // Array of beta values for unit volumes

    
    /* --------- flow information --------- */
    // Feed unit number and feed rates
    int feed_unit;
    double feed_palusznium_rate;  // kg/s
    double feed_gormanium_rate;   // kg/s
    double feed_waste_rate;       // kg/s
    
    // Final product stream flow rates
    double palusznium_product_palusznium; // kg/s
    double palusznium_product_gormanium;  // kg/s
    double palusznium_product_waste;      // kg/s
    
    double gormanium_product_palusznium;  // kg/s
    double gormanium_product_gormanium;   // kg/s
    double gormanium_product_waste;       // kg/s
    
    double tailings_palusznium;           // kg/s
    double tailings_gormanium;            // kg/s
    double tailings_waste;                // kg/s
    
    /* --------- economic parameters --------- */
    // Economic parameters
    double palusznium_value;      // £/kg in Palusznium stream
    double gormanium_value;       // £/kg in Gormanium stream
    double waste_penalty_palusznium; // £/kg waste in Palusznium stream
    double waste_penalty_gormanium;  // £/kg waste in Gormanium stream



    /* ----------------------------- Methods -------------------------------- */
    /**
     * @brief Constructor for the Circuit class
     * @param num_units Number of units in the circuit
     * @param beta Optional pointer to the beta values for unit volumes
     */
    Circuit(int num_units, double *beta = nullptr);
    
    // Initialize the circuit from a circuit vector
    /**
     * @brief Initialize the circuit from a circuit vector
     * @param vector_size Size of the circuit vector
     * @param circuit_vector Pointer to the circuit vector data
     * @param beta Optional pointer to the beta values for unit volumes
     * @return true if initialization is successful, false otherwise
     */
    bool initialize_from_vector(int vector_size, const int* circuit_vector, const double* beta = nullptr);
    
    // Check validity of a circuit vector
    static bool check_validity(int vector_size, const int* circuit_vector);
    static bool check_validity(int vector_size, const int* circuit_vector,
                              int unit_parameters_size, const double* unit_parameters);
    
    // Run a mass balance calculation on the circuit
    /**
     * @brief Run a mass balance calculation on the circuit
     * @param tolerance Convergence tolerance
     * @param max_iterations Maximum number of iterations
     * @return true if the mass balance converges, false otherwise
     */
    bool run_mass_balance(double tolerance = 1e-6, int max_iterations = 1000);
    
    // Get the economic value of the circuit
    /**
     * @brief Get the economic value of the circuit
     * @return The economic value of the circuit
     */
    double get_economic_value() const;
    
    /**
     * @brief Get the recovery of valuable materials
     * @return The recovery of valuable materials
     */
    double get_palusznium_recovery() const;

    /**
     * @brief Get the recovery of valuable materials
     * @return The recovery of valuable materials
     */
    double get_gormanium_recovery() const;
    
    /**
     * @brief Get the grade of valuable materials in products
     * @return The grade of valuable materials in products
     */
    double get_palusznium_grade() const;

    /**
     * @brief Get the grade of valuable materials in products
     * @return The grade of valuable materials in products
     */
    double get_gormanium_grade() const;
    
    /**
     * @brief Export the circuit to a dot file for visualization
     * @param filename The name of the output dot file
     * @return true if export is successful, false otherwise
     */
    bool export_to_dot(const std::string& filename) const;
    

    // Mark units that are accessible from a given unit (for validity checking)
    void mark_units(int unit_num);
    
    // Check if all units are accessible from the feed
    bool check_all_units_accessible() const;
    
    // Check if all units have routes to at least two output streams
    bool check_routes_to_outputs() const;
    
    // Check for self-recycle and other invalid configurations
    bool check_no_self_recycle() const;
    bool check_not_all_same_destination() const;
    
};

