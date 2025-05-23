/**
 * @file CCircuit.cpp
 * @brief Implementation of the Circuit class
 * 
 * This file contains the implementation of the Circuit class, which represents
 * a mineral-processing circuit. The class includes methods for checking
 * the validity of the circuit, marking units, running mass balance
 * calculations, and exporting the circuit to a dot file for visualization.
 * 
 */
#include <cmath>
#include <queue>
#include <vector>

#include <CCircuit.h>
#include <CUnit.h>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <stdio.h>

/**
 * @brief Constructor for the Circuit class
 * 
 */
Circuit::Circuit(int num_units)
    : units(num_units), feed_unit(0), feed_palusznium_rate(Constants::Feed::DEFAULT_PALUSZNIUM_FEED),
      feed_gormanium_rate(Constants::Feed::DEFAULT_GORMANIUM_FEED),
      feed_waste_rate(Constants::Feed::DEFAULT_WASTE_FEED), palusznium_product_palusznium(0.0),
      palusznium_product_gormanium(0.0), palusznium_product_waste(0.0), gormanium_product_palusznium(0.0),
      gormanium_product_gormanium(0.0), gormanium_product_waste(0.0), tailings_palusznium(0.0), tailings_gormanium(0.0),
      tailings_waste(0.0), palusznium_value(Constants::Economic::PALUSZNIUM_VALUE_IN_PALUSZNIUM_STREAM),
      gormanium_value(Constants::Economic::GORMANIUM_VALUE_IN_GORMANIUM_STREAM),
      waste_penalty_palusznium(Constants::Economic::WASTE_PENALTY_IN_PALUSZNIUM_STREAM),
      waste_penalty_gormanium(Constants::Economic::WASTE_PENALTY_IN_GORMANIUM_STREAM), beta(nullptr),
      circuit_vector(nullptr), n(num_units)
{
}

/**
 * @brief Check the validity of the circuit
 * 
 * This function checks the validity of the circuit vector by performing
 * various checks, including length check, feed check, index check,
 * self-loop check, same output check, reachability check, terminal check,
 * and mass balance convergence check. 
 * 
 * @param vector_size Size of the circuit vector
 * @param vec Circuit vector
 * 
 * @return true if the circuit is valid, false otherwise
 * 
 */
bool Circuit::check_validity(int vector_size, const int* vec)
{
  // 1. length must be 2*n+1
  int expected = 2 * n + 1;
  if (vector_size != expected) {
    return false;
  }

  // 2. feed check: feed cannot directly feed to terminal
  feed_dest = vec[0]; // feed points to the unit
  if (feed_dest < 0 || feed_dest >= n) {
    return false;
  }

  // read each unit's concentrate and tailing and do static check
  struct Dest {
    int conc;
    int tail;
  };
  std::vector<Dest> dest(n);

  int max_idx = n + 2; // the last valid index

  for (int i = 0; i < n; ++i) {
    int conc = vec[1 + 2 * i]; // conc points to the unit
    int tail = vec[2 + 2 * i]; // tail points to the unit

  // 3. index check: conc must be in (0, n+2), tail must be in (0, n+2)
    if (conc < 0 || conc > max_idx) {
      return false;
    }
    if (tail < 0 || tail > max_idx) {
      return false;
    }

    // 4. no self-loop: conc cannot be equal to i, tail cannot be equal to i
    if (conc == i || tail == i) {
      return false;
    }

    // 5. same output: conc cannot be equal to tail
    if (conc == tail) {
      return false;
    }

    dest[i] = {conc, tail};

    units[i].conc_num = conc;
    units[i].tails_num = tail;
    units[i].mark = false;
  }

  // 6. reachability check: all units must be reachable from feed
  this->mark_units(feed_dest);

  for (int i = 0; i < n; ++i) {
    if (!units[i].mark) {
      return false;
    }
  }

  // 7. two terminals check: each unit must reach at least 2 different terminals
  std::vector<int8_t> cache(n, -1);
  uint8_t global_mask = 0;
  for (int i = 0; i < n; ++i) {
    uint8_t mask = this->term_mask(i);
    global_mask |= mask;
    int cnt = (mask & 1) + ((mask >> 1) & 1) + ((mask >> 2) & 1);
    if (cnt < 2) {
      return false;
    }
  }

  // 8. final terminal check: P1/P2, TA must be present
  if ((global_mask & (0b001 | 0b010)) == 0) {
    return false;
  }

  if ((global_mask & 0b100) == 0) {
    return false;
  }

  // 9. mass balance check: mass balance must converge
  if (!run_mass_balance(1e-6, 100)) {
    return false;
  }

  return true;
}

