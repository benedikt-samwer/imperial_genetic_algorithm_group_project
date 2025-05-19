#include "CUnit.h"
#include "CCircuit.h"
#include "CSimulator.h"

#include <cmath>

struct Simulator_Parameters default_simulator_parameters = {0.01, 1000};

int fake_answer_vector[] = {0, 1, 2, 2, 3, 4, 2, 1, 4, 4, 5, 4, 4, 1, 2, 6};

double circuit_performance(int vector_size, int* circuit_vector,
                        int unit_parameters_size, double *unit_parameters,
                        struct Simulator_Parameters simulator_parameters) {
//This function takes a circuit vector and returns a performance value.
//The current version of the function is a dummy function that returns
// a performance value based on how close the circuit vector is to a predetermined answer vector.

  double performance = 0.0; 
  for (int i=0;i<vector_size;i++) {
    //dummy_answer_vector is a predetermined answer vector (same size as circuit_vector)
    performance += (20-std::abs(circuit_vector[i]-fake_answer_vector[i]))*100.0; 
  }
  return performance; 
}

// overloads (delete if not needed)
double circuit_performance(int vector_size, int* circuit_vector,
                        int unit_parameters_size, double *unit_parameters){
  return circuit_performance(vector_size, circuit_vector,
                          unit_parameters_size, unit_parameters,
                          default_simulator_parameters);
};
double circuit_performance(int vector_size, int* circuit_vector){
    int num_parameters = (vector_size-1)/2;
    double *parameters;
    parameters = new double[num_parameters];
    for (int i=0; i<num_parameters; i++) {
        parameters[i] = 1.0;
    }
    return circuit_performance(vector_size, circuit_vector, 
                            num_parameters, parameters,
                            default_simulator_parameters);
};

// Other functions and variables to evaluate a real circuit.