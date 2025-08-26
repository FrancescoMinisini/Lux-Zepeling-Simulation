//
/// \file Test/include/SimConfig.hh
/// \brief Centralized simulation parameters for geometry, optics, generator.

#ifndef Test_SimConfig_h
#define Test_SimConfig_h 1

#include "G4SystemOfUnits.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"

namespace Test {

struct SimConfig {
  // --- Geometry (mini TPC)
  struct Geometry {
    G4double lxe_radius   = 30*cm;
    G4double lxe_height   = 50*cm;
    G4double gxe_height   = 3*cm;
    G4double wall_thick   = 1*cm;   // PTFE thickness
    G4double pmt_thick    = 2*mm;   // PMT disk thickness
  } geom;

  // --- Optics/scintillation (LXe)
  struct Optics {
    G4bool   enable_optics          = true;    // turn optical physics on/off
    G4double rindex                 = 1.69;    // ~178 nm index (flat)
    G4double abs_length             = 10.*m;   // placeholder
    G4double rayleigh_length        = 30.*cm;  // placeholder
    G4double scint_yield_perMeV     = 42000./MeV;
    G4double scint_fast_time        = 2.2*ns;
    G4double scint_yield_ratio      = 1.0;     // all fast, placeholder
    // Photon energy grid around 178 nm (~7.0 eV). 2-point flat spectrum is fine.
    G4double eV_min                 = 6.8*eV;
    G4double eV_max                 = 7.2*eV;
  } opt;

  // --- Generator (WIMP-like Xe recoil)
  struct Generator {
    G4bool   use_wimp_proxy         = true;    // Xe recoil if true, else gamma (calib)
    G4int    ion_Z                  = 54;      // Xenon
    G4int    ion_A                  = 131;     // Xe-131
    G4double E_min                  = 1.*keV;  // keV_nr lower bound
    G4double E_max                  = 30.*keV; // keV_nr upper bound
    // If using gamma instead:
    G4double gamma_energy           = 662.*keV;
    G4ThreeVector gamma_position    = G4ThreeVector(0.,0.,-20.*cm);
    G4ThreeVector gamma_direction   = G4ThreeVector(0.,0.,  1.);
  } gen;

  // --- Optical parameters (process-level toggles)
  struct OpticalProcess {
    G4bool scint_by_particle_type   = true;
    G4bool scint_track_sec_first    = true;
    G4bool cerenkov_enable          = false;
    G4int  cerenkov_max_photons     = 0;
    G4bool cerenkov_track_sec_first = false;
  } optProc;

  static inline SimConfig& Get() {
    static SimConfig cfg;
    return cfg;
  }

};

} // namespace Test

#endif
