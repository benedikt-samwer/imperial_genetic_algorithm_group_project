#include <vector>
#include <queue>
#include <cmath>

#include <stdio.h>
#include <CUnit.h>
#include <CCircuit.h>
#include <iostream>
#include <cstdint>

Circuit::Circuit(int num_units){
  n = num_units;
  this->units.resize(num_units);
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
bool Circuit::check_validity(int vector_size, const int* vec)
{
    // 1. length must be 2*n+1
    int expected = 2 * n + 1;
    if (vector_size != expected) {
      // std::cout << "false 00" << std::endl;
      return false;
    }


    // 2. feed check
    feed_dest = vec[0];                      // feed points to the unit
    // feed cannot directly feed to terminal
    if (feed_dest < 0 || feed_dest >= n){
      // std::cout << "false 01" << std::endl;
      return false;
    }

          
    // read each unit's conc and tail and do static check
    struct Dest { int conc; int tail; };
    std::vector<Dest> dest(n);

    int max_idx = n + 2;                     // the last valid index

    for (int i = 0; i < n; ++i) {
      int conc = vec[1 + 2*i];                 // conc points to the unit
      int tail = vec[2 + 2*i];                 // tail points to the unit

      // 3. index check 
      if (conc < 0 || conc > max_idx) {
        // std::cout << "false 02" << std::endl;
        return false;
      }
      if (tail < 0 || tail > max_idx) {
        // std::cout << "false 03" << std::endl;
        return false;
      }

      // 4. no self-loop
      if (conc == i || tail == i) {
        // std::cout << "false 04" << std::endl;
        return false;
      }

      // 5. same output
      if (conc == tail) {
        // std::cout << "false 05" << std::endl;
        return false;
      }

      dest[i] = {conc, tail};

      units[i].conc_num = conc;
      units[i].tails_num = tail;
      units[i].mark      = false;
  }

    // 6. unit reachability check
    this->mark_units(feed_dest);

    for (int i = 0; i < n; ++i) {
        if (!units[i].mark) {         // R1 trigger
            // std::cout << "false 06\n";
            return false;
        }
    }

    // 7. two terminals check
    std::vector<int8_t> cache(n, -1);
    uint8_t global_mask = 0; 
    for (int i = 0; i < n; ++i) {
        // uint8_t mask = outlet_mask(i, cache); 
        uint8_t mask = this->term_mask(i);
        global_mask |= mask; 
        int cnt = (mask & 1) + ((mask >> 1) & 1) + ((mask >> 2) & 1);
        if (cnt < 2) { 
          // std::cout << "false 07\n"; 
          return false; 
        } // not reaching at least 2 different terminals
    }

    // 8. final terminal check
    if ( !(global_mask & 0b011) ) {            // no P1 or P2
        // std::cout << "false 07a (no P1/P2)\n";
        return false;
    }
    if ( !(global_mask & 0b100) ) {            // no TA
        // std::cout << "false 07b (no TA)\n";
        return false;
    }

    
    // 9. check mass balance conversgence
    if (!this->run_mass_balance(1e-6, 100)) {
        // std::cout << "false 99 (mass-balance diverge)\n";
        return false;
    }

    return true;    // legal
}

bool Circuit::check_validity(int vector_size, const int *circuit_vector,
                              int unit_parameters_size, double *unit_parameters)
                              
{
    bool valid = check_validity(vector_size, circuit_vector);
    // check the validity of the circuit vector
    if (!valid){
        // std::cout<<"false 00"<<std::endl;
        return false;
    }

    if (unit_parameters == nullptr) {
        return valid;
    }

    // If parameters are provided, check their size and values
    if (unit_parameters_size != n) {
        // std::cout << "false P0 (parameter length mismatch)" << std::endl;
        return false;
    }

    // each parameter must be in [0,1] (or other physical range)
    for (int i = 0; i < unit_parameters_size; ++i) {
        double beta = unit_parameters[i];
        if (beta < 0.0 || beta > 1.0 || std::isnan(beta)) {
            // std::cout << "false P1 (β out of range)" << std::endl;
            return false;
        }
    }
    std::cout<<"check_validity 4"<<std::endl;

    return true;          // legal

}

// void Circuit::mark_units(int unit_num) {

//   if (this->units[unit_num].mark) return;

//   this->units[unit_num].mark = true;

//   //If we have seen this unit already exit
//   //Mark that we have now seen the unit

//   //If conc_num does not point at a circuit outlet recursively call the function
//   if (this->units[unit_num].conc_num<this->units.size()) {
//     mark_units(this->units[unit_num].conc_num);
//   } else {
//     // ...Potentially do something to indicate that you have seen an exit
//   }
  
//   //If tails_num does not point at a circuit outletrecursively call the function 

//   if (this->units[unit_num].tails_num<this->units.size()) {
//     mark_units(this->units[unit_num].tails_num); 
//   } else {
//     // ...Potentially do something to indicate that you have seen an exit
//   }
// }

void Circuit::mark_units(int start_unit) {
    // 重置所有标记（必须在每次调用前执行）
    for (auto& unit : units) {
        unit.mark = false;
    }

    // 使用栈实现迭代式DFS（避免递归深度限制）
    std::stack<int> to_visit;
    to_visit.push(start_unit);

    while (!to_visit.empty()) {
        int u = to_visit.top();
        to_visit.pop();

        // 跳过已处理或无效单元
        if (u < 0 || u >= units.size() || units[u].mark) {
            continue;
        }

        // 标记当前单元
        units[u].mark = true;

        // 处理 conc 路径（仅限有效单元）
        int conc_dest = units[u].conc_num;
        if (conc_dest >= 0 && conc_dest < units.size()) {
            if (!units[conc_dest].mark) {
                to_visit.push(conc_dest);
            }
        }

        // 处理 tail 路径（仅限有效单元）
        int tail_dest = units[u].tails_num;
        if (tail_dest >= 0 && tail_dest < units.size()) {
            if (!units[tail_dest].mark) {
                to_visit.push(tail_dest);
            }
        }
    }
}

Circuit::Circuit(int num_units,double *beta)
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
      beta(beta),
      palusznium_value(Constants::Economic::PALUSZNIUM_VALUE_IN_PALUSZNIUM_STREAM),
      gormanium_value(Constants::Economic::GORMANIUM_VALUE_IN_GORMANIUM_STREAM),
      waste_penalty_palusznium(Constants::Economic::WASTE_PENALTY_IN_PALUSZNIUM_STREAM),
      waste_penalty_gormanium(Constants::Economic::WASTE_PENALTY_IN_GORMANIUM_STREAM)
{}

bool Circuit::initialize_from_vector(int vector_size, const int* circuit_vector) {
    return initialize_from_vector(vector_size, circuit_vector, nullptr);
}

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
        units[i] = CUnit(conc, tails);
        if (beta!= nullptr) {
         
            units[i].update_volume(beta[i]);
        }
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
    // std::cout << "Unit number: " << units.size() << std::endl;

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
        
        // Check outputs of current unit
        int conc_dest = units[current].conc_num;
        int tails_dest = units[current].tails_num;
        
        // Check concentrate output
        if (conc_dest >= n) {
            // Terminal output
            if (conc_dest == OUT_P1()) mask |= 1;      // Palusznium product
            else if (conc_dest == OUT_P2()) mask |= 2; // Gormanium product
            else if (conc_dest == OUT_TA()) mask |= 4; // Tailings
        } else if (!visited[conc_dest]) {
            // Continue search
            visited[conc_dest] = true;
            q.push(conc_dest);
        }
        
        // Check tailings output
        if (tails_dest >= n) {
            // Terminal output
            if (tails_dest == OUT_P1()) mask |= 1;      // Palusznium product
            else if (tails_dest == OUT_P2()) mask |= 2; // Gormanium product
            else if (tails_dest == OUT_TA()) mask |= 4; // Tailings
        } else if (!visited[tails_dest]) {
            // Continue search
            visited[tails_dest] = true;
            q.push(tails_dest);
        }
    }
    
    return mask;
}

