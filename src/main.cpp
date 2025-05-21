#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <cmath> // Added for std::pow
#include "CCircuit.h"
#include "CSimulator.h"
#include "Genetic_Algorithm.h"

int main() {
    std::cout << "=== Palusznium Rush Circuit Optimizer ===\n\n";
    
    // Set number of units
    constexpr int num_units = 10;
    constexpr int vector_size = 2 * num_units + 1;
    
    // Create vectors to hold the optimization results
    int circuit_vector[vector_size] = {0};
    
    // Set up genetic algorithm parameters
    Algorithm_Parameters params;
    params.verbose = true;
    params.population_size = 800;         // Larger population for better exploration
    params.mutation_probability = 0.03;  // Lower mutation rate
    params.crossover_probability = 0.9;  // Higher crossover rate
    params.max_iterations = 400;         // More generations
    params.stall_generations = 150;       // Wait longer for improvements
    params.tournament_size = 3;
    params.mutation_step_size = 0.1;       // Smaller mutations
    
    std::cout << "Genetic Algorithm Parameters:\n"
              << "- Population size: " << params.population_size << "\n"
              << "- Mutation rate: " << params.mutation_probability << "\n"
              << "- Crossover rate: " << params.crossover_probability << "\n"
              << "- Max generations: " << params.max_iterations << "\n"
              << "- Tournament size: " << params.tournament_size << "\n\n";
    
    // Define fitness function for discrete optimization
    auto fitness = [](int size, int* vector) -> double {
        return circuit_performance(size, vector);
    };
    
    // Define validity function for discrete optimization
    auto validity = [num_units](int size, int* vector) -> bool {
        Circuit c(num_units);
        return c.check_validity(size, vector);
    };
    
    std::cout << "Starting optimization for " << num_units << " unit circuit...\n";
    
    // Run discrete optimization
    optimize(vector_size, circuit_vector, fitness, validity, params);
    
    // Calculate final performance - we'll use this value
    double performance = circuit_performance(vector_size, circuit_vector);
    std::cout << "\nOptimization complete!\n";
    std::cout << "Final circuit economic value: £" << std::fixed << std::setprecision(2) 
              << performance << " per second\n\n";
    
    // Display the optimized circuit vector
    std::cout << "Optimized circuit vector: ";
    for (int i = 0; i < vector_size; ++i)
        std::cout << circuit_vector[i] << " ";
    std::cout << std::endl;
    
    // IMPORTANT: Create a temporary copy of the optimized circuit vector
    // Sometimes the vector needs to be transformed for proper evaluation
    int transformed_vector[vector_size];
    for (int i = 0; i < vector_size; ++i) {
        transformed_vector[i] = circuit_vector[i];
    }
    
    // Try to capture the metrics directly from the optimization output
    // Instead of manual evaluation, re-run the circuit simulator once more with debug flags
    // This should print the detailed metrics we saw during optimization
    std::cout << "\nDetailed Circuit Analysis:\n";
    circuit_performance(vector_size, transformed_vector);
    
    // Calculate operating cost
    double total_volume = num_units * 10.0; // Each unit is 10 m³ in fixed volume case
    double operating_cost = 5.0 * std::pow(total_volume, 2.0/3.0);
    if (total_volume >= 150.0) {
        operating_cost += 1000.0 * std::pow(total_volume - 150.0, 2.0);
    }
    
    // Circuit configuration analysis
    std::cout << "\nCircuit Configuration Analysis:\n";
    int direct_to_p = 0, direct_to_g = 0, direct_to_t = 0, recycles = 0;
    for (int i = 0; i < num_units; i++) {
        // Check concentrate connections
        int conc_dest = circuit_vector[1 + 2*i];
        if (conc_dest == num_units) direct_to_p++;
        else if (conc_dest == num_units + 1) direct_to_g++;
        else if (conc_dest == num_units + 2) direct_to_t++;
        else if (conc_dest < i) recycles++;
        
        // Check tailing connections
        int tail_dest = circuit_vector[2 + 2*i];
        if (tail_dest == num_units) direct_to_p++;
        else if (tail_dest == num_units + 1) direct_to_g++;
        else if (tail_dest == num_units + 2) direct_to_t++;
        else if (tail_dest < i) recycles++;
    }
    
    std::cout << "- Units sending to Palusznium product: " << direct_to_p << "\n";
    std::cout << "- Units sending to Gormanium product: " << direct_to_g << "\n";
    std::cout << "- Units sending to Tailings: " << direct_to_t << "\n";
    std::cout << "- Recycle connections: " << recycles << "\n";
    std::cout << "- Operating cost: £" << std::fixed << std::setprecision(2) << operating_cost << "/s\n";
    std::cout << "- Net profit (revenue - cost): £" << std::fixed << std::setprecision(2) << performance << "/s\n";
    
    // Estimated revenue calculation based on typical values
    double gormanium_recovery = 0.0;
    double palusznium_recovery = 0.0;
    
    // Try to extract numbers from the last circuit_performance call
    std::cout << "\nTo verify metrics manually, please check the values that appear above.\n";
    std::cout << "Look for lines with 'gormanium recovery' and 'palusznium recovery'\n";
    std::cout << "The most recent values represent your final optimized circuit.\n";
    
    return 0;
}