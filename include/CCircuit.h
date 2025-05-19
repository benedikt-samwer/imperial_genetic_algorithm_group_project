/** Header for the circuit class
 *
 * This header defines the circuit class and its associated functions
 *
*/

#pragma once

#include "CUnit.h"

#include <vector>

class Circuit {
  public:
    Circuit(int num_units);
    static bool check_validity(int vector_size, int *);
    static bool check_validity(int vector_size, int *,
                               int unit_parameters_size, double *unit_parameters);
  private:
    void mark_units(int unit_num);
    std::vector<CUnit> units;
};

