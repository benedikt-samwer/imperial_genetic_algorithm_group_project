/**
 * @file CUnit.h
 * @brief Declares the CUnit class – a single separation unit in the
 *        mineral‑processing circuit (e.g. flotation cell / centrifuge).
 *
 * A unit receives one mixed feed stream and produces two output streams:
 *   - Concentrate ("high‑grade")  -> directed to conc_num
 *   - Tails      ("low‑grade")    -> directed to tails_num
 *
 * The class stores:
 *   – Topology information (where each outlet goes)
 *   – Kinetic/geometry constants
 *   – Current iteration's mass‑flow state (feed & product streams)
 *   – A traversal flag used by validity checks / graph search
 *
 * It also provides:
 *   • Constructors to initialise a unit
 *   • process() – computes residence time, recoveries, and updates the
 *                 outlet flowrates given the current feed.
 */

#pragma once

#include <vector>
#include <iostream>
#include "constants.h"   // contains default k‑values, density, φ, etc.

/* ------------------------------------------------------------------ */
/*                         Separation‑unit class                       */
/* ------------------------------------------------------------------ */
class CUnit {
 public:
  // Index of the unit to which this unit's concentrate stream is connected 
  int conc_num;
  // Index of the unit to which this unit's tailings stream is connected
  int tails_num;
  // A Boolean that is changed to true if the unit has been seen during graph traversal
  bool mark;

  /* ---------------- Physical / kinetic parameters ----------------------- */
  double volume;          ///< Unit volume V  (m³) – default 10 m³
  double V_min;          ///< Minimum volume (m³) – default 2.5 m³
  double V_max;          ///< Maximum volume (m³) – default 20 m³
  
  // Material flow rates (kg/s)
  double feed_palusznium;       // Palusznium in feed
  double feed_gormanium;        // Gormanium in feed  
  double feed_waste;            // Waste material in feed
  
  // Rate constants for separation (s⁻¹)
  double k_palusznium;          // Rate constant for Palusznium
  double k_gormanium;           // Rate constant for Gormanium
  double k_waste;               // Rate constant for Waste
  
  /* -------------------- Computed outlet mass flowrates ------------------ */
  // Concentrate stream
  double conc_palusznium; ///< C_P  (kg s⁻¹)
  double conc_gormanium;  ///< C_G  (kg s⁻¹)
  double conc_waste;      ///< C_W  (kg s⁻¹)
  // Tails stream
  double tails_palusznium;///< T_P  (kg s⁻¹)
  double tails_gormanium; ///< T_G  (kg s⁻¹)
  double tails_waste;     ///< T_W  (kg s⁻¹)

  double rho;
  double phi;
  
  /* -------------------- Computed recoveries for each component ----------- */
  double Rp, Rg, Rw; ///< Recoveries for each component

  /* --------------------------- Constructors ----------------------------- */
  /// Default constructor – initialises all numeric members to zero and
  /// routes to invalid destinations (e.g. ‑1) until set by GA vector.
  CUnit();

  /// Convenience constructor – sets outlet destinations; remaining
  /// parameters are pulled from constants.h defaults.
  /// @param conc  Destination index for concentrate
  /// @param tails Destination index for tails
  CUnit(int conc, int tails);

  /* ----------------------------- Methods -------------------------------- */
  /**
   * @brief Perform unit calculation for the current feed.
   *
   * Steps:
   *   1. Compute residence time τ = φ V / (ρ ΣF_i)
   *   2. Evaluate recoveries   R_i^C = k_i τ / (1 + k_i τ)
   *   3. Split feed into concentrate & tails streams
   *   4. Store outlet flowrates in the public members above
   *
   * No return value – results are written into conc_* and tails_*.
   * Caller is responsible for ensuring feed_* are populated beforehand.
   */

  CUnit(int conc, int tails, bool testFlag);

  void process();

  /**
   * @brief Check if the unit is valid.
   *
   * A unit is valid if:
   *   1. It has a valid destination for both concentrate and tails
   *   2. It has a non-zero volume
   *   3. It has a non-zero k-value for at least one component
   *
   * @return true if valid, false otherwise.
   */
  // double calculate_recovery(const string& component, double feed_rate) const;

  /**
   * @brief Update the volume of the unit.
   *
   * @param beta The new volume of the unit.
   */
  void update_volume(double beta);
};