/**
 * @brief Check the validity of the circuit vector and its parameters
 * 
 * This function checks the validity of the circuit vector and its parameters
 * by performing various checks, including length check, parameter range check,
 * and validity of the circuit vector.
 * 
 * @param vector_size Size of the circuit vector
 * @param circuit_vector Circuit vector
 * @param unit_parameters_size Size of the unit parameters
 * @param unit_parameters Unit parameters
 * 
 * @return true if the circuit vector and parameters are valid, false otherwise
 */
bool Circuit::check_validity(int vector_size, const int* circuit_vector, int unit_parameters_size,
                             double* unit_parameters)
{
  bool valid = check_validity(vector_size, circuit_vector);
  // check the validity of the circuit vector
  if (!valid) {
    return false;
  }
  // check the validity of the unit parameters
  if (unit_parameters == nullptr) {
    return valid;
  }

  // the length of the continuous parameters must be exactly n units
  if (unit_parameters_size != n) {
    return false;
  }

  // each parameter must be in [0,1] (or other physical range)
  for (int i = 0; i < unit_parameters_size; ++i) {
    double beta = unit_parameters[i];
    if (beta < 0.0 || beta > 1.0 || std::isnan(beta)) {
      return false;
    }
  }

  return true;
}

/**
 * @brief Mark the units in the circuit
 * 
 * This function marks the units in the circuit as visited. It recursively
 * traverses the circuit starting from the given unit number and marks each
 * unit as visited.
 * 
 * @param unit_num The unit number to start marking from
 */
void Circuit::mark_units(int unit_num)
{

    if (this->units[unit_num].mark)
        return;

    this->units[unit_num].mark = true;

    // If we have seen this unit already exit
    // Mark that we have now seen the unit

    // If conc_num does not point at a circuit outlet recursively call the
    // function
    if (this->units[unit_num].conc_num < this->units.size())
    {
        mark_units(this->units[unit_num].conc_num);
    }

    // If tails_num does not point at a circuit outletrecursively call the
    // function

    if (this->units[unit_num].tails_num < this->units.size())
    {
        mark_units(this->units[unit_num].tails_num);
    }
}

/**
 * @brief Constructor for the Circuit class
 * 
 * This constructor initializes the circuit with the given number of units
 * and a pointer to the beta array.
 * 
 * @param num_units Number of units in the circuit
 * @param beta Pointer to the beta array
 */
Circuit::Circuit(int num_units, double* beta)
    : units(num_units), feed_unit(0), n(num_units),

      feed_palusznium_rate(Constants::Feed::DEFAULT_PALUSZNIUM_FEED),
      feed_gormanium_rate(Constants::Feed::DEFAULT_GORMANIUM_FEED),
      feed_waste_rate(Constants::Feed::DEFAULT_WASTE_FEED), palusznium_product_palusznium(0.0),
      palusznium_product_gormanium(0.0), palusznium_product_waste(0.0), gormanium_product_palusznium(0.0),
      gormanium_product_gormanium(0.0), gormanium_product_waste(0.0), tailings_palusznium(0.0), tailings_gormanium(0.0),
      tailings_waste(0.0), beta(beta), palusznium_value(Constants::Economic::PALUSZNIUM_VALUE_IN_PALUSZNIUM_STREAM),
      gormanium_value(Constants::Economic::GORMANIUM_VALUE_IN_GORMANIUM_STREAM),
      waste_penalty_palusznium(Constants::Economic::WASTE_PENALTY_IN_PALUSZNIUM_STREAM),
      waste_penalty_gormanium(Constants::Economic::WASTE_PENALTY_IN_GORMANIUM_STREAM),
      palusznium_value_in_gormanium(Constants::Economic::PALUSZNIUM_VALUE_IN_GORMANIUM_STREAM),
      gormanium_value_in_palusznium(Constants::Economic::GORMANIUM_VALUE_IN_PALUSZNIUM_STREAM)
{
}

