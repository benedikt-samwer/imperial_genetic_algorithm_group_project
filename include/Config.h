/**
 * @file Config.h
 * @brief Configuration file for the Genetic Algorithm
 * 
 * This file contains the definition of the Algorithm_Parameters structure
 * and the function to load parameters from a configuration file.
 * 
 * The Algorithm_Parameters structure holds various parameters for the
 * genetic algorithm.
 *
 */
#pragma once

#include "Genetic_Algorithm.h"
#include <fstream>
#include <iostream>
#include <string>

// Load GA parameters from a simple key=value text file
inline void load_parameters(const std::string& file, Algorithm_Parameters& p)
{
    std::ifstream in(file);
    if (!in)
    {
        std::cerr << "Warning: could not open " << file << " — using default parameters.\n";
        return;
    }
    // Read the file line by line
    std::string line;
    auto trim = [&](std::string& s)
    {
        const char* ws = " \t\n\r";
        size_t start = s.find_first_not_of(ws);
        size_t end = s.find_last_not_of(ws);
        s = (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    };
    while (std::getline(in, line))
    {
        // remove comments
        if (auto pos = line.find('#'); pos != std::string::npos)
            line.resize(pos);
        trim(line);
        if (line.empty())
            continue;
        // split key and value
        auto pos = line.find('=');
        if (pos == std::string::npos)
            continue;
        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 1);
        trim(key);
        trim(val);
        try
        {
            if (key == "random_seed") // Seed for the random number generator
                p.random_seed = std::stoi(val);
            else if (key == "num_units") // Number of units in the circuit
                p.num_units = std::stoi(val);
            else if (key == "mode") // Optimization mode: discrete, continuous, or hybrid
                p.mode = val; // d, c, or h
            else if (key == "max_iterations") // Maximum number of generations
                p.max_iterations = std::stoi(val);
            else if (key == "population_size") // Number of individuals in the population
                p.population_size = std::stoi(val);
            else if (key == "elite_count")  // Number of best individuals to keep unchanged
                p.elite_count = std::stoi(val);
            else if (key == "tournament_size") // Number of contenders per tournament
                p.tournament_size = std::stoi(val);
            else if (key == "selection_pressure") // Linear rank selection pressure parameter
                p.selection_pressure = std::stod(val);
            else if (key == "crossover_probability") // Probability of crossover
                p.crossover_probability = std::stod(val);
            else if (key == "crossover_points") // Number of crossover points (1 or 2)
                p.crossover_points = std::stoi(val);
            else if (key == "mutation_probability") // Probability of mutation per gene
                p.mutation_probability = std::stod(val);
            else if (key == "mutation_step_size") // Maximum change in value during mutation
                p.mutation_step_size = std::stoi(val);
            else if (key == "allow_mutation_wrapping") // Allow mutations to wrap around
                p.allow_mutation_wrapping = (val == "true" || val == "1");
            else if (key == "use_inversion") // Use inversion mutation
                p.use_inversion = (val == "true" || val == "1");
            else if (key == "inversion_probability") // Probability of inversion mutation
                p.inversion_probability = std::stod(val);
            else if (key == "use_scaling_mutation") // Use scaling mutation
                p.use_scaling_mutation = (val == "true" || val == "1");
            else if (key == "scaling_mutation_prob") // Probability of scaling mutation
                p.scaling_mutation_prob = std::stod(val);
            else if (key == "scaling_mutation_min") // Minimum scaling factor
                p.scaling_mutation_min = std::stod(val);
            else if (key == "scaling_mutation_max") // Maximum scaling factor
                p.scaling_mutation_max = std::stod(val);
            else if (key == "convergence_threshold") // Convergence threshold
                p.convergence_threshold = std::stod(val);
            else if (key == "stall_generations") // Max generations with no improvement
                p.stall_generations = std::stoi(val);
            else if (key == "verbose") // Print progress information
                p.verbose = (val == "true" || val == "1");
            else if (key == "log_results") // Log results to file
                p.log_results = (val == "true" || val == "1");
            else if (key == "log_file") // Log file name
                p.log_file = val;
            else
            {
                std::cerr << "Warning: unknown parameter '" << key << "' in " << file << "\n";
            }
        }
        catch (...)
        {
            std::cerr << "Warning: could not parse '" << key << "='" << val << "'\n";
        }
    }
}