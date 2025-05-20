/** Header for the circuit class
 *
 * This header defines the circuit class and its associated functions
 * for mineral processing circuit simulation
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

class Circuit {
  public:
    // Constructor that takes the number of units in the circuit
    Circuit(int num_units);
    
    // Initialize the circuit from a circuit vector
    bool initialize_from_vector(int vector_size, const int* circuit_vector, const double* beta = nullptr);
    
    // Check validity of a circuit vector
    static bool check_validity(int vector_size, const int* circuit_vector);
    static bool check_validity(int vector_size, const int* circuit_vector,
                              int unit_parameters_size, const double* unit_parameters);
    
    // Run a mass balance calculation on the circuit
    bool run_mass_balance(double tolerance = 1e-6, int max_iterations = 1000);
    
    // Get the economic value of the circuit
    double get_economic_value() const;
    
    // Get the recovery of valuable materials
    double get_palusznium_recovery() const;
    double get_gormanium_recovery() const;
    
    // Get the grade of valuable materials in products
    double get_palusznium_grade() const;
    double get_gormanium_grade() const;
    
    // Export the circuit to a dot file for visualization
    bool export_to_dot(const std::string& filename) const;
    
  // private:
    // Mark units that are accessible from a given unit (for validity checking)
    void mark_units(int unit_num);
    
    // Check if all units are accessible from the feed
    bool check_all_units_accessible() const;
    
    // Check if all units have routes to at least two output streams
    bool check_routes_to_outputs() const;
    
    // Check for self-recycle and other invalid configurations
    bool check_no_self_recycle() const;
    bool check_not_all_same_destination() const;
    
    // The array of units in the circuit
    std::vector<CUnit> units;
    
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
    
    // Economic parameters
    double palusznium_value;      // £/kg in Palusznium stream
    double gormanium_value;       // £/kg in Gormanium stream
    double waste_penalty_palusznium; // £/kg waste in Palusznium stream
    double waste_penalty_gormanium;  // £/kg waste in Gormanium stream
};