/**
 * @brief Constructor for the Circuit class
 * 
 * This constructor initializes the circuit with the given number of units,
 * a pointer to the beta array, and a test flag.
 * 
 * @param num_units Number of units in the circuit
 * @param beta Pointer to the beta array
 * @param testFlag Test flag to indicate whether to use test parameters
 */
Circuit::Circuit(int num_units, double* beta, bool testFlag)
    : units(num_units), feed_unit(0), n(num_units),

      feed_palusznium_rate(Constants::Feed::DEFAULT_PALUSZNIUM_FEED),
      feed_gormanium_rate(Constants::Feed::DEFAULT_GORMANIUM_FEED),
      feed_waste_rate(Constants::Feed::DEFAULT_WASTE_FEED), palusznium_product_palusznium(0.0),
      palusznium_product_gormanium(0.0), palusznium_product_waste(0.0), gormanium_product_palusznium(0.0),
      gormanium_product_gormanium(0.0), gormanium_product_waste(0.0), tailings_palusznium(0.0), tailings_gormanium(0.0),
      tailings_waste(0.0), beta(beta), palusznium_value(Constants::Economic::PALUSZNIUM_VALUE_IN_PALUSZNIUM_STREAM),
      gormanium_value(Constants::Economic::GORMANIUM_VALUE_IN_GORMANIUM_STREAM),
      waste_penalty_palusznium(Constants::Economic::WASTE_PENALTY_IN_PALUSZNIUM_STREAM),
      waste_penalty_gormanium(Constants::Economic::WASTE_PENALTY_IN_GORMANIUM_STREAM),
      palusznium_value_in_gormanium(Constants::Economic::PALUSZNIUM_VALUE_IN_GORMANIUM_STREAM),
      gormanium_value_in_palusznium(Constants::Economic::GORMANIUM_VALUE_IN_PALUSZNIUM_STREAM)
{
    if (testFlag)
    {
        this->feed_palusznium_rate = Constants::Test::DEFAULT_PALUSZNIUM_FEED;
        this->feed_gormanium_rate = Constants::Test::DEFAULT_GORMANIUM_FEED;
        this->feed_waste_rate = Constants::Test::DEFAULT_WASTE_FEED;

        this->palusznium_value = Constants::Test::PALUSZNIUM_VALUE_IN_PALUSZNIUM_STREAM;
        this->gormanium_value = Constants::Test::GORMANIUM_VALUE_IN_GORMANIUM_STREAM;
        this->waste_penalty_palusznium = Constants::Test::WASTE_PENALTY_IN_PALUSZNIUM_STREAM;
        this->waste_penalty_gormanium = Constants::Test::WASTE_PENALTY_IN_GORMANIUM_STREAM;
        this->palusznium_value_in_gormanium = Constants::Test::PALUSZNIUM_VALUE_IN_GORMANIUM_STREAM;
        this->gormanium_value_in_palusznium = Constants::Test::GORMANIUM_VALUE_IN_PALUSZNIUM_STREAM;
    }
}

