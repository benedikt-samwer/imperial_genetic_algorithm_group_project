#include <vector>

#include <stdio.h>
#include <CUnit.h>
#include <CCircuit.h>

// Circuit::Circuit(int num_units){
//   this->units.resize(num_units);
// }

bool Circuit::check_validity(int vector_size, const int *circuit_vector)
{

  // Rewrite this to check the validity of the circuit vector
  // This is a dummy function that checks if the circuit vector is valid

  return true;
}

bool Circuit::check_validity(int vector_size, const int *circuit_vector,
                              int num_parameters, const double *parameters)
                              
{

  // Rewrite this to check the validity of the circuit vector
  // This is a dummy function that checks if the circuit vector is valid

  return true;
}




void Circuit::mark_units(int unit_num) {

  if (this->units[unit_num].mark) return;

  this->units[unit_num].mark = true;

  //If we have seen this unit already exit
  //Mark that we have now seen the unit

  //If conc_num does not point at a circuit outlet recursively call the function
  if (this->units[unit_num].conc_num<this->units.size()) {
    mark_units(this->units[unit_num].conc_num);
  } else {
    // ...Potentially do something to indicate that you have seen an exit
  }
  
  //If tails_num does not point at a circuit outletrecursively call the function 

  if (this->units[unit_num].tails_num<this->units.size()) {
    mark_units(this->units[unit_num].tails_num); 
  } else {
    // ...Potentially do something to indicate that you have seen an exit
  }
}




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
      waste_penalty_gormanium(Constants::Economic::WASTE_PENALTY_IN_GORMANIUM_STREAM)
{}

// Initialize the circuit from a circuit vector
bool Circuit::initialize_from_vector(int vector_size, const int* circuit_vector, const double* beta) {
    // num_units = n
    int num_units = (vector_size - 1) / 2;
    if (vector_size != 2 * num_units + 1) return false;
    units.resize(num_units);

    // feed_unit is the first element of the circuit vector
    feed_unit = circuit_vector[0];

    // Map the target units to corresponding unit numbers
    for (int i = 0; i < num_units; ++i) {
        int conc = circuit_vector[1 + 2 * i];
        int tails = circuit_vector[1 + 2 * i + 1];

        // transform the unit numbers from n, n+1, n+2 to -1, -2, -3
        if (conc == num_units)       conc = PALUSZNIUM_PRODUCT;   
        else if (conc == num_units+1) conc = GORMANIUM_PRODUCT;   
        else if (conc == num_units+2) conc = TAILINGS_OUTPUT;
        
        if (tails == num_units)       tails = PALUSZNIUM_PRODUCT; 
        else if (tails == num_units+1) tails = GORMANIUM_PRODUCT;
        else if (tails == num_units+2) tails = TAILINGS_OUTPUT;

        //TODO: here we can set the volume of the unit through beta
        // beta is a choosable parameter
        // if (beta) {
        //     units[i].update_volume(beta[i]);
        // }

        units[i] = CUnit(conc, tails);
    }
    return true;
}


// Mass balance calculation
bool Circuit::run_mass_balance(double tolerance, int max_iterations) {
    // Initialize feed for all the units
    for (auto& u : units) {
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
    std::cout << "Unit number: " << units.size() << std::endl;

    for (int iter = 0; iter < max_iterations; ++iter) {
        // std::cout << "\n==========Iteration" << iter + 1 << "==========\n\n";

        // Record the current feed
        // Record the current feed to last_feed and clear the current feed
        if(iter ==0){
            for (size_t i = 0; i < units.size(); ++i) {
                last_feed_p[i] = units[i].feed_palusznium;
                last_feed_g[i] = units[i].feed_gormanium;
                last_feed_w[i] = units[i].feed_waste;
                // clear the current feed
                units[i].feed_palusznium = 0.0;
                units[i].feed_gormanium = 0.0;
                units[i].feed_waste = 0.0;
            }
        }else{
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
            
            // std::cout << "  unit" << i << " feed (P,G,W): " << units[i].feed_palusznium << ", "
                    // << units[i].feed_gormanium << ", " << units[i].feed_waste << "\n";
            units[i].process();
            // std::cout << "  unit" << i << " concentrate flow (P,G,W): " << units[i].conc_palusznium << ", "
                    // << units[i].conc_gormanium << ", " << units[i].conc_waste << "\n";
            // std::cout << "  unit" << i << " tailings flow (P,G,W): " << units[i].tails_palusznium << ", "
                    // << units[i].tails_gormanium << ", " << units[i].tails_waste << "\n\n";
        }

        // This vector is used to mark whether the feed for each unit has been cleared
        // We need to make sure that the feed for each unit is cleared only once
        std::vector<bool> feedCleared(units.size(), false);

        // Initialize the product flow rates
        palusznium_product_palusznium = palusznium_product_gormanium = palusznium_product_waste = 0.0;
        gormanium_product_palusznium = gormanium_product_gormanium = gormanium_product_waste = 0.0;
        tailings_palusznium = tailings_gormanium = tailings_waste = 0.0;


        // std::cout<<"=====Distributing downstream data====="<<std::endl;
        for (size_t i = 0; i < units.size(); ++i) {
            // concentrate flow
            int concDest = units[i].conc_num;
            // std::cout << "  unit" << i << " concentrate flow to: " << concDest << std::endl;
            if (concDest == PALUSZNIUM_PRODUCT) {
                palusznium_product_palusznium += units[i].conc_palusznium;
                palusznium_product_gormanium += units[i].conc_gormanium;
                palusznium_product_waste += units[i].conc_waste;
            } else if (concDest == GORMANIUM_PRODUCT) {
                // std::cout << "  unit" << i << " concentrate flow to: GORMANIUM_PRODUCT" << std::endl;
                gormanium_product_palusznium += units[i].conc_palusznium;
                gormanium_product_gormanium += units[i].conc_gormanium;
                gormanium_product_waste += units[i].conc_waste;
            } else if (concDest == TAILINGS_OUTPUT) {
                tailings_palusznium += units[i].conc_palusznium;
                tailings_gormanium += units[i].conc_gormanium;
                tailings_waste += units[i].conc_waste;
            } else if (concDest >= 0 && concDest < (int)units.size()) {
                // std::cout << "  unit" << i << " concentrate flow to: " << concDest << std::endl;
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

            // std::cout << "  unit" << i << " tailings flow to: " << units[i].tails_num << std::endl;
            // std::cout<<"Tailings data"<<std::endl<<units[i].tails_palusznium << " " << units[i].tails_gormanium << std::endl;
            // std::cout<<units[i].tails_waste << std::endl;
            // tail flow
            int tailsDest = units[i].tails_num;
            if (tailsDest == PALUSZNIUM_PRODUCT) {
                palusznium_product_palusznium += units[i].tails_palusznium;
                palusznium_product_gormanium += units[i].tails_gormanium;
                palusznium_product_waste += units[i].tails_waste;
            } else if (tailsDest == GORMANIUM_PRODUCT) {
                // std::cout << "  unit" << i << " tailings flow to: GORMANIUM_PRODUCT" << std::endl;
                gormanium_product_palusznium += units[i].tails_palusznium;
                gormanium_product_gormanium += units[i].tails_gormanium;
                gormanium_product_waste += units[i].tails_waste;
                // std::cout<<"Tailings data"<<std::endl<<units[i].tails_palusznium << " " << units[i].tails_gormanium << std::endl;
                // std::cout<<units[i].tails_waste << std::endl;
            } else if (tailsDest == TAILINGS_OUTPUT) {
                tailings_palusznium += units[i].tails_palusznium;
                tailings_gormanium += units[i].tails_gormanium;
                tailings_waste += units[i].tails_waste;
            } else if (tailsDest >= 0 && tailsDest < (int)units.size()) {
                // std::cout << "  unit" << i << " non-tailings unit tailings flow to: " << tailsDest << std::endl;
                if (!feedCleared[tailsDest]) {
                    feedCleared[tailsDest] = true;
                    units[tailsDest].feed_palusznium = 0.0;
                    units[tailsDest].feed_gormanium = 0.0;
                    units[tailsDest].feed_waste = 0.0;
                }
                // std::cout<<"Tailings data"<<std::endl<<units[i].tails_palusznium << " " << units[i].tails_gormanium << std::endl;
                // std::cout<<units[i].tails_waste << std::endl;
                // std::cout<<"Ready to distribute!"<<std::endl;
                units[tailsDest].feed_palusznium += units[i].tails_palusznium;
                units[tailsDest].feed_gormanium += units[i].tails_gormanium;
                units[tailsDest].feed_waste += units[i].tails_waste;
                // std::cout<<"Tailings data"<<std::endl<<units[tailsDest].feed_palusznium << " " << units[tailsDest].feed_gormanium << std::endl;
                // std::cout<<units[tailsDest].feed_waste << std::endl;
            }
        }
 

        // convergence check
        double max_rel_change = 0.0;
        for (size_t i = 0; i < units.size(); ++i) {
            double rel_p = std::abs(units[i].feed_palusznium - last_feed_p[i]) / std::max(last_feed_p[i], 1e-12);
            double rel_g = std::abs(units[i].feed_gormanium - last_feed_g[i]) / std::max(last_feed_g[i], 1e-12);
            double rel_w = std::abs(units[i].feed_waste - last_feed_w[i]) / std::max(last_feed_w[i], 1e-12);
            max_rel_change = std::max({max_rel_change, rel_p, rel_g, rel_w});
        }


        // ---- debug output ----
        // for (size_t i = 0; i < units.size(); ++i) {
        //     std::cout << "  Unit" << i
        //             << " Last feed (P,G,W): " << last_feed_p[i] << ", "
        //             << last_feed_g[i] << ", " << last_feed_w[i]
        //             << " | Current feed (P,G,W): " << units[i].feed_palusznium << ", "
        //             << units[i].feed_gormanium << ", " << units[i].feed_waste << "\n";
        // }
        // std::cout << "  Palusznium product flow: " << palusznium_product_palusznium
        //         << "  Gormanium product flow: " << gormanium_product_gormanium
        //         << "  Tailings flow: " << tailings_waste << std::endl;
        // std::cout << "  max_rel_change = " << max_rel_change << std::endl;
        // ---- debug output ----
        if (max_rel_change < tolerance) return true;

        
    }
    return false; // not converged
}

// Calculate the economic value of the circuit
double Circuit::get_economic_value() const {
    double value = 0.0;
    // Palusznium product
    value += palusznium_product_palusznium * palusznium_value;
    value += palusznium_product_gormanium * -20.0;
    value += palusznium_product_waste * waste_penalty_palusznium;
    // Gormanium product 
    value += gormanium_product_palusznium * 0.0;
    value += gormanium_product_gormanium * gormanium_value;
    value += gormanium_product_waste * waste_penalty_gormanium;

    double total_volume = 0.0;
    for (const auto& u : units) total_volume += u.volume;
    double cost = 5.0 * std::pow(total_volume, 2.0/3.0);
    if (total_volume >= 150.0) {
        cost += 1000.0 * std::pow(total_volume - 150.0, 2.0);
    }
    value -= cost; // cost of the circuit
    std::cout<<"gormanium recovery: "<<(get_palusznium_recovery())*100<<"%"<<std::endl;
    std::cout<<"palusznium recovery: "<<(get_gormanium_recovery())*100<<"%"<<std::endl;

    std::cout<<"grade for palusznium: "<<(get_palusznium_grade())*100<<"%"<<std::endl;// debug output
    std::cout<<"grade for gormanium: "<<(get_gormanium_grade())*100<<"%"<<std::endl;// debug output
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
    if (total_feed < 1e-12) return 0.0;
    return recovered / total_feed;
}
double Circuit::get_gormanium_recovery() const {
    double total_feed = feed_gormanium_rate;
    double recovered =  gormanium_product_gormanium;
    if (total_feed < 1e-12) return 0.0;
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
    double total = palusznium_product_palusznium + palusznium_product_gormanium + palusznium_product_waste;
    return (total > 0) ? (palusznium_product_palusznium / total) : 0.0;
}
double Circuit::get_gormanium_grade() const {
    double total = gormanium_product_palusznium + gormanium_product_gormanium + gormanium_product_waste;
    return (total > 0) ? (gormanium_product_gormanium / total) : 0.0;
}

// Export the circuit to a dot file for visualization
bool Circuit::export_to_dot(const std::string& filename) const {
    std::ofstream ofs(filename);
    if (!ofs) return false;
    ofs << "digraph Circuit {\n";
    for (size_t i = 0; i < units.size(); ++i) {
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