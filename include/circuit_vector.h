/**
 * Header for circuit vector representation
 * 
 * This header defines the format and operations related to circuit vectors,
 * which describe the connections in a mineral processing circuit.
 */

#pragma once

#include <vector>
#include <string>
#include <iostream>

// Constants for the circuit destinations (product streams)
// These match the values in CCircuit.h but are duplicated here for convenience
enum CircuitDestination {
    PALUSZNIUM_PRODUCT = -1,  // Final Palusznium concentrate product
    GORMANIUM_PRODUCT = -2,   // Final Gormanium concentrate product  
    TAILINGS_OUTPUT = -3       // Final tailings output
};

/**
 * Class representing a circuit vector
 * 
 * A circuit vector has the following format:
 * [feed_unit, unit0_high, unit0_inter, unit0_tail, unit1_high, unit1_inter, unit1_tail, ...]
 * 
 * where:
 * - feed_unit: Index of the unit receiving the circuit feed (0 to num_units-1)
 * - unitX_high: Destination of the high-grade concentrate stream from unit X
 * - unitX_inter: Destination of the intermediate stream from unit X
 * - unitX_tail: Destination of the tailings stream from unit X
 * 
 * Destination can be:
 * - 0 to (num_units-1): Index of the unit receiving the stream
 * - PALUSZNIUM_PRODUCT (-1): Final Palusznium concentrate product
 * - GORMANIUM_PRODUCT (-2): Final Gormanium concentrate product
 * - TAILINGS_OUTPUT (-3): Final tailings output
 */
class CircuitVector {
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
    
    // Get high-grade destination for unit
    int get_high_grade_dest(int unit) const;
    
    // Get intermediate destination for unit
    int get_intermediate_dest(int unit) const;
    
    // Get tailings destination for unit
    int get_tailings_dest(int unit) const;
    
    // Set high-grade destination for unit
    void set_high_grade_dest(int unit, int dest);
    
    // Set intermediate destination for unit
    void set_intermediate_dest(int unit, int dest);
    
    // Set tailings destination for unit
    void set_tailings_dest(int unit, int dest);
    
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