/**
 * @brief Initialize the circuit from a circuit vector
 * 
 * This function initializes the circuit from a circuit vector. It takes
 * the size of the vector and the vector itself as input parameters.
 * 
 * @param vector_size Size of the circuit vector
 * @param circuit_vector Circuit vector
 * 
 * @return true if initialization is successful, false otherwise
 */
bool Circuit::initialize_from_vector(int vector_size, const int* circuit_vector)
{
    return initialize_from_vector(vector_size, circuit_vector, nullptr, false);
}

/**
 * @brief Initialize the circuit from a circuit vector
 * 
 * This function initializes the circuit from a circuit vector. It takes
 * the size of the vector, the vector itself, and a pointer to the beta
 * array as input parameters.
 * 
 * @param vector_size Size of the circuit vector
 * @param circuit_vector Circuit vector
 * @param beta Pointer to the beta array
 * 
 * @return true if initialization is successful, false otherwise
 */
bool Circuit::initialize_from_vector(int vector_size, const int* circuit_vector, const double* beta)
{
    return initialize_from_vector(vector_size, circuit_vector, beta, false);
}

/**
 * @brief Initialize the circuit from a circuit vector
 * 
 * This function initializes the circuit from a circuit vector. It takes
 * the size of the vector, the vector itself, and a test flag as input
 * parameters.
 * 
 * @param vector_size Size of the circuit vector
 * @param circuit_vector Circuit vector
 * @param testFlag Test flag to indicate whether to use test parameters
 * 
 * @return true if initialization is successful, false otherwise
 * 
 */
bool Circuit::initialize_from_vector(int vector_size, const int* circuit_vector, bool testFlag)
{

    // Initialize the circuit from the circuit vector
    return initialize_from_vector(vector_size, circuit_vector, nullptr, testFlag);
}

/**
 * @brief Initialize the circuit from a circuit vector
 * 
 * This function initializes the circuit from a circuit vector. It takes
 * the size of the vector, the vector itself, a pointer to the beta array,
 * and a test flag as input parameters.
 * 
 * @param vector_size Size of the circuit vector
 * @param circuit_vector Circuit vector
 * @param beta Pointer to the beta array
 * @param testFlag Test flag to indicate whether to use test parameters
 * 
 * @return true if initialization is successful, false otherwise
 */
bool Circuit::initialize_from_vector(int vector_size, const int* circuit_vector, const double* beta, bool testFlag)
{
    // num_units = n
    int num_units = (vector_size - 1) / 2;
    if (vector_size != 2 * num_units + 1)
        return false;
    units.resize(num_units);
    this->circuit_vector = circuit_vector;

    // feed_unit is the first element of the circuit vector
    feed_unit = circuit_vector[0];

    // Map the target units to corresponding unit numbers
    for (int i = 0; i < num_units; ++i)
    {
        int conc = circuit_vector[1 + 2 * i];
        int tails = circuit_vector[1 + 2 * i + 1];

        // transform the unit numbers from n, n+1, n+2 to -1, -2, -3
        if (conc == num_units)
            conc = PALUSZNIUM_PRODUCT;
        else if (conc == num_units + 1)
            conc = GORMANIUM_PRODUCT;
        else if (conc == num_units + 2)
            conc = TAILINGS_OUTPUT;

        if (tails == num_units)
            tails = PALUSZNIUM_PRODUCT;
        else if (tails == num_units + 1)
            tails = GORMANIUM_PRODUCT;
        else if (tails == num_units + 2)
            tails = TAILINGS_OUTPUT;

       
        units[i] = CUnit(conc, tails, testFlag);
        if (beta != nullptr)
        {

            units[i].update_volume(beta[i]);
        }
    }
    return true;
}

/**
 * @brief Run mass balance calculations for the circuit
 * 
 * This function runs mass balance calculations for the circuit. It takes
 * a tolerance and a maximum number of iterations as input parameters.
 * 
 * @param tolerance Tolerance for convergence
 * @param max_iterations Maximum number of iterations
 * 
 * @return true if mass balance converges, false otherwise
 */
