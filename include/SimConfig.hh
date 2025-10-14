// ===== FILE: include/SimConfig.hh =====
#ifndef LZSIM_SIMCONFIG_HH
#define LZSIM_SIMCONFIG_HH
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Types.hh"
#include "G4UnitsTable.hh"


namespace LZSim {
struct SimConfig {
  struct Geometry {
    G4double gxe_radius = 50. * cm;  // Scaled down from LZ ~73 cm
    G4double gxe_height = 1.0 * cm;  // Typical gas gap
    G4double pmt_thick = 0.5 * cm;
  } geom;

  struct Optical {
    G4bool enable_optics = true;
    G4double eV_min = 6. * eV;
    G4double eV_max = 8. * eV;  // ~175 nm for Xe
    G4double rindex = 1.001;  // For gas Xe
    G4double abs_length = 100. * m;
    G4double rayleigh_length = 10. * cm;
    G4double scint_yield_perMeV = 1e6;  // Increased for more photons as EL proxy
    G4double scint_fast_time = 2. * ns;
    G4double scint_yield_ratio = 1.0;
    G4double pmt_qe = 0.3;
  } opt;

  struct Generator {
    G4double electron_energy = 5. * keV;  // keV range
    G4ThreeVector electron_position = {0, 0, -0.5 * 1.0 * cm};  // Bottom of gas
    G4ThreeVector electron_direction = {0, 0, 1};  // Up
  } gen;

  struct Field {
    G4double gas_field = -5. * kilovolt / cm;  // Electric field in gas for drift
  } field;

  static SimConfig& Get() {
    static SimConfig instance;
    return instance;
  }
};
} // namespace LZSim

#endif