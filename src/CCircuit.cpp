#include <vector>
#include <queue>
#include <cmath>

#include <stdio.h>
#include <CUnit.h>
#include <CCircuit.h>
#include <iostream>

Circuit::Circuit(int num_units){
  n = num_units;
  this->units.resize(num_units);
}

bool Circuit::check_validity(int vector_size, const int* vec)
{
    // length must be 2*n+1
    int expected = 2 * n + 1;
    if (vector_size != expected) {
      std::cout << "false 00" << std::endl;
      return false;
    }

    // parse feed
    feed_dest = vec[0];                      // feed points to the unit
    // feed cannot directly feed to terminal
    if (feed_dest < 0 || feed_dest >= n){
      std::cout << "false 01" << std::endl;
      return false;
    }
        
    // read each unit's conc and tail and do static check
    struct Dest { int conc; int tail; };
    std::vector<Dest> dest(n);

    int max_idx = n + 2;                     // the last valid index

    for (int i = 0; i < n; ++i) {
      int conc = vec[1 + 2*i];                 // conc points to the unit
      int tail = vec[2 + 2*i];                 // tail points to the unit

      // R5 vector elements must be in (0, n+2)
      if (conc < 0 || conc > max_idx) {
        std::cout << "false 02" << std::endl;
        return false;
      }
      if (tail < 0 || tail > max_idx) {
        std::cout << "false 03" << std::endl;
        return false;
      }

      // R3 no self-loop
      if (conc == i || tail == i) {
        std::cout << "false 04" << std::endl;
        return false;
      }

      // R4 all outputs point to the same element
      if (conc == tail) {
        std::cout << "false 05" << std::endl;
        return false;
      }

      dest[i] = {conc, tail};

      units[i].conc_num = conc;
      units[i].tails_num = tail;
      units[i].mark      = false;
  }

    // R1 reachability check
    mark_units(feed_dest);

    for (int i = 0; i < n; ++i) {
        if (!units[i].mark) {         // R1 trigger
            std::cout << "false 06\n";
            return false;
        }
    }

    // R2 each unit must reach at least 2 different terminals
    std::vector<int8_t> cache(n, -1);
    for (int i = 0; i < n; ++i) {
        // uint8_t mask = outlet_mask(i, cache); 
        uint8_t mask = term_mask(i);
        int cnt = (mask & 1) + ((mask >> 1) & 1) + ((mask >> 2) & 1);
        if (cnt < 2) { std::cout << "false 07\n"; return false; }
    }

    // R6 detect cycle of length ≥2 (self-loop is prohibited)
    // 0 = white, 1 = gray, 2 = black （white: not visited，gray: visited but not explored，black: visited and explored）
    // std::vector<char> color(n, 0);


    // auto has_cycle = [&](auto&& self, int u) -> bool
    // {
    //     // visit the gray node again, it means this node along the descendants back to itself, forming a cycle
    //     if (color[u] == 1) {
    //       std::cout << "false 08" << std::endl;
    //       return true;
    //     } 
    //     // the node has been fully explored, no cycle in the subtree
    //     if (color[u] == 2) {
    //       return false;
    //     }
    //     color[u] = 1; // mark as gray
    //     // recursively check the subtree, along conc and tail recursively call, and find a cycle through its some descendant
    //     if (dest[u].conc < n && self(self, dest[u].conc)) {
    //       std::cout << "false 09" << std::endl;
    //       return true;
    //     }
    //     if (dest[u].tail < n && self(self, dest[u].tail)) {
    //       std::cout << "false 11" << std::endl;
    //       return true;
    //     }
    //     color[u] = 2;                     // mark black
    //     return false;
    // };

    // for (int i = 0; i < n; ++i)
    //     if (has_cycle(has_cycle, i)) {
    //       return false;
    //     }
    
    // check mass balance convergence
    if (!mass_balance_converges(1e-6, 2000)) {
        std::cout << "false 99 (mass-balance diverge)\n";
        return false;
    }

    return true;    // legal
}

bool Circuit::check_validity(int vector_size, int *circuit_vector,
                              int num_parameters, double *parameters)
                              
{
    // check the validity of the circuit vector
    if (!check_validity(vector_size,
                        static_cast<const int*>(circuit_vector)))
        return false;

    // the length of the continuous parameters must be exactly = n units
    if (num_parameters != n) {
        std::cout << "false P0 (parameter length)" << std::endl;
        return false;
    }

    // each parameter must be in [0,1] (or other physical range)
    for (int i = 0; i < num_parameters; ++i) {
        double beta = parameters[i];
        if (beta < 0.0 || beta > 1.0 || std::isnan(beta)) {
            std::cout << "false P1 (β out of range)" << std::endl;
            return false;
        }
    }

    return true;          // legal

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
  
  //If tails_num does not point at a circuit outlet recursively call the function 

  if (this->units[unit_num].tails_num<this->units.size()) {
    mark_units(this->units[unit_num].tails_num); 
  } else {
    // ...Potentially do something to indicate that you have seen an exit
  }
}