bool Circuit::run_mass_balance(double tolerance, int max_iterations)
{
    // Initialize feed for all the units
    for (auto& u : units)
    {
        u.feed_palusznium = 0.0;
        u.feed_gormanium = 0.0;
        u.feed_waste = 0.0;
    }
    // Initialize feed for the first unit
    units[feed_unit].feed_palusznium = feed_palusznium_rate;
    units[feed_unit].feed_gormanium = feed_gormanium_rate;
    units[feed_unit].feed_waste = feed_waste_rate;

    // Record the last feed for each unit for convergence check
    std::vector<double> last_feed_p(units.size(), 0.0);
    std::vector<double> last_feed_g(units.size(), 0.0);
    std::vector<double> last_feed_w(units.size(), 0.0);
    // std::cout << "Unit number: " << units.size() << std::endl;

    for (int iter = 0; iter < max_iterations; ++iter)
    {

        // Record the current feed
        // Record the current feed to last_feed and clear the current feed
        if (iter == 0)
        {
            for (size_t i = 0; i < units.size(); ++i)
            {
                last_feed_p[i] = units[i].feed_palusznium;
                last_feed_g[i] = units[i].feed_gormanium;
                last_feed_w[i] = units[i].feed_waste;
                // clear the current feed
                units[i].feed_palusznium = 0.0;
                units[i].feed_gormanium = 0.0;
                units[i].feed_waste = 0.0;
            }
        }
        else
        {
            for (size_t i = 0; i < units.size(); ++i)
            {
                last_feed_p[i] = units[i].feed_palusznium;
                last_feed_g[i] = units[i].feed_gormanium;
                last_feed_w[i] = units[i].feed_waste;
            }
        }
        // Initialize feed for the first unit
        units[feed_unit].feed_palusznium = feed_palusznium_rate;
        units[feed_unit].feed_gormanium = feed_gormanium_rate;
        units[feed_unit].feed_waste = feed_waste_rate;

        // Process all units
        for (size_t i = 0; i < units.size(); ++i)
        {
            units[i].process();
        }

        // This vector is used to mark whether the feed for each unit has been
        // cleared We need to make sure that the feed for each unit is cleared only
        // once
        std::vector<bool> feedCleared(units.size(), false);

        // Initialize the product flow rates
        palusznium_product_palusznium = palusznium_product_gormanium = palusznium_product_waste = 0.0;
        gormanium_product_palusznium = gormanium_product_gormanium = gormanium_product_waste = 0.0;
        tailings_palusznium = tailings_gormanium = tailings_waste = 0.0;

        // "=====Distributing downstream data====="<<std::endl;
        for (size_t i = 0; i < units.size(); ++i)
        {
            // concentrate flow
            int concDest = units[i].conc_num;
            if (concDest == PALUSZNIUM_PRODUCT)
            {
                palusznium_product_palusznium += units[i].conc_palusznium;
                palusznium_product_gormanium += units[i].conc_gormanium;
                palusznium_product_waste += units[i].conc_waste;
            }
            else if (concDest == GORMANIUM_PRODUCT)
            {
                gormanium_product_palusznium += units[i].conc_palusznium;
                gormanium_product_gormanium += units[i].conc_gormanium;
                gormanium_product_waste += units[i].conc_waste;
            }
            else if (concDest == TAILINGS_OUTPUT)
            {
                tailings_palusznium += units[i].conc_palusznium;
                tailings_gormanium += units[i].conc_gormanium;
                tailings_waste += units[i].conc_waste;
            }
            else if (concDest >= 0 && concDest < (int)units.size())
            {
                if (!feedCleared[concDest])
                {
                    feedCleared[concDest] = true;
                    units[concDest].feed_palusznium = 0.0;
                    units[concDest].feed_gormanium = 0.0;
                    units[concDest].feed_waste = 0.0;
                }
                units[concDest].feed_palusznium += units[i].conc_palusznium;
                units[concDest].feed_gormanium += units[i].conc_gormanium;
                units[concDest].feed_waste += units[i].conc_waste;
            }

            // tailings flow
            int tailsDest = units[i].tails_num;
            if (tailsDest == PALUSZNIUM_PRODUCT)
            {
                palusznium_product_palusznium += units[i].tails_palusznium;
                palusznium_product_gormanium += units[i].tails_gormanium;
                palusznium_product_waste += units[i].tails_waste;
            }
            else if (tailsDest == GORMANIUM_PRODUCT)
            {
                gormanium_product_palusznium += units[i].tails_palusznium;
                gormanium_product_gormanium += units[i].tails_gormanium;
                gormanium_product_waste += units[i].tails_waste;

            }
            else if (tailsDest == TAILINGS_OUTPUT)
            {
                tailings_palusznium += units[i].tails_palusznium;
                tailings_gormanium += units[i].tails_gormanium;
                tailings_waste += units[i].tails_waste;
            }
            else if (tailsDest >= 0 && tailsDest < (int)units.size())
            {
                if (!feedCleared[tailsDest])
                {
                    feedCleared[tailsDest] = true;
                    units[tailsDest].feed_palusznium = 0.0;
                    units[tailsDest].feed_gormanium = 0.0;
                    units[tailsDest].feed_waste = 0.0;
                }

                units[tailsDest].feed_palusznium += units[i].tails_palusznium;
                units[tailsDest].feed_gormanium += units[i].tails_gormanium;
                units[tailsDest].feed_waste += units[i].tails_waste;

            }
        }

        // convergence check
        double max_rel_change = 0.0;
        for (size_t i = 0; i < units.size(); ++i)
        {
            double rel_p = std::abs(units[i].feed_palusznium - last_feed_p[i]) / std::max(last_feed_p[i], 1e-12);
            double rel_g = std::abs(units[i].feed_gormanium - last_feed_g[i]) / std::max(last_feed_g[i], 1e-12);
            double rel_w = std::abs(units[i].feed_waste - last_feed_w[i]) / std::max(last_feed_w[i], 1e-12);
            max_rel_change = std::max({max_rel_change, rel_p, rel_g, rel_w});
        }

        if (max_rel_change < tolerance)
            return true;
    }
    return false; // not converged
}

