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

// 初始化电路结构
bool Circuit::initialize_from_vector(int vector_size, const int* circuit_vector) {
    // num_units = n
    int num_units = (vector_size - 1) / 2;
    if (vector_size != 2 * num_units + 1) return false;
    units.resize(num_units);

    // feed_unit 是第一个值
    feed_unit = circuit_vector[0];

    // 将 n, n+1, n+2 映射到负值索引
    for (int i = 0; i < num_units; ++i) {
        int conc = circuit_vector[1 + 2 * i];
        int tails = circuit_vector[1 + 2 * i + 1];

        // 若遇到 n, n+1, n+2，则转为 -1, -2, -3
        if (conc == num_units)       conc = PALUSZNIUM_PRODUCT;   
        else if (conc == num_units+1) conc = GORMANIUM_PRODUCT;   
        else if (conc == num_units+2) conc = TAILINGS_OUTPUT;
        
        if (tails == num_units)       tails = PALUSZNIUM_PRODUCT; 
        else if (tails == num_units+1) tails = GORMANIUM_PRODUCT;
        else if (tails == num_units+2) tails = TAILINGS_OUTPUT;

        units[i] = CUnit(conc, tails);
    }
    return true;
}
// bool Circuit::initialize_from_vector(int vector_size, const int* circuit_vector) {
//     int num_units = (vector_size - 1) / 2;
//     if (vector_size != 2 * num_units + 1) return false;
//     units.resize(num_units);

//     feed_unit = circuit_vector[0];
//     for (int i = 0; i < num_units; ++i) {
//         int conc = circuit_vector[1 + 2 * i];
//         int tails = circuit_vector[1 + 2 * i + 1];
//         units[i] = CUnit(conc, tails);
//     }
//     return true;
// }

// 质量平衡主循环
bool Circuit::run_mass_balance(double tolerance, int max_iterations) {
    // 初始化所有单元的进料为0
    for (auto& u : units) {
        u.feed_palusznium = 0.0;
        u.feed_gormanium = 0.0;
        u.feed_waste = 0.0;
    }
    // 初始进料
    units[feed_unit].feed_palusznium = feed_palusznium_rate;
    units[feed_unit].feed_gormanium = feed_gormanium_rate;
    units[feed_unit].feed_waste = feed_waste_rate;

    // 记录上一次进料用于收敛判断
    std::vector<double> last_feed_p(units.size(), 0.0);
    std::vector<double> last_feed_g(units.size(), 0.0);
    std::vector<double> last_feed_w(units.size(), 0.0);
    std::cout << "单元数量: " << units.size() << std::endl;

    for (int iter = 0; iter < max_iterations; ++iter) {
        std::cout << "迭代 " << iter + 1 << "：\n";

        // 记录当前进料
        // 记录当次循环的feed到last中去，并清零当前循环的feed
        if(iter ==0){
            for (size_t i = 0; i < units.size(); ++i) {
                last_feed_p[i] = units[i].feed_palusznium;
                last_feed_g[i] = units[i].feed_gormanium;
                last_feed_w[i] = units[i].feed_waste;
                // 清零，待重新累加
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
        // 初始进料
        // 往第一个里面feed进料
        units[feed_unit].feed_palusznium = feed_palusznium_rate;
        units[feed_unit].feed_gormanium = feed_gormanium_rate;
        units[feed_unit].feed_waste = feed_waste_rate;

        // 处理所有单元
        for (size_t i = 0; i < units.size(); ++i) {
            std::cout << "  单元" << i << " 进料(P,G,W): " << units[i].feed_palusznium << ", "
                    << units[i].feed_gormanium << ", " << units[i].feed_waste << "\n";
            units[i].process();
            std::cout << "  单元" << i << " 浓缩流(P,G,W): " << units[i].conc_palusznium << ", "
                    << units[i].conc_gormanium << ", " << units[i].conc_waste << "\n";
            std::cout << "  单元" << i << " 尾矿流(P,G,W): " << units[i].tails_palusznium << ", "
                    << units[i].tails_gormanium << ", " << units[i].tails_waste << "\n";
        }

        // 先准备一个标记，用于确保每个下游单元只清零一次
        std::vector<bool> feedCleared(units.size(), false);

        // 汇总所有出口流到下游单元或最终产品
        palusznium_product_palusznium = palusznium_product_gormanium = palusznium_product_waste = 0.0;
        gormanium_product_palusznium = gormanium_product_gormanium = gormanium_product_waste = 0.0;
        tailings_palusznium = tailings_gormanium = tailings_waste = 0.0;

        
        std::cout<<"=====分配下游数据====="<<std::endl;
        for (size_t i = 0; i < units.size(); ++i) {
            // 浓缩流
            int concDest = units[i].conc_num;
            // std::cout << "  单元" << i << " 浓缩流向: " << concDest << std::endl;
            if (concDest == PALUSZNIUM_PRODUCT) {
                palusznium_product_palusznium += units[i].conc_palusznium;
                palusznium_product_gormanium += units[i].conc_gormanium;
                palusznium_product_waste += units[i].conc_waste;
            } else if (concDest == GORMANIUM_PRODUCT) {
                // std::cout << "  单元" << i << " 浓缩流向: GORMANIUM_PRODUCT" << std::endl;
                gormanium_product_palusznium += units[i].conc_palusznium;
                gormanium_product_gormanium += units[i].conc_gormanium;
                gormanium_product_waste += units[i].conc_waste;
            } else if (concDest == TAILINGS_OUTPUT) {
                tailings_palusznium += units[i].conc_palusznium;
                tailings_gormanium += units[i].conc_gormanium;
                tailings_waste += units[i].conc_waste;
            } else if (concDest >= 0 && concDest < (int)units.size()) {
                // std::cout << "  单元" << i << " 浓缩流向: " << concDest << std::endl;
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

            // std::cout << "  单元" << i << " 尾矿流向: " << units[i].tails_num << std::endl;
            // std::cout<<"尾矿数据"<<std::endl<<units[i].tails_palusznium << " " << units[i].tails_gormanium << std::endl;
            // std::cout<<units[i].tails_waste << std::endl;
            // 尾矿流
            int tailsDest = units[i].tails_num;
            if (tailsDest == PALUSZNIUM_PRODUCT) {
                palusznium_product_palusznium += units[i].tails_palusznium;
                palusznium_product_gormanium += units[i].tails_gormanium;
                palusznium_product_waste += units[i].tails_waste;
            } else if (tailsDest == GORMANIUM_PRODUCT) {
                // std::cout << "  单元" << i << " 尾矿流向: GORMANIUM_PRODUCT" << std::endl;
                gormanium_product_palusznium += units[i].tails_palusznium;
                gormanium_product_gormanium += units[i].tails_gormanium;
                gormanium_product_waste += units[i].tails_waste;
                // std::cout<<"尾矿数据"<<std::endl<<units[i].tails_palusznium << " " << units[i].tails_gormanium << std::endl;
                // std::cout<<units[i].tails_waste << std::endl;
            } else if (tailsDest == TAILINGS_OUTPUT) {
                tailings_palusznium += units[i].tails_palusznium;
                tailings_gormanium += units[i].tails_gormanium;
                tailings_waste += units[i].tails_waste;
            } else if (tailsDest >= 0 && tailsDest < (int)units.size()) {
                // std::cout << "  单元" << i << " 非尾矿单元尾矿流向: " << tailsDest << std::endl;
                if (!feedCleared[tailsDest]) {
                    feedCleared[tailsDest] = true;
                    units[tailsDest].feed_palusznium = 0.0;
                    units[tailsDest].feed_gormanium = 0.0;
                    units[tailsDest].feed_waste = 0.0;
                } 
                // std::cout<<"尾矿数据"<<std::endl<<units[i].tails_palusznium << " " << units[i].tails_gormanium << std::endl;
                // std::cout<<units[i].tails_waste << std::endl;
                // std::cout<<"准备分配了！"<<std::endl;
                units[tailsDest].feed_palusznium += units[i].tails_palusznium;
                units[tailsDest].feed_gormanium += units[i].tails_gormanium;
                units[tailsDest].feed_waste += units[i].tails_waste;
                // std::cout<<"尾矿数据"<<std::endl<<units[tailsDest].feed_palusznium << " " << units[tailsDest].feed_gormanium << std::endl;
                // std::cout<<units[tailsDest].feed_waste << std::endl;
            }
        }
 

        // 判断收敛
        double max_rel_change = 0.0;
        for (size_t i = 0; i < units.size(); ++i) {
            double rel_p = std::abs(units[i].feed_palusznium - last_feed_p[i]) / std::max(last_feed_p[i], 1e-12);
            double rel_g = std::abs(units[i].feed_gormanium - last_feed_g[i]) / std::max(last_feed_g[i], 1e-12);
            double rel_w = std::abs(units[i].feed_waste - last_feed_w[i]) / std::max(last_feed_w[i], 1e-12);
            max_rel_change = std::max({max_rel_change, rel_p, rel_g, rel_w});
        }


        // ---- 调试输出 ----
        for (size_t i = 0; i < units.size(); ++i) {
            std::cout << "  单元" << i
                    << " 上轮进料(P,G,W): " << last_feed_p[i] << ", "
                    << last_feed_g[i] << ", " << last_feed_w[i]
                    << " | 本轮进料(P,G,W): " << units[i].feed_palusznium << ", "
                    << units[i].feed_gormanium << ", " << units[i].feed_waste << "\n";
        }
        std::cout << "  Palusznium产品流量: " << palusznium_product_palusznium
                << "  Gormanium产品流量: " << gormanium_product_gormanium
                << "  尾矿流量: " << tailings_waste << std::endl;
        std::cout << "  max_rel_change = " << max_rel_change << std::endl;
        // ---- 调试输出 ----
        if (max_rel_change < tolerance) return true;

        
    }
    return false; // 未收敛
}

// 经济性计算
double Circuit::get_economic_value() const {
    double value = 0.0;
    // Palusznium product
    value += palusznium_product_palusznium * palusznium_value;
    value += palusznium_product_gormanium * -20.0;
    value += palusznium_product_waste * waste_penalty_palusznium;
    // Gormanium产品
    value += gormanium_product_palusznium * 0.0;
    value += gormanium_product_gormanium * gormanium_value;
    value += gormanium_product_waste * waste_penalty_gormanium;

    double total_volume = 0.0;
    for (const auto& u : units) total_volume += u.volume;
    double cost = 5.0 * std::pow(total_volume, 2.0/3.0);
    if (total_volume >= 150.0) {
        cost += 1000.0 * std::pow(total_volume - 150.0, 2.0);
    }
    value -= cost; // 作为惩罚扣除
    get_palusznium_recovery();// 调试输出
    get_gormanium_recovery();// 调试输出
    get_palusznium_grade();// 调试输出
    get_gormanium_grade();// 调试输出
    return value;
}

// 回收率
double Circuit::get_palusznium_recovery() const {
    CUnit cal =  CUnit(-1,-1);
    cal.feed_palusznium = feed_palusznium_rate;
    cal.feed_gormanium = feed_gormanium_rate;
    cal.feed_waste = feed_waste_rate;
    cal.process();
    std::cout<<"palusznium recovery: "<<cal.Rp<<std::endl;


}
double Circuit::get_gormanium_recovery() const {
    CUnit cal =  CUnit(-1,-1);
    cal.feed_palusznium = feed_palusznium_rate;
    cal.feed_gormanium = feed_gormanium_rate;
    cal.feed_waste = feed_waste_rate;
    cal.process();
    std::cout<<"gormanium recovery: "<<cal.Rg<<std::endl;
}

// 品位
double Circuit::get_palusznium_grade() const {
    double total = palusznium_product_palusznium + palusznium_product_gormanium + palusznium_product_waste;
    return (total > 0) ? (palusznium_product_palusznium / total) : 0.0;
}
double Circuit::get_gormanium_grade() const {
    double total = gormanium_product_palusznium + gormanium_product_gormanium + gormanium_product_waste;
    return (total > 0) ? (gormanium_product_gormanium / total) : 0.0;
}

// 可视化导出（简单dot格式）
bool Circuit::export_to_dot(const std::string& filename) const {
    std::ofstream ofs(filename);
    if (!ofs) return false;
    ofs << "digraph Circuit {\n";
    for (size_t i = 0; i < units.size(); ++i) {
        ofs << "  unit" << i << " [label=\"Unit " << i << "\"];\n";
        // 浓缩流
        if (units[i].conc_num >= 0)
            ofs << "  unit" << i << " -> unit" << units[i].conc_num << " [label=\"conc\"];\n";
        else if (units[i].conc_num == PALUSZNIUM_PRODUCT)
            ofs << "  unit" << i << " -> palusznium_product [label=\"conc\"];\n";
        else if (units[i].conc_num == GORMANIUM_PRODUCT)
            ofs << "  unit" << i << " -> gormanium_product [label=\"conc\"];\n";
        else if (units[i].conc_num == TAILINGS_OUTPUT)
            ofs << "  unit" << i << " -> tailings [label=\"conc\"];\n";
        // 尾矿流
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