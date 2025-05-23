/**
 * @file circuit_vector.h
 * @brief Circuit Vector Header
 *
 * This header defines the format and operations related to circuit vectors,
 * which describe the connections in a mineral processing circuit.
 *
 * A circuit vector has the following format:
 * [feed_unit, unit0_conc, unit0_waste, unit1_conc, unit1_waste, ...]
 *
 * where:
 * - feed_unit: Index of the unit receiving the circuit feed (0 to num_units-1)
 * - unitX_conc: Destination of the concentrate stream from unit X
 * - unitX_waste: Destination of the waste stream from unit X
 *
 * Destination can be:
 * - 0 to (num_units-1): Index of the unit receiving the stream
 * - PALUSZNIUM_PRODUCT (-1): Final Palusznium concentrate product
 * - GORMANIUM_PRODUCT (-2): Final Gormanium concentrate product
 * - TAILINGS_OUTPUT (-3): Final tailings output
 */

#pragma once

#include "CCircuit.h" // Include for CircuitDestination enum
#include <iostream>
#include <string>
#include <vector>

class CircuitVector
{
public:
    // Constructor for empty circuit vector
    CircuitVector() = default;

    // Constructor for circuit vector with specified number of units
    CircuitVector(int num_units);

    // Constructor from existing circuit vector data
    CircuitVector(int vector_size, const int* data);

    // Get number of units in the circuit
    int get_num_units() const;

    // Get feed unit
    int get_feed_unit() const;

    // Set feed unit
    void set_feed_unit(int unit);

    // Get concentrate destination for unit
    int get_concentrate_dest(int unit) const;

    // Get waste destination for unit
    int get_waste_dest(int unit) const;

    // Set concentrate destination for unit
    void set_concentrate_dest(int unit, int dest);

    // Set waste destination for unit
    void set_waste_dest(int unit, int dest);

    // Get the raw vector data
    const std::vector<int>& get_data() const;

    // Convert to raw array (for compatibility with existing functions)
    const int* data() const;

    // Get the size of the vector
    int size() const;

    // Randomize the circuit vector with valid values
    void randomize();

    // Print the circuit vector
    void print(std::ostream& os = std::cout) const;

    // Save to file
    bool save_to_file(const std::string& filename) const;

    // Load from file
    bool load_from_file(const std::string& filename);

private:
    std::vector<int> vector_data;
    int num_units = 0;
};