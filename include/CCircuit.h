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
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

// Constants for the circuit outlet destinations
// These values will be used in the circuit vector to indicate the final product
// streams
enum CircuitDestination
{
    PALUSZNIUM_PRODUCT = -1, // Final Palusznium concentrate product
    GORMANIUM_PRODUCT = -2,  // Final Gormanium concentrate product
    TAILINGS_OUTPUT = -3     // Final tailings output
};

/* ------------------------------------------------------------------ */
/*                         Circuit class                       */
/* ------------------------------------------------------------------ */

class Circuit
{
public:
    // Constructor that takes the number of units in the circuit
    Circuit(int num_units);

    // Constructor with beta values for unit volumes
    Circuit(int num_units, double* beta);

    // Test constructor with beta values and test flag
    Circuit(int num_units, double* beta, bool testFlag);

    // Initialize the circuit from a circuit vector
    bool initialize_from_vector(int vector_size, const int* circuit_vector);
    bool initialize_from_vector(int vector_size, const int* circuit_vector, const double* beta);

    bool initialize_from_vector(int vector_size, const int* circuit_vector, bool testFlag);
    bool initialize_from_vector(int vector_size, const int* circuit_vector, const double* beta, bool testFlag);

    // Check validity of a circuit vector
    bool check_validity(int vector_size, const int* circuit_vector);
    bool check_validity(int vector_size, const int* circuit_vector, int unit_parameters_size, double* unit_parameters);

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

    /**
     * @brief Save all units to a CSV file.
     * @param filename The name of the output CSV file.
     * @return true if save is successful, false otherwise
     */
    bool save_all_units_to_csv(const std::string& filename);

    /**
     * @brief Save a vector to a CSV file.
     * @param filename The name of the output CSV file.
     * @return true if save is successful, false otherwise
     */
    bool save_vector_to_csv(const std::string& filename);

    /**
     * @brief Save the circuit data to a CSV file.
     * @param filename The name of the output CSV file.
     * @return true if save is successful, false otherwise
     */
    bool save_output_info(const std::string& filename);

private:
    // The array of units in the circuit
    std::vector<CUnit> units;

    // Circuit vector (for output)
    const int* circuit_vector;

    /* --------- volume information --------- */
    double* beta; // Array of beta values for unit volumes

    /* --------- flow information --------- */
    // Feed unit number and feed rates
    int feed_unit;
    double feed_palusznium_rate; // kg/s
    double feed_gormanium_rate;  // kg/s
    double feed_waste_rate;      // kg/s

    // Final product stream flow rates
    double palusznium_product_palusznium; // kg/s
    double palusznium_product_gormanium;  // kg/s
    double palusznium_product_waste;      // kg/s

    double gormanium_product_palusznium; // kg/s
    double gormanium_product_gormanium;  // kg/s
    double gormanium_product_waste;      // kg/s

    double tailings_palusznium; // kg/s
    double tailings_gormanium;  // kg/s
    double tailings_waste;      // kg/s

    /* --------- economic parameters --------- */
    // Economic parameters
    double palusznium_value;              // £/kg in Palusznium stream
    double gormanium_value;               // £/kg in Gormanium stream
    double gormanium_value_in_palusznium; // £/kg in Palusznium stream
    double palusznium_value_in_gormanium; // £/kg in Gormanium stream
    double waste_penalty_palusznium;      // £/kg waste in Palusznium stream
    double waste_penalty_gormanium;       // £/kg waste in Gormanium stream

    // Mark units that are accessible from a given unit (for validity checking)
    void mark_units(int unit_num);

    // Check if all units are accessible from the feed
    bool check_all_units_accessible() const;

    // Check if all units have routes to at least two output streams
    bool check_routes_to_outputs() const;

    // Check for self-recycle and other invalid configurations
    bool check_no_self_recycle() const;
    bool check_not_all_same_destination() const;

    /* ----------  Circuit validity checking ---------- */
    int n;
    int feed_dest = 0;
    uint8_t outlet_mask(int unit_idx, std::vector<int8_t>& cache) const;
    uint8_t term_mask(int start) const;
    void process_destination(int dest, uint8_t& mask, std::vector<bool>& visited, std::queue<int>& q) const;

    inline int OUT_P1() const
    {
        return n;
    } // palusznium
    inline int OUT_P2() const
    {
        return n + 1;
    } // gormanium
    inline int OUT_TA() const
    {
        return n + 2;
    } // tails
};
