#include <iostream>
#include <iomanip>
#include "CCircuit.h"
#include "CSimulator.h"
#include "CUnit.h"
#include <vector>


void testUnit(){
     // 创建一个单元
    CUnit unit;
    // 设置进料
    unit.feed_palusznium = 0.0;
    unit.feed_gormanium = 10.0;
    unit.feed_waste = 90.0;

    // 运行 process
    unit.process();

    // 输出格式化结果
    std::cout << std::fixed << std::setprecision(10);

    // std::cout << std::fixed << std::setprecision(4);
    std::cout << "Concentrate stream:" << std::endl;
    std::cout << "  Palusznium: " << unit.conc_palusznium << " kg/s" << std::endl;
    std::cout << "  Gormanium:  " << unit.conc_gormanium << " kg/s" << std::endl;
    std::cout << "  Waste:      " << unit.conc_waste << " kg/s" << std::endl;

    std::cout << "Tails stream:" << std::endl;
    std::cout << "  Palusznium: " << unit.tails_palusznium << " kg/s" << std::endl;
    std::cout << "  Gormanium:  " << unit.tails_gormanium << " kg/s" << std::endl;
    std::cout << "  Waste:      " << unit.tails_waste << " kg/s" << std::endl;

    std::cout << "Recoveries:" << std::endl;    
    std::cout << "  Palusznium: " << unit.Rp << std::endl;
    std::cout << "  Gormanium:  " << unit.Rg << std::endl;
    std::cout << "  Waste:      " << unit.Rw << std::endl;

    // 验证正确性（出口总和应等于进料）
    double sum_p = unit.conc_palusznium + unit.tails_palusznium;
    double sum_g = unit.conc_gormanium + unit.tails_gormanium;
    double sum_w = unit.conc_waste + unit.tails_waste;
    std::cout << "\nCheck (should equal feed):" << std::endl;
    std::cout << "  Palusznium: " << sum_p << " (feed: 8.0)" << std::endl;
    std::cout << "  Gormanium:  " << sum_g << " (feed: 12.0)" << std::endl;
    std::cout << "  Waste:      " << sum_w << " (feed: 80.0)" << std::endl;

    std::cout<< unit.conc_palusznium << " " << unit.tails_palusznium << std::endl;
    std::cout<< unit.conc_gormanium << " " << unit.tails_gormanium << std::endl;
    std::cout<< unit.conc_waste << " " << unit.tails_waste << std::endl;
}

int testCCircuit(){
    int circuit_vec[] = {0, 3, 1, 3, 2, 3, 5, 4, 7, 6, 3, 3, 8}  ;
    // int circuit_vec[] = {1,2,3,0,3,4,3,0,6};

    // std::vector<int> circuit_vec1 = {0, 3, 1, 3, 2, 3, 5, 4, 7, 6, 3, 3, 8};


    Circuit circuit(13);
    if (!circuit.initialize_from_vector(13, circuit_vec)) {
        std::cout << "电路初始化失败！" << std::endl;
        return 1;
    }

    if (!circuit.run_mass_balance()) {
        std::cout << "质量平衡未收敛！" << std::endl;
        return 1;
    }

    std::cout << "Palusznium产品流量: " << circuit.palusznium_product_palusznium << " kg/s" << std::endl;
    std::cout << "Gormanium产品流量:  " << circuit.gormanium_product_gormanium << " kg/s" << std::endl;
    std::cout << "尾矿流量:           " << circuit.tailings_waste << " kg/s" << std::endl;

    std::cout << "经济价值: " << circuit.get_economic_value() << std::endl;
    std::cout << "Palusznium回收率: " << circuit.get_palusznium_recovery() << std::endl;
    std::cout << "Gormanium回收率:  " << circuit.get_gormanium_recovery() << std::endl;
    
    circuit.export_to_dot("../circuit.dot");
    std::cout << "已导出 dot 文件: circuit.dot" << std::endl;
        
    double result = circuit_performance(13, circuit_vec);
    
    std::cout << "circuit_performance(13, vec1) = "<< result <<"\n";
    return 0;
}

int main() {
    testCCircuit();
    // testUnit();

    return 0;
}