bool Circuit::mass_balance_converges(double tol, int maxIter) const
{
    using namespace Constants;
    
    // physical constants & rate constants
    const double rho = Physical::MATERIAL_DENSITY;    // ρ = 3000 kg/m³
    const double phi = Physical::SOLIDS_CONTENT;      // φ = 0.10
    const double V = Constants::Circuit::DEFAULT_UNIT_VOLUME; // V = 10 m³
    
    const double kP  = Physical::K_PALUSZNIUM;  // 0.008 s⁻¹
    const double kG  = Physical::K_GORMANIUM;   // 0.004 s⁻¹
    const double kW  = Physical::K_WASTE;       // 0.0005 s⁻¹

    // default tolerance & max iterations
    if (tol     <= 0.0) tol     = Simulation::DEFAULT_TOLERANCE;
    if (maxIter <= 0  ) maxIter = Simulation::DEFAULT_MAX_ITERATIONS;
    
    // index convention
    const int NS = n + 3;                // n units + 3 terminals
    
    // initialize flow rates (kg/s)
    std::vector<double> FP(NS, 0.0), FW(NS, 0.0), FG(NS, 0.0);   // current iteration
    std::vector<double> FPn(NS, 0.0), FWn(NS, 0.0), FGn(NS, 0.0); // next iteration
    
    FP[feed_dest] = Feed::DEFAULT_PALUSZNIUM_FEED;   // 8 kg/s
    FG[feed_dest] = Feed::DEFAULT_GORMANIUM_FEED;    // 12 kg/s
    FW[feed_dest] = Feed::DEFAULT_WASTE_FEED;       // 80 kg/s
    
    // mass balance iteration
    for (int it = 0; it < maxIter; ++it)
    {
        // Reset next iteration flow rates
        std::fill(FPn.begin(), FPn.end(), 0.0);
        std::fill(FWn.begin(), FWn.end(), 0.0);
        std::fill(FGn.begin(), FGn.end(), 0.0);
        
        for (int u = 0; u < n; ++u)
        {
            double FP_in = FP[u];
            double FG_in = FG[u];
            double FW_in = FW[u];
            
            // Skip if no material entering this unit
            if (FP_in == 0.0 && FG_in == 0.0 && FW_in == 0.0) continue;
            
            // calculate residence time τ
            double total_solids = FP_in + FG_in + FW_in;
            double Qv = total_solids / rho / phi;      // m³/s
            if (Qv < 1e-12) Qv = 1e-12;               // avoid division by zero
            double tau = V / Qv;                      // s
            
            // first-order kinetics recovery
            double RP = (kP * tau) / (1.0 + kP * tau);    // Palusznium recovery
            double RG = (kG * tau) / (1.0 + kG * tau);    // Gormanium recovery
            double RW = (kW * tau) / (1.0 + kW * tau);    // Waste recovery
            
            // flow rate allocation to conc / tail
            double FPc = FP_in * RP;
            double FPt = FP_in - FPc;
            
            double FGc = FG_in * RG;
            double FGt = FG_in - FGc;
            
            double FWc = FW_in * RW;
            double FWt = FW_in - FWc;
            
            int dstC = units[u].conc_num;   // destination of concentrate stream
            int dstT = units[u].tails_num;  // destination of tailings stream
            
            // Update flow rates for next iteration
            FPn[dstC] += FPc; 
            FGn[dstC] += FGc;
            FWn[dstC] += FWc;
            
            FPn[dstT] += FPt;
            FGn[dstT] += FGt;
            FWn[dstT] += FWt;
        }
        
        // convergence criterion: check all three components
        double err = 0.0;
        for (int idx = 0; idx < n; ++idx) {
            err = std::max(err, std::fabs(FPn[idx] - FP[idx]));
            err = std::max(err, std::fabs(FGn[idx] - FG[idx]));
            err = std::max(err, std::fabs(FWn[idx] - FW[idx]));
        }
        if (err < tol) {
          std::cout << "iteration: " << it << std::endl;
          std::cout << "err: " << err << std::endl;
          return true; 
        }     // converged
        
        // Prepare for next iteration
        FP.swap(FPn);
        FG.swap(FGn);
        FW.swap(FWn);
    }
    return false;                        // not converged after maxIter
}

// calculate the mask of the terminal streams that the unit can reach
uint8_t Circuit::term_mask(int start) const
{
    std::vector<char> seen(n, 0);
    std::queue<int> q;  q.push(start);
    uint8_t m = 0;
    // BFS
    while (!q.empty()) {
        int u = q.front(); q.pop();
        if (u < n) {                          // u is a unit
            if (seen[u]) continue;
            seen[u] = 1;
            q.push(units[u].conc_num);
            q.push(units[u].tails_num);
        } else {                              // u is a terminal
            if (u == OUT_P1()) m |= 0b001;
            else if (u == OUT_P2()) m |= 0b010;
            else                 m |= 0b100;
        }
    }
    return m;
}
