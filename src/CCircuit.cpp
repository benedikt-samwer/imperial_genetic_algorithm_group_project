#include <vector>

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
    std::vector<int8_t> cache(n, -1);        // memorize bitmask 0bxyz
    for (int i = 0; i < n; ++i) {
        uint8_t m = outlet_mask(i, cache);
        int cnt = (m & 1) + ((m >> 1) & 1) + ((m >> 2) & 1);
        if (cnt < 2) {
          std::cout << "false 07" << std::endl;
          return false;
        }
    }

    // R6 detect cycle of length ≥2 (self-loop is prohibited)
    // 0 = white, 1 = gray, 2 = black （white: not visited，gray: visited but not explored，black: visited and explored）
    std::vector<char> color(n, 0);


    auto has_cycle = [&](auto&& self, int u) -> bool
    {
        // visit the gray node again, it means this node along the descendants back to itself, forming a cycle
        if (color[u] == 1) {
          std::cout << "false 08" << std::endl;
          return true;
        } 
        // the node has been fully explored, no cycle in the subtree
        if (color[u] == 2) {
          return false;
        }
        color[u] = 1; // mark as gray
        // recursively check the subtree, along conc and tail recursively call, and find a cycle through its some descendant
        if (dest[u].conc < n && self(self, dest[u].conc)) {
          std::cout << "false 09" << std::endl;
          return true;
        }
        if (dest[u].tail < n && self(self, dest[u].tail)) {
          std::cout << "false 11" << std::endl;
          return true;
        }
        color[u] = 2;                     // mark black
        return false;
    };

    for (int i = 0; i < n; ++i)
        if (has_cycle(has_cycle, i)) {
          return false;
        }

    return true;    // legal
}

bool Circuit::check_validity(int vector_size, int *circuit_vector,
                              int num_parameters, double *parameters)
                              
{

  // Rewrite this to check the validity of the circuit vector
  // This is a dummy function that checks if the circuit vector is valid
  return check_validity(vector_size, circuit_vector);

  // return true;
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


uint8_t Circuit::outlet_mask(int unit_num, std::vector<int8_t>& cache) const
{
    if (unit_num < n)          // normal unit
    {
        if (cache[unit_num] != -1) return static_cast<uint8_t>(cache[unit_num]);

        uint8_t m = 0;
        m |= outlet_mask(units[unit_num].conc_num , cache);
        m |= outlet_mask(units[unit_num].tails_num, cache);
        cache[unit_num] = static_cast<int8_t>(m);
        return m;
    }
    else                        // terminal stream
    {
        if (unit_num == OUT_P1()) return 0b001;
        if (unit_num == OUT_P2()) return 0b010;
        /* unit_num == OUT_TA() */return 0b100;
    }
}