#include <cmath>
#include <queue>
#include <vector>

#include <CCircuit.h>
#include <CUnit.h>
#include <cstdint>
#include <iostream>
#include <stdio.h>

Circuit::Circuit(int num_units) 
    : units(num_units), 
      feed_unit(0),
      feed_palusznium_rate(Constants::Feed::DEFAULT_PALUSZNIUM_FEED),
      feed_gormanium_rate(Constants::Feed::DEFAULT_GORMANIUM_FEED),
      feed_waste_rate(Constants::Feed::DEFAULT_WASTE_FEED),
      palusznium_product_palusznium(0.0), 
      palusznium_product_gormanium(0.0),
      palusznium_product_waste(0.0), 
      gormanium_product_palusznium(0.0),
      gormanium_product_gormanium(0.0),
      gormanium_product_waste(0.0),
      tailings_palusznium(0.0),
      tailings_gormanium(0.0),
      tailings_waste(0.0),
      palusznium_value(Constants::Economic::PALUSZNIUM_VALUE_IN_PALUSZNIUM_STREAM),
      gormanium_value(Constants::Economic::GORMANIUM_VALUE_IN_GORMANIUM_STREAM),
      waste_penalty_palusznium(Constants::Economic::WASTE_PENALTY_IN_PALUSZNIUM_STREAM),
      waste_penalty_gormanium(Constants::Economic::WASTE_PENALTY_IN_GORMANIUM_STREAM),
      beta(nullptr),
      circuit_vector(nullptr),
      n(num_units)
{
}

// 1. length check: length must be 2*n+1
// 2. feed check: feed cannot directly feed to terminal
// 3. index check: conc must be in (0, n+2), tail must be in (0, n+2)
// 4. no self-loop: conc cannot be equal to i, tail cannot be equal to i
// 5. same output: conc cannot be equal to tail
// 6. reachability check: all units must be reachable from feed
// 7. two terminals check: each unit must reach at least 2 different terminals
// 8. final terminal check: P1/P2, TA must be present
// 9. mass balance check: mass balance must converge
bool Circuit::check_validity(int vector_size, const int *vec) {
    // 1. length must be 2*n+1
    int expected = 2 * n + 1;
    if (vector_size != expected) {
        // std::cout << "❌[Validity] Length mismatch: expected " << expected << ", got " << vector_size << std::endl;
        return false;
    }

    // 2. feed check
    feed_dest = vec[0]; // feed points to the unit
    // feed cannot directly feed to terminal
    if (feed_dest < 0 || feed_dest >= n) {
        // std::cout << "❌[Validity] Feed destination " << feed_dest << " is invalid (must be 0 to " << (n-1) << ")" << std::endl;
        return false;
    }

    // read each unit's conc and tail and do static check
    struct Dest {
        int conc;
        int tail;
    };
    std::vector<Dest> dest(n);

    int max_idx = n + 2; // the last valid index

    for (int i = 0; i < n; ++i) {
        int conc = vec[1 + 2 * i]; // conc points to the unit
        int tail = vec[2 + 2 * i]; // tail points to the unit

        // 3. index check
        if (conc < 0 || conc > max_idx) {
            // std::cout << "❌[Validity] Unit " << i << " concentrate destination " << conc << " is out of range" << std::endl;
            return false;
        }
        if (tail < 0 || tail > max_idx) {
            // std::cout << "❌[Validity] Unit " << i << " tailings destination " << tail << " is out of range" << std::endl;
            return false;
        }

        // 4. no self-loop
        if (conc == i || tail == i) {
            // std::cout << "❌[Validity] Unit " << i << " has self-loop" << std::endl;
            return false;
        }

        // 5. same output
        if (conc == tail) {
            // std::cout << "❌[Validity] Unit " << i << " has same concentrate and tailings destination" << std::endl;
            return false;
        }

        dest[i] = {conc, tail};

        units[i].conc_num = conc;
        units[i].tails_num = tail;
        units[i].mark = false;
    }

    // 6. unit reachability check
    this->mark_units(feed_dest);

    for (int i = 0; i < n; ++i) {
        if (!units[i].mark) {
            // std::cout << "❌[Validity] Unit " << i << " is not reachable from feed" << std::endl;
            return false;
        }
    }

    // 7. two terminals check
    std::vector<int8_t> cache(n, -1);
    uint8_t global_mask = 0;
    for (int i = 0; i < n; ++i) {
        uint8_t mask = this->term_mask(i);
        global_mask |= mask;
        int cnt = (mask & 1) + ((mask >> 1) & 1) + ((mask >> 2) & 1);
        if (cnt < 2) {
            // std::cout << "❌[Validity] Unit " << i << " does not reach at least 2 different terminals" << std::endl;
            return false;
        }
    }

    // 8. final terminal check: P1/P2, TA must be present
    if ((global_mask & (0b001 | 0b010)) == 0) {
        // std::cout << "❌[Validity] Circuit does not reach any product streams (P1 or P2)" << std::endl;
        return false;
    }
    
    if ((global_mask & 0b100) == 0) {
        // std::cout << "❌[Validity] Circuit does not reach tailings stream" << std::endl;
        return false;
    }

    // check mass balance convergence
    if (!run_mass_balance(1e-6, 100)) {
        // std::cout << "❌[Validity] Mass balance did not converge" << std::endl;
        return false;
    }

    // std::cout << "✅[Validity] Circuit passed all validity checks" << std::endl;
    return true;
}
bool Circuit::check_validity(int vector_size, const int *circuit_vector,
                             int unit_parameters_size, double *unit_parameters)
{
  bool valid = check_validity(vector_size, circuit_vector);
  // check the validity of the circuit vector
  if (!valid) {
    // std::cout << "❌[Validity] Circuit vector is invalid" << std::endl;
    return false;
  }

  if (unit_parameters == nullptr) {
    // std::cout << "✅[Validity] Circuit vector is valid, no parameters to check" << std::endl;
    return valid;
  }

  // the length of the continuous parameters must be exactly = n units
  if (unit_parameters_size != n) {
    // std::cout << "❌[Parameters] Parameter length mismatch: expected " << n << ", got " << unit_parameters_size << std::endl;
    return false;
  }

  // each parameter must be in [0,1] (or other physical range)
  for (int i = 0; i < unit_parameters_size; ++i) {
    double beta = unit_parameters[i];
    if (beta < 0.0 || beta > 1.0 || std::isnan(beta)) {
      // std::cout << "❌[Parameters] β" << i << " = " << beta << " is out of range [0,1]" << std::endl;
      return false;
    }
  }

  // std::cout << "✅[Validity] Circuit vector and parameters are valid" << std::endl;
  return true; // legal
}

