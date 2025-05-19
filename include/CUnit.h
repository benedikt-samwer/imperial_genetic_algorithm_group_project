/** Header for the unit class
 * 
 * This class defines a separation unit in the mineral processing circuit
 * Each unit has three output streams: high-grade concentrate, intermediate, and tailings
 * 
 */

#pragma once

#include <vector>

class CUnit {
 public:
  // Index of the unit to which this unit's high-grade concentrate stream is connected 
  int conc_num;
  // Index of the unit to which this unit's intermediate stream is connected
  int inter_num;
  // Index of the unit to which this unit's tailings stream is connected 
  int tails_num;
  // A Boolean that is changed to true if the unit has been seen during graph traversal
  bool mark;

  // Physical properties
  double volume;                 // Volume of the unit in m³
  
  // Material flow rates (kg/s)
  double feed_palusznium;       // Palusznium in feed
  double feed_gormanium;        // Gormanium in feed  
  double feed_waste;            // Waste material in feed
  
  // Rate constants for separation (s⁻¹)
  double k_palusznium_high;     // Rate constant for Palusznium to high-grade
  double k_palusznium_inter;    // Rate constant for Palusznium to intermediate
  double k_gormanium_high;      // Rate constant for Gormanium to high-grade
  double k_gormanium_inter;     // Rate constant for Gormanium to intermediate
  double k_waste_high;          // Rate constant for Waste to high-grade
  double k_waste_inter;         // Rate constant for Waste to intermediate

  // Constructor with default values
  CUnit() : conc_num(0), inter_num(0), tails_num(0), mark(false), volume(10.0),
            feed_palusznium(0.0), feed_gormanium(0.0), feed_waste(0.0),
            k_palusznium_high(0.008), k_palusznium_inter(0.004), 
            k_gormanium_high(0.004), k_gormanium_inter(0.002),
            k_waste_high(0.0005), k_waste_inter(0.00025) {}

  /*

    ...other member functions and variables of CUnit

  */
};