/**
 * @brief Get the economic value of the circuit
 * 
 * This function calculates the economic value of the circuit based on
 * the product flow rates and the values of the materials.
 * 
 * @return The economic value of the circuit
 * 
 */
double Circuit::get_economic_value() const
{
    double value = 0.0;

    // Palusznium product
    value += palusznium_product_palusznium * palusznium_value;
    value += palusznium_product_gormanium * gormanium_value_in_palusznium;
    value += palusznium_product_waste * waste_penalty_palusznium;

    // Gormanium product
    value += gormanium_product_gormanium * gormanium_value;
    value += gormanium_product_palusznium * palusznium_value_in_gormanium;
    value += gormanium_product_waste * waste_penalty_gormanium;

    double total_volume = 0.0;
    for (const auto& u : units)
        total_volume += u.volume;
    double cost = 5.0 * std::pow(total_volume, 2.0 / 3.0);
    if (total_volume >= 150.0)
    {
        cost += 1000.0 * std::pow(total_volume - 150.0, 2.0);
    }
    value -= cost; // cost of the circuit
    return value;
}

/**
 * @brief Get the recovery of valuable materials
 * 
 * This function calculates the recovery of valuable materials in the
 * circuit based on the product flow rates and the feed rates.
 * 
 * @return The recovery of valuable materials
 * 
 */
double Circuit::get_palusznium_recovery() const
{

    double total_feed = feed_palusznium_rate;
    double recovered = palusznium_product_palusznium;
    if (total_feed < 1e-12)
        return 0.0;
    return recovered / total_feed;
}

/**
 * @brief Get the recovery of gormanium
 * 
 * This function calculates the recovery of gormanium in the circuit
 * based on the product flow rates and the feed rates.
 * 
 * @return The recovery of gormanium
 * 
 */
double Circuit::get_gormanium_recovery() const
{
    double total_feed = feed_gormanium_rate;
    double recovered = gormanium_product_gormanium;
    if (total_feed < 1e-12)
        return 0.0;
    return recovered / total_feed;
}

/**
 * @brief Get the grade of palusznium
 * 
 * This function calculates the grade of palusznium in the circuit
 * based on the product flow rates.
 * 
 * @return The grade of palusznium
 * 
 */
double Circuit::get_palusznium_grade() const
{
    double total = palusznium_product_palusznium + palusznium_product_gormanium + palusznium_product_waste;
    return (total > 0) ? (palusznium_product_palusznium / total) : 0.0;
}

/**
 * @brief Get the grade of gormanium
 * 
 * This function calculates the grade of gormanium in the circuit
 * based on the product flow rates.
 * 
 * @return The grade of gormanium
 * 
 */
double Circuit::get_gormanium_grade() const
{
    double total = gormanium_product_palusznium + gormanium_product_gormanium + gormanium_product_waste;
    return (total > 0) ? (gormanium_product_gormanium / total) : 0.0;
}

/**
 * @brief Export the circuit to a DOT file
 * 
 * This function exports the circuit to a DOT file for visualization.
 * 
 * @param filename The name of the output DOT file
 * 
 * @return true if export is successful, false otherwise
 */
bool Circuit::export_to_dot(const std::string& filename) const
{
    std::ofstream ofs(filename);
    if (!ofs)
        return false;
    ofs << "digraph Circuit {\n";
    for (size_t i = 0; i < units.size(); ++i)
    {
        ofs << "  unit" << i << " [label=\"Unit " << i << "\"];\n";
        // concentrate flow
        if (units[i].conc_num >= 0)
            ofs << "  unit" << i << " -> unit" << units[i].conc_num << " [label=\"conc\"];\n";
        else if (units[i].conc_num == PALUSZNIUM_PRODUCT)
            ofs << "  unit" << i << " -> palusznium_product [label=\"conc\"];\n";
        else if (units[i].conc_num == GORMANIUM_PRODUCT)
            ofs << "  unit" << i << " -> gormanium_product [label=\"conc\"];\n";
        else if (units[i].conc_num == TAILINGS_OUTPUT)
            ofs << "  unit" << i << " -> tailings [label=\"conc\"];\n";
        // tailings flow
        if (units[i].tails_num >= 0)
            ofs << "  unit" << i << " -> unit" << units[i].tails_num << " [label=\"tails\"];\n";
        else if (units[i].tails_num == PALUSZNIUM_PRODUCT)
            ofs << "  unit" << i << " -> palusznium_product [label=\"tails\"];\n";
        else if (units[i].tails_num == GORMANIUM_PRODUCT)
            ofs << "  unit" << i << " -> gormanium_product [label=\"tails\"];\n";
        else if (units[i].tails_num == TAILINGS_OUTPUT)
            ofs << "  unit" << i << " -> tailings [label=\"tails\"];\n";
    }
    ofs << "  palusznium_product [shape=box, label=\"Palusznium Product\"];\n";
    ofs << "  gormanium_product [shape=box, label=\"Gormanium Product\"];\n";
    ofs << "  tailings [shape=box, label=\"Tailings\"];\n";
    ofs << "}\n";
    return true;
}

/**
 * @brief Get the terminal mask for a given unit
 * 
 * This function calculates the terminal mask for a given unit. It uses
 * breadth-first search to traverse the circuit and find the terminals.
 * 
 * @param start The starting unit number
 * 
 * @return The terminal mask
 */
uint8_t Circuit::term_mask(int start) const
{
    uint8_t mask = 0;
    std::vector<bool> visited(n, false);

    // Use queue for breadth-first search
    std::queue<int> q;
    q.push(start);
    visited[start] = true;

    while (!q.empty())
    {
        int current = q.front();
        q.pop();

        const int conc_dest = units[current].conc_num;
        const int tail_dest = units[current].tails_num;

        process_destination(conc_dest, mask, visited, q);
        process_destination(tail_dest, mask, visited, q);

        if ((mask & (mask - 1)) >= 3)
            break;
    }

    return mask;
}

/**
 * @brief Process the destination unit
 * 
 * This function processes the destination unit and updates the mask
 * accordingly. It also adds the destination unit to the queue for further
 * processing.
 * 
 * @param dest The destination unit number
 * @param mask The terminal mask
 * @param visited Vector to keep track of visited units
 * @param q Queue for breadth-first search
 */
void Circuit::process_destination(int dest, uint8_t& mask, std::vector<bool>& visited, std::queue<int>& q) const
{
    if (dest >= n)
    {
        if (dest == OUT_P1())
            mask |= 0b001;
        else if (dest == OUT_P2())
            mask |= 0b010;
        else if (dest == OUT_TA())
            mask |= 0b100;
    }
    else
    {
        if (!visited[dest])
        {
            visited[dest] = true;
            q.push(dest);
        }
    }
}

/**
 * @brief Save the circuit output information to a CSV file
 * 
 * This function saves the circuit output information to a CSV file.
 * It appends the data to the file if it already exists.
 * 
 * @param filename The name of the output CSV file
 * 
 * @return true if saving is successful, false otherwise
 */
bool Circuit::save_all_units_to_csv(const std::string& filename)
{
    std::ofstream ofs(filename, std::ios::app);
    if (!ofs.is_open())
    {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return false;
    }

    // output in a single line
    ofs << std::fixed << std::setprecision(2);

    for (size_t i = 0; i < units.size(); ++i)
    {
        const CUnit& unit = units[i];

        ofs << unit.conc_palusznium + unit.conc_gormanium + unit.conc_waste << ","
            << unit.tails_palusznium + unit.tails_gormanium + unit.tails_waste;

        if (i < units.size() - 1)
        {
            ofs << ",";
        }
    }
    ofs << "\n";

    ofs.close();
    return true;
}

/**
 * @brief Save the circuit output information to a CSV file
 * 
 * This function saves the circuit output information to a CSV file.
 * It appends the data to the file if it already exists.
 * 
 * @param filename The name of the output CSV file
 * 
 * @return true if saving is successful, false otherwise
 */
bool Circuit::save_vector_to_csv(const std::string& filename)
{
    std::ofstream ofs(filename, std::ios::app);
    if (!ofs.is_open())
    {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return false;
    }

    int length = static_cast<int>(units.size()) * 2 + 1;

    for (int i = 0; i < length; ++i)
    {
        ofs << circuit_vector[i];
        if (i < length - 1)
        {
            ofs << ",";
        }
    }
    ofs << "\n";

    ofs.close();
    return true;
}

/**
 * @brief Save the circuit output information to a CSV file
 * 
 * This function saves the circuit output information to a CSV file.
 * It appends the data to the file if it already exists.
 * 
 * @param filename The name of the output CSV file
 * 
 * @return true if saving is successful, false otherwise
 */
bool Circuit::save_output_info(const std::string& filename)
{
    namespace fs = std::filesystem; // If you see an error here it is because of the C++ version, it will compile fine
    fs::path p(filename);

    if (p.has_parent_path())
    {
        std::error_code ec;
        fs::create_directories(p.parent_path(), ec); // ok if it already exists
        if (ec)
        {
            std::cerr << "Cannot create directory " << p.parent_path() << " : " << ec.message() << '\n';
            return false;
        }
    }

    // truncate the file, then append the two blocks of data
    std::ofstream{filename, std::ios::trunc};
    return save_vector_to_csv(filename) && save_all_units_to_csv(filename);
}
