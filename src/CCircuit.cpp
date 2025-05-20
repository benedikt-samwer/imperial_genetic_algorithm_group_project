#include <vector>

#include <stdio.h>
#include <CUnit.h>
#include <CCircuit.h>

Circuit::Circuit(int num_units){
  this->units.resize(num_units);
}

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
  
  //If tails_num does not point at a circuit outlet recursively call the function 

  if (this->units[unit_num].tails_num<this->units.size()) {
    mark_units(this->units[unit_num].tails_num); 
  } else {
    // ...Potentially do something to indicate that you have seen an exit
  }
}


