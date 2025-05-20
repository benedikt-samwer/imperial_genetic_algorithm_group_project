#include "CUnit.h"
#include "CCircuit.h"
#include "CSimulator.h"
#include <cmath>
#include <limits>

struct Simulator_Parameters default_simulator_parameters = {1e-6, 100};

double circuit_performance(int vector_size, int* circuit_vector,
                        int unit_parameters_size, double *unit_parameters,
                        struct Simulator_Parameters simulator_parameters) {
    // 1. 解析单元数
    int num_units = (vector_size - 1) / 2;
    if (vector_size != 2 * num_units + 1 || num_units <= 0) {
        // 非法输入
        return -1e12;
    }

    // 2. 构建电路
    Circuit circuit(num_units);
    if (!circuit.initialize_from_vector(vector_size, circuit_vector)) {
        // 非法结构
        return -1e12;
    }

    // 3. 运行质量平衡
    bool converged = circuit.run_mass_balance(simulator_parameters.tolerance, simulator_parameters.max_iterations);
    if (!converged) {
        // 不收敛，视为无效
        return -1e12;
    }

    // 4. 返回经济性（利润）作为 performance
    return circuit.get_economic_value();
}

// overloads
double circuit_performance(int vector_size, int* circuit_vector,
                        int unit_parameters_size, double *unit_parameters){
    return circuit_performance(vector_size, circuit_vector,
                          unit_parameters_size, unit_parameters,
                          default_simulator_parameters);
}
double circuit_performance(int vector_size, int* circuit_vector){
    int num_parameters = (vector_size-1)/2;
    double *parameters = new double[num_parameters];
    for (int i=0; i<num_parameters; i++) {
        parameters[i] = 1.0;
    }
    double result = circuit_performance(vector_size, circuit_vector, 
                            num_parameters, parameters,
                            default_simulator_parameters);
    delete[] parameters;
    return result;
}