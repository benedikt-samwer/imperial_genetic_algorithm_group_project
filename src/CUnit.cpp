#include "CUnit.h"

CUnit::CUnit()
    : conc_num(0), tails_num(0), mark(false), volume(10.0),
      feed_palusznium(0.0), feed_gormanium(0.0), feed_waste(0.0),
      k_palusznium(0.008), k_gormanium(0.004), k_waste(0.0005),
      conc_palusznium(0.0), conc_gormanium(0.0), conc_waste(0.0),
      tails_palusznium(0.0), tails_gormanium(0.0), tails_waste(0.0), V_min(2.5), V_max(20.0) {}

CUnit::CUnit(int conc, int tails)
    : conc_num(conc), tails_num(tails), mark(false),
      volume(Constants::Circuit::DEFAULT_UNIT_VOLUME),
      feed_palusznium(0.0), feed_gormanium(0.0), feed_waste(0.0),
      k_palusznium(Constants::Physical::K_PALUSZNIUM),
      k_gormanium(Constants::Physical::K_GORMANIUM),
      k_waste(Constants::Physical::K_WASTE),
      conc_palusznium(0.0), conc_gormanium(0.0), conc_waste(0.0),
      tails_palusznium(0.0), tails_gormanium(0.0), tails_waste(0.0), V_min(Constants::Circuit::MIN_UNIT_VOLUME), V_max(Constants::Circuit::MAX_UNIT_VOLUME) {}



void CUnit::process()
{
    /* ----------- 1. Residence time τ ----------- */
    constexpr double rho   = Constants::Physical::MATERIAL_DENSITY;   // 3000 kg/m³
    constexpr double phi   = Constants::Physical::SOLIDS_CONTENT;       // 0.10

    // total solids feed (kg/s)
    const double Ftot = feed_palusznium + feed_gormanium + feed_waste;
    // guard against division‑by‑zero / vanishing flow
    const double minFlow = 1e-10;                          
    const double tau = phi * volume / (std::max(Ftot, minFlow)/rho);

    /* ----------- 2. Recoveries R_i^C ----------- */
    Rp = k_palusznium * tau / (1.0 + k_palusznium * tau);
    Rg = k_gormanium * tau / (1.0 + k_gormanium * tau);
    Rw = k_waste      * tau / (1.0 + k_waste      * tau);

    /* ----------- 3. Split feed into products --- */
    // Palusznium
    conc_palusznium  = feed_palusznium * Rp;
    tails_palusznium = feed_palusznium - conc_palusznium;

    // Gormanium
    conc_gormanium  = feed_gormanium * Rg;
    tails_gormanium = feed_gormanium - conc_gormanium;

    // Waste
    conc_waste  = feed_waste * Rw;
    tails_waste = feed_waste - conc_waste;
}

void CUnit::update_volume(double beta) {
    this->volume = this->V_min + (this->V_max - this->V_min) * beta; 
}