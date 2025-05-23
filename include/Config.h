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
            if (key == "random_seed")
                p.random_seed = std::stoi(val);
            else if (key == "num_units")
                p.num_units = std::stoi(val);
            else if (key == "mode")
                p.mode = val; // d, c, or h
            else if (key == "max_iterations")
                p.max_iterations = std::stoi(val);
            else if (key == "population_size")
                p.population_size = std::stoi(val);
            else if (key == "elite_count")
                p.elite_count = std::stoi(val);
            else if (key == "tournament_size")
                p.tournament_size = std::stoi(val);
            else if (key == "selection_pressure")
                p.selection_pressure = std::stod(val);
            else if (key == "crossover_probability")
                p.crossover_probability = std::stod(val);
            else if (key == "crossover_points")
                p.crossover_points = std::stoi(val);
            else if (key == "mutation_probability")
                p.mutation_probability = std::stod(val);
            else if (key == "mutation_step_size")
                p.mutation_step_size = std::stoi(val);
            else if (key == "allow_mutation_wrapping")
                p.allow_mutation_wrapping = (val == "true" || val == "1");
            else if (key == "use_inversion")
                p.use_inversion = (val == "true" || val == "1");
            else if (key == "inversion_probability")
                p.inversion_probability = std::stod(val);
            else if (key == "use_scaling_mutation")
                p.use_scaling_mutation = (val == "true" || val == "1");
            else if (key == "scaling_mutation_prob")
                p.scaling_mutation_prob = std::stod(val);
            else if (key == "scaling_mutation_min")
                p.scaling_mutation_min = std::stod(val);
            else if (key == "scaling_mutation_max")
                p.scaling_mutation_max = std::stod(val);
            else if (key == "convergence_threshold")
                p.convergence_threshold = std::stod(val);
            else if (key == "stall_generations")
                p.stall_generations = std::stoi(val);
            else if (key == "verbose")
                p.verbose = (val == "true" || val == "1");
            else if (key == "log_results")
                p.log_results = (val == "true" || val == "1");
            else if (key == "log_file")
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