void Circuit::mark_units(int unit_num) {

  if (this->units[unit_num].mark)
    return;

  this->units[unit_num].mark = true;

  // If we have seen this unit already exit
  // Mark that we have now seen the unit

  // If conc_num does not point at a circuit outlet recursively call the
  // function
  if (this->units[unit_num].conc_num < this->units.size()) {
    mark_units(this->units[unit_num].conc_num);
  } else {
    // ...Potentially do something to indicate that you have seen an exit
  }

  // If tails_num does not point at a circuit outletrecursively call the
  // function

  if (this->units[unit_num].tails_num < this->units.size()) {
    mark_units(this->units[unit_num].tails_num);
  } else {
    // ...Potentially do something to indicate that you have seen an exit
  }
}

Circuit::Circuit(int num_units, double *beta)
    : units(num_units), feed_unit(0),

      feed_palusznium_rate(Constants::Feed::DEFAULT_PALUSZNIUM_FEED),
      feed_gormanium_rate(Constants::Feed::DEFAULT_GORMANIUM_FEED),
      feed_waste_rate(Constants::Feed::DEFAULT_WASTE_FEED),
      palusznium_product_palusznium(0.0), palusznium_product_gormanium(0.0),
      palusznium_product_waste(0.0), gormanium_product_palusznium(0.0),
      gormanium_product_gormanium(0.0), gormanium_product_waste(0.0),
      tailings_palusznium(0.0), tailings_gormanium(0.0), tailings_waste(0.0),
      beta(beta),
      palusznium_value(
          Constants::Economic::PALUSZNIUM_VALUE_IN_PALUSZNIUM_STREAM),
      gormanium_value(Constants::Economic::GORMANIUM_VALUE_IN_GORMANIUM_STREAM),
      waste_penalty_palusznium(
          Constants::Economic::WASTE_PENALTY_IN_PALUSZNIUM_STREAM),
      waste_penalty_gormanium(
          Constants::Economic::WASTE_PENALTY_IN_GORMANIUM_STREAM),
      palusznium_value_in_gormanium(Constants::Economic::PALUSZNIUM_VALUE_IN_GORMANIUM_STREAM
        ),
      gormanium_value_in_palusznium(Constants::Economic::GORMANIUM_VALUE_IN_PALUSZNIUM_STREAM)
       {}

Circuit::Circuit(int num_units, double *beta, bool testFlag)
    : units(num_units), feed_unit(0),

      feed_palusznium_rate(Constants::Feed::DEFAULT_PALUSZNIUM_FEED),
      feed_gormanium_rate(Constants::Feed::DEFAULT_GORMANIUM_FEED),
      feed_waste_rate(Constants::Feed::DEFAULT_WASTE_FEED),
      palusznium_product_palusznium(0.0), palusznium_product_gormanium(0.0),
      palusznium_product_waste(0.0), gormanium_product_palusznium(0.0),
      gormanium_product_gormanium(0.0), gormanium_product_waste(0.0),
      tailings_palusznium(0.0), tailings_gormanium(0.0), tailings_waste(0.0),
      beta(beta),
      palusznium_value(
          Constants::Economic::PALUSZNIUM_VALUE_IN_PALUSZNIUM_STREAM),
      gormanium_value(Constants::Economic::GORMANIUM_VALUE_IN_GORMANIUM_STREAM),
      waste_penalty_palusznium(
          Constants::Economic::WASTE_PENALTY_IN_PALUSZNIUM_STREAM),
      waste_penalty_gormanium(
          Constants::Economic::WASTE_PENALTY_IN_GORMANIUM_STREAM),
    palusznium_value_in_gormanium(Constants::Economic::PALUSZNIUM_VALUE_IN_GORMANIUM_STREAM
        ),
      gormanium_value_in_palusznium(Constants::Economic::GORMANIUM_VALUE_IN_PALUSZNIUM_STREAM)
           {
            std::cout<<"testFlag: "<<testFlag<<std::endl;
            if(testFlag) {
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

bool Circuit::initialize_from_vector(int vector_size,
                                     const int *circuit_vector) {
  return initialize_from_vector(vector_size, circuit_vector, nullptr, false);
}

bool Circuit::initialize_from_vector(int vector_size,
                                     const int *circuit_vector,
                                     const double *beta) {
  return initialize_from_vector(vector_size, circuit_vector, beta, false);
}

bool Circuit::initialize_from_vector(int vector_size,
                                     const int *circuit_vector,
                                     bool testFlag) {


  // Initialize the circuit from the circuit vector
  return initialize_from_vector(vector_size, circuit_vector, nullptr, testFlag);
}


// Initialize the circuit from a circuit vector
bool Circuit::initialize_from_vector(int vector_size, const int *circuit_vector,
                                     const double *beta, bool testFlag) {
  // num_units = n
  int num_units = (vector_size - 1) / 2;
  if (vector_size != 2 * num_units + 1)
    return false;
  units.resize(num_units);
  this->circuit_vector = circuit_vector;

  // feed_unit is the first element of the circuit vector
  feed_unit = circuit_vector[0];

  // Map the target units to corresponding unit numbers
  for (int i = 0; i < num_units; ++i) {
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

    // TODO: here we can set the volume of the unit through beta
    //  beta is a choosable parameter
    units[i] = CUnit(conc, tails, testFlag);
    if (beta != nullptr) {

      units[i].update_volume(beta[i]);
    }
  }
  return true;
}

// Mass balance calculation
bool Circuit::run_mass_balance(double tolerance, int max_iterations) {
  // Initialize feed for all the units
  for (auto &u : units) {
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

  for (int iter = 0; iter < max_iterations; ++iter) {
    // std::cout << "\n==========Iteration" << iter + 1 << "==========\n\n";

    // Record the current feed
    // Record the current feed to last_feed and clear the current feed
    if (iter == 0) {
      for (size_t i = 0; i < units.size(); ++i) {
        last_feed_p[i] = units[i].feed_palusznium;
        last_feed_g[i] = units[i].feed_gormanium;
        last_feed_w[i] = units[i].feed_waste;
        // clear the current feed
        units[i].feed_palusznium = 0.0;
        units[i].feed_gormanium = 0.0;
        units[i].feed_waste = 0.0;
      }
    } else {
      for (size_t i = 0; i < units.size(); ++i) {
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
    for (size_t i = 0; i < units.size(); ++i) {

      // std::cout << "  unit" << i << " feed (P,G,W): " <<
      // units[i].feed_palusznium << ", "
      // << units[i].feed_gormanium << ", " << units[i].feed_waste << "\n";
      units[i].process();
      // std::cout << "  unit" << i << " concentrate flow (P,G,W): " <<
      // units[i].conc_palusznium << ", "
      // << units[i].conc_gormanium << ", " << units[i].conc_waste << "\n";
      // std::cout << "  unit" << i << " tailings flow (P,G,W): " <<
      // units[i].tails_palusznium << ", "
      // << units[i].tails_gormanium << ", " << units[i].tails_waste << "\n\n";
    }

    // This vector is used to mark whether the feed for each unit has been
    // cleared We need to make sure that the feed for each unit is cleared only
    // once
    std::vector<bool> feedCleared(units.size(), false);

    // Initialize the product flow rates
    palusznium_product_palusznium = palusznium_product_gormanium =
        palusznium_product_waste = 0.0;
    gormanium_product_palusznium = gormanium_product_gormanium =
        gormanium_product_waste = 0.0;
    tailings_palusznium = tailings_gormanium = tailings_waste = 0.0;

    // std::cout<<"=====Distributing downstream data====="<<std::endl;
    for (size_t i = 0; i < units.size(); ++i) {
      // concentrate flow
      int concDest = units[i].conc_num;
      // std::cout << "  unit" << i << " concentrate flow to: " << concDest <<
      // std::endl;
      if (concDest == PALUSZNIUM_PRODUCT) {
        palusznium_product_palusznium += units[i].conc_palusznium;
        palusznium_product_gormanium += units[i].conc_gormanium;
        palusznium_product_waste += units[i].conc_waste;
      } else if (concDest == GORMANIUM_PRODUCT) {
        // std::cout << "  unit" << i << " concentrate flow to:
        // GORMANIUM_PRODUCT" << std::endl;
        gormanium_product_palusznium += units[i].conc_palusznium;
        gormanium_product_gormanium += units[i].conc_gormanium;
        gormanium_product_waste += units[i].conc_waste;
      } else if (concDest == TAILINGS_OUTPUT) {
        tailings_palusznium += units[i].conc_palusznium;
        tailings_gormanium += units[i].conc_gormanium;
        tailings_waste += units[i].conc_waste;
      } else if (concDest >= 0 && concDest < (int)units.size()) {
        // std::cout << "  unit" << i << " concentrate flow to: " << concDest <<
        // std::endl;
        if (!feedCleared[concDest]) {
          feedCleared[concDest] = true;
          units[concDest].feed_palusznium = 0.0;
          units[concDest].feed_gormanium = 0.0;
          units[concDest].feed_waste = 0.0;
        }
        units[concDest].feed_palusznium += units[i].conc_palusznium;
        units[concDest].feed_gormanium += units[i].conc_gormanium;
        units[concDest].feed_waste += units[i].conc_waste;
      }

      // std::cout << "  unit" << i << " tailings flow to: " <<
      // units[i].tails_num << std::endl; std::cout<<"Tailings
      // data"<<std::endl<<units[i].tails_palusznium << " " <<
      // units[i].tails_gormanium << std::endl; std::cout<<units[i].tails_waste
      // << std::endl; tail flow
      int tailsDest = units[i].tails_num;
      if (tailsDest == PALUSZNIUM_PRODUCT) {
        palusznium_product_palusznium += units[i].tails_palusznium;
        palusznium_product_gormanium += units[i].tails_gormanium;
        palusznium_product_waste += units[i].tails_waste;
      } else if (tailsDest == GORMANIUM_PRODUCT) {
        // std::cout << "  unit" << i << " tailings flow to: GORMANIUM_PRODUCT"
        // << std::endl;
        gormanium_product_palusznium += units[i].tails_palusznium;
        gormanium_product_gormanium += units[i].tails_gormanium;
        gormanium_product_waste += units[i].tails_waste;
        // std::cout<<"Tailings data"<<std::endl<<units[i].tails_palusznium << "
        // " << units[i].tails_gormanium << std::endl;
        // std::cout<<units[i].tails_waste << std::endl;
      } else if (tailsDest == TAILINGS_OUTPUT) {
        tailings_palusznium += units[i].tails_palusznium;
        tailings_gormanium += units[i].tails_gormanium;
        tailings_waste += units[i].tails_waste;
      } else if (tailsDest >= 0 && tailsDest < (int)units.size()) {
        // std::cout << "  unit" << i << " non-tailings unit tailings flow to: "
        // << tailsDest << std::endl;
        if (!feedCleared[tailsDest]) {
          feedCleared[tailsDest] = true;
          units[tailsDest].feed_palusznium = 0.0;
          units[tailsDest].feed_gormanium = 0.0;
          units[tailsDest].feed_waste = 0.0;
        }
        // std::cout<<"Tailings data"<<std::endl<<units[i].tails_palusznium << "
        // " << units[i].tails_gormanium << std::endl;
        // std::cout<<units[i].tails_waste << std::endl;
        // std::cout<<"Ready to distribute!"<<std::endl;
        units[tailsDest].feed_palusznium += units[i].tails_palusznium;
        units[tailsDest].feed_gormanium += units[i].tails_gormanium;
        units[tailsDest].feed_waste += units[i].tails_waste;
        // std::cout<<"Tailings
        // data"<<std::endl<<units[tailsDest].feed_palusznium << " " <<
        // units[tailsDest].feed_gormanium << std::endl;
        // std::cout<<units[tailsDest].feed_waste << std::endl;
      }
    }

    // convergence check
    double max_rel_change = 0.0;
    for (size_t i = 0; i < units.size(); ++i) {
      double rel_p = std::abs(units[i].feed_palusznium - last_feed_p[i]) /
                     std::max(last_feed_p[i], 1e-12);
      double rel_g = std::abs(units[i].feed_gormanium - last_feed_g[i]) /
                     std::max(last_feed_g[i], 1e-12);
      double rel_w = std::abs(units[i].feed_waste - last_feed_w[i]) /
                     std::max(last_feed_w[i], 1e-12);
      max_rel_change = std::max({max_rel_change, rel_p, rel_g, rel_w});
    }

    // ---- debug output ----
    // for (size_t i = 0; i < units.size(); ++i) {
    //     std::cout << "  Unit" << i
    //             << " Last feed (P,G,W): " << last_feed_p[i] << ", "
    //             << last_feed_g[i] << ", " << last_feed_w[i]
    //             << " | Current feed (P,G,W): " << units[i].feed_palusznium <<
    //             ", "
    //             << units[i].feed_gormanium << ", " << units[i].feed_waste <<
    //             "\n";
    // }
    // std::cout << "  Palusznium product flow: " <<
    // palusznium_product_palusznium
    //         << "  Gormanium product flow: " << gormanium_product_gormanium
    //         << "  Tailings flow: " << tailings_waste << std::endl;
    // std::cout << "  max_rel_change = " << max_rel_change << std::endl;
    // ---- debug output ----
    if (max_rel_change < tolerance)
      return true;
  }
  return false; // not converged
}

// Calculate the economic value of the circuit
double Circuit::get_economic_value() const {
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
  for (const auto &u : units)
    total_volume += u.volume;
  double cost = 5.0 * std::pow(total_volume, 2.0 / 3.0);
  if (total_volume >= 150.0) {
    cost += 1000.0 * std::pow(total_volume - 150.0, 2.0);
  }
  value -= cost; // cost of the circuit
  // std::cout<<"gormanium recovery:
  // "<<(get_palusznium_recovery())*100<<"%"<<std::endl; std::cout<<"palusznium
  // recovery: "<<(get_gormanium_recovery())*100<<"%"<<std::endl;

  // std::cout<<"grade for palusznium:
  // "<<(get_palusznium_grade())*100<<"%"<<std::endl;// debug output
  // std::cout<<"grade for gormanium:
  // "<<(get_gormanium_grade())*100<<"%"<<std::endl;// debug output
  return value;
}

// calculate the recovery of valuable materials
double Circuit::get_palusznium_recovery() const {
  // CUnit cal =  CUnit(-1,-1);
  // cal.feed_palusznium = feed_palusznium_rate;
  // cal.feed_gormanium = feed_gormanium_rate;
  // cal.feed_waste = feed_waste_rate;
  // cal.process();
  // std::cout<<"palusznium recovery: "<<cal.Rp<<std::endl;

  double total_feed = feed_palusznium_rate;
  double recovered = palusznium_product_palusznium;
  if (total_feed < 1e-12)
    return 0.0;
  return recovered / total_feed;
}
double Circuit::get_gormanium_recovery() const {
  double total_feed = feed_gormanium_rate;
  double recovered = gormanium_product_gormanium;
  if (total_feed < 1e-12)
    return 0.0;
  return recovered / total_feed;
  // CUnit cal =  CUnit(-1,-1);
  // cal.feed_palusznium = feed_palusznium_rate;
  // cal.feed_gormanium = feed_gormanium_rate;
  // cal.feed_waste = feed_waste_rate;
  // cal.process();
  // std::cout<<"gormanium recovery: "<<cal.Rg<<std::endl;
}

// Calculate the grade of valuable materials
double Circuit::get_palusznium_grade() const {
  double total = palusznium_product_palusznium + palusznium_product_gormanium +
                 palusznium_product_waste;
  return (total > 0) ? (palusznium_product_palusznium / total) : 0.0;
}
double Circuit::get_gormanium_grade() const {
  double total = gormanium_product_palusznium + gormanium_product_gormanium +
                 gormanium_product_waste;
  return (total > 0) ? (gormanium_product_gormanium / total) : 0.0;
}

// Export the circuit to a dot file for visualization
bool Circuit::export_to_dot(const std::string &filename) const {
  std::ofstream ofs(filename);
  if (!ofs)
    return false;
  ofs << "digraph Circuit {\n";
  for (size_t i = 0; i < units.size(); ++i) {
    ofs << "  unit" << i << " [label=\"Unit " << i << "\"];\n";
    // concentrate flow
    if (units[i].conc_num >= 0)
      ofs << "  unit" << i << " -> unit" << units[i].conc_num
          << " [label=\"conc\"];\n";
    else if (units[i].conc_num == PALUSZNIUM_PRODUCT)
      ofs << "  unit" << i << " -> palusznium_product [label=\"conc\"];\n";
    else if (units[i].conc_num == GORMANIUM_PRODUCT)
      ofs << "  unit" << i << " -> gormanium_product [label=\"conc\"];\n";
    else if (units[i].conc_num == TAILINGS_OUTPUT)
      ofs << "  unit" << i << " -> tailings [label=\"conc\"];\n";
    // tailings flow
    if (units[i].tails_num >= 0)
      ofs << "  unit" << i << " -> unit" << units[i].tails_num
          << " [label=\"tails\"];\n";
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

uint8_t Circuit::term_mask(int start) const {
  uint8_t mask = 0;
  std::vector<bool> visited(n, false);

  // Use queue for breadth-first search
  std::queue<int> q;
  q.push(start);
  visited[start] = true;

  while (!q.empty()) {
        int current = q.front();
        q.pop();

        const int conc_dest = units[current].conc_num;
        const int tail_dest = units[current].tails_num;

        process_destination(conc_dest, mask, visited, q);
        process_destination(tail_dest, mask, visited, q);

        if ((mask & (mask-1)) >= 3) break;
    }

    return mask;
}

void Circuit::process_destination(int dest, uint8_t& mask, 
                                 std::vector<bool>& visited,
                                 std::queue<int>& q) const {
    if (dest >= n) {
        if (dest == OUT_P1()) mask |= 0b001;
        else if (dest == OUT_P2()) mask |= 0b010;
        else if (dest == OUT_TA()) mask |= 0b100;
    } else {
        if (!visited[dest]) {
            visited[dest] = true;
            q.push(dest);
        }
    }
}


bool Circuit::save_all_units_to_csv(const std::string &filename) {
  std::ofstream ofs(filename, std::ios::app);
  if (!ofs.is_open()) {
    std::cerr << "Error: Unable to open file " << filename << std::endl;
    return false;
  }

  // output in a single line
  ofs << std::fixed << std::setprecision(2);

  for (size_t i = 0; i < units.size(); ++i) {
    const CUnit &unit = units[i];

    ofs << unit.conc_palusznium + unit.conc_gormanium + unit.conc_waste << ","
        << unit.tails_palusznium + unit.tails_gormanium + unit.tails_waste;

    if (i < units.size() - 1) {
      ofs << ",";
    }
  }
  ofs << "\n";

  ofs.close();
  return true;
}

bool Circuit::save_vector_to_csv(const std::string &filename) {
  std::ofstream ofs(filename, std::ios::app);
  if (!ofs.is_open()) {
    std::cerr << "Error: Unable to open file " << filename << std::endl;
    return false;
  }

  int length = static_cast<int>(units.size()) * 2 + 1;

  for (int i = 0; i < length; ++i) {
    ofs << circuit_vector[i];
    if (i < length - 1) {
      ofs << ",";
    }
  }
  ofs << "\n";

  ofs.close();
  return true;
}

bool Circuit::save_output_info(const std::string &filename) {
  if (!filename.empty()) {
    // Clear the file if it exists
    std::ofstream ofs(filename, std::ios::trunc);
    ofs.close();
  }
  return save_vector_to_csv(filename) && save_all_units_to_csv(filename);
}
