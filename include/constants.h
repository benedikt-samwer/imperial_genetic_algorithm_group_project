/**
 * Header file for project-wide constants
 *
 * This file defines physical constants, economic parameters,
 * and other global values used throughout the project.
 */

#pragma once

namespace Constants {
namespace Test {
// Feed rates (kg/s)
constexpr double DEFAULT_PALUSZNIUM_FEED = 10.0;
constexpr double DEFAULT_GORMANIUM_FEED = 10.0;
constexpr double DEFAULT_WASTE_FEED = 10.0;

// Values in £/kg
constexpr double PALUSZNIUM_VALUE_IN_PALUSZNIUM_STREAM = 100.0;
constexpr double GORMANIUM_VALUE_IN_PALUSZNIUM_STREAM = 0.0;
constexpr double WASTE_PENALTY_IN_PALUSZNIUM_STREAM = 0.0;

constexpr double PALUSZNIUM_VALUE_IN_GORMANIUM_STREAM = 0.0;
constexpr double GORMANIUM_VALUE_IN_GORMANIUM_STREAM = 100.0;
constexpr double WASTE_PENALTY_IN_GORMANIUM_STREAM = 0.0;

// Operating cost parameters
constexpr double COST_COEFFICIENT = 5.0;
constexpr double VOLUME_PENALTY_COEFFICIENT = 1000.0;

// Material properties
constexpr double MATERIAL_DENSITY =
    3000.0;                            // kg/m³, density of all solid materials
constexpr double SOLIDS_CONTENT = 0.1; // Fraction of solids by volume

// Rate constants (s⁻¹)
constexpr double K_PALUSZNIUM = 0.008; // Rate constant for Palusznium
constexpr double K_GORMANIUM = 0.004;  // Rate constant for Gormanium
constexpr double K_WASTE = 0.0005;     // Rate constant for Waste

constexpr double DEFAULT_UNIT_VOLUME = 5.0;  // m³
constexpr double MIN_UNIT_VOLUME = 2.5;      // m³ (for variable case)
constexpr double MAX_UNIT_VOLUME = 20.0;     // m³ (for variable case)
constexpr double MAX_CIRCUIT_VOLUME = 150.0; // m³

constexpr int DEFAULT_NUM_UNITS = 10; // Default number of units in circuit

} // namespace Test

// Physical constants
namespace Physical {
// Material properties
constexpr double MATERIAL_DENSITY =
    3000.0;                            // kg/m³, density of all solid materials
constexpr double SOLIDS_CONTENT = 0.1; // Fraction of solids by volume

// Rate constants (s⁻¹)
constexpr double K_PALUSZNIUM = 0.008; // Rate constant for Palusznium
constexpr double K_GORMANIUM = 0.004;  // Rate constant for Gormanium
constexpr double K_WASTE = 0.0005;     // Rate constant for Waste
} // namespace Physical

// Economic parameters
namespace Economic {
// Values in £/kg
constexpr double PALUSZNIUM_VALUE_IN_PALUSZNIUM_STREAM = 120.0;
constexpr double GORMANIUM_VALUE_IN_PALUSZNIUM_STREAM = -20.0;
constexpr double WASTE_PENALTY_IN_PALUSZNIUM_STREAM = -300.0;

constexpr double PALUSZNIUM_VALUE_IN_GORMANIUM_STREAM = 0.0;
constexpr double GORMANIUM_VALUE_IN_GORMANIUM_STREAM = 80.0;
constexpr double WASTE_PENALTY_IN_GORMANIUM_STREAM = -25.0;

// Operating cost parameters
constexpr double COST_COEFFICIENT = 5.0;
constexpr double VOLUME_PENALTY_COEFFICIENT = 1000.0;
} // namespace Economic

// Default feed rates (kg/s)
namespace Feed {
constexpr double DEFAULT_PALUSZNIUM_FEED = 8.0;
constexpr double DEFAULT_GORMANIUM_FEED = 12.0;
constexpr double DEFAULT_WASTE_FEED = 80.0;
} // namespace Feed

// Circuit parameters
namespace Circuit {
constexpr double DEFAULT_UNIT_VOLUME = 10.0; // m³
constexpr double MIN_UNIT_VOLUME = 2.5;      // m³ (for variable case)
constexpr double MAX_UNIT_VOLUME = 20.0;     // m³ (for variable case)
constexpr double MAX_CIRCUIT_VOLUME = 150.0; // m³

constexpr int DEFAULT_NUM_UNITS = 10; // Default number of units in circuit
} // namespace Circuit

// Simulation parameters
namespace Simulation {
constexpr double DEFAULT_TOLERANCE = 1e-6;
constexpr int DEFAULT_MAX_ITERATIONS = 1000;
constexpr double MIN_FLOW_RATE =
    1e-6; // Minimum flow rate to prevent numerical issues
} // namespace Simulation

// Genetic algorithm parameters
namespace GA {
constexpr int DEFAULT_POPULATION_SIZE = 100;
constexpr int DEFAULT_MAX_GENERATIONS = 1000;
constexpr double DEFAULT_CROSSOVER_RATE = 0.8;
constexpr double DEFAULT_MUTATION_RATE = 0.01;
constexpr int DEFAULT_ELITE_COUNT = 1;
} // namespace GA
} // namespace Constants