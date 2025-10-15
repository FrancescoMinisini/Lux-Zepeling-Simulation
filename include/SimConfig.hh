#ifndef LZSIM_SIMCONFIG_HH
#define LZSIM_SIMCONFIG_HH
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Types.hh"
#include "G4UnitsTable.hh"

namespace LZSim {
struct SimConfig {
  struct Geometry {
    G4double gxe_radius = 50. * cm;
    G4double gxe_height = 10. * cm;  // Aumentato per drift lungo (~10 μs)
    G4double wall_thick = 1.0 * cm;
    G4double pmt_thick = 0.5 * cm;
  } geom;

  struct Optical {
    G4bool enable_optics = true;
    G4double eV_min = 6. * eV;
    G4double eV_max = 8. * eV;
    G4double rindex = 1.46;  // Fix: Match con PMT per ridurre TIR
    G4double abs_length = 1e30 * cm;
    G4double rayleigh_length = 1e30 * cm;
    G4double scint_yield_perMeV = 10000.;  // Fix: Alto per proxy EL visibile
    G4double scint_fast_time = 2. * ns;
    G4double scint_yield_ratio = 1.0;
    G4double pmt_qe = 1.;
  } opt;

  struct Generator {
    G4double electron_energy = 5. * keV;
    G4ThreeVector electron_position = {0, 0, -0.5 * cm};  // Fix: Bottom di GXe (per height=1 cm)
    G4ThreeVector electron_direction = {0, 0, 1};
    G4int n_electrons = 50;  // Numero e- per evento (da lit: tipico per keV in Xe)
  } gen;

  struct Field {
    G4double gas_field = -1. * kilovolt / cm;  // Fix: Valore tipico per EL, negativo per drift up
  } field;

  struct ELParams {
    G4double el_gain_per_mm = 500.;  // Fotoni per mm di drift per e- (da lit: ~100-500)
    G4double v_drift_mm_per_ns = 0.001;  // 1 mm/μs = 0.001 mm/ns (da lit: ~1 mm/μs a 1 kV/cm)
    G4double diffusion_const_cm2_per_s = 0.1;  // D_L (da lit: ~0.1 cm²/s)
  } el;

  static SimConfig& Get() {
    static SimConfig instance;
    return instance;
  }
};
} // namespace LZSim

#endif