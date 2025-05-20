/** Header for the unit class
 * 
 * This class defines a separation unit in the mineral processing circuit
 * Each unit has two output streams: concentrate and waste
 * 
 */

#pragma once

#include <vector>

class CUnit {
 public:
  // Index of the unit to which this unit's concentrate stream is connected 
  int conc_num;
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
  double k_palusznium;          // Rate constant for Palusznium
  double k_gormanium;           // Rate constant for Gormanium
  double k_waste;               // Rate constant for Waste

  // Constructor with default values
  CUnit() : conc_num(0), tails_num(0), mark(false), volume(10.0),
            feed_palusznium(0.0), feed_gormanium(0.0), feed_waste(0.0),
            k_palusznium(0.008), k_gormanium(0.004), k_waste(0.0005) {}

  /*

    ...other member functions and variables of CUnit

  */
};

