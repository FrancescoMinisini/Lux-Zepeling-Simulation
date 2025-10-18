#ifndef Test_SimConfig_h
#define Test_SimConfig_h 1
#include "G4SystemOfUnits.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"
namespace Test {
struct SimConfig {
  // --- Geometry (mini TPC)
  struct Geometry {
    G4double lxe_radius = 30*cm;
    G4double lxe_height = 50*cm;
    G4double gxe_height = 0.1*cm;
    G4double wall_thick = 1*cm;
    G4double pmt_thick = 2*mm;
  } geom;
  // --- Optics/scintillation (LXe)
  struct Optics {
    G4bool enable_optics = true;
    G4double rindex = 1.69;
    G4double abs_length = 1000.*m;
    G4double rayleigh_length = 500.*cm; // Increased to reduce scattering
    G4double scint_yield_perMeV = 200000./MeV;
    G4double scint_fast_time = 2.2*ns;
    G4double scint_yield_ratio = 1.0;
    G4double eV_min = 6.8*eV;
    G4double eV_max = 7.2*eV;
    G4double pmt_qe = 1.0;
  } opt;
  // --- Generator (WIMP-like Xe recoil + categorie)
  struct Generator {
    G4bool use_wimp_proxy = false; // Gamma for testing
    // Ion recoil
    G4int ion_Z = 54;
    G4int ion_A = 131;
    G4double E_min = 1.*keV;
    G4double E_max = 30.*keV;
    // Gamma calib
    G4double gamma_energy = 662.*keV;
    G4ThreeVector gamma_position = G4ThreeVector(0.,0.,0.); // Centered
    G4ThreeVector gamma_direction = G4ThreeVector(0.,0.,1.);
    // Categoria: 0..3 (Single, DoubleNear, DoubleFar, Triple)
    G4int event_category = 0;
    // Controlli spaziotemporali
    G4double near_dt_ns = 2.0; // ns
    G4double near_dr = 2.0*mm; // mm
    G4double far_dt_ns = 200.0; // ns
    G4double far_min_dr = 20.0*mm; // mm
    // triple timings
    G4double triple_t_ns_1 = 0.0;
    G4double triple_t_ns_2 = 50.0;
    G4double triple_t_ns_3 = 120.0;
  } gen;
  // --- Optical process toggles
  struct OpticalProcess {
    G4bool scint_by_particle_type = false;
    G4bool scint_track_sec_first = true;
    G4bool cerenkov_enable = false;
    G4int cerenkov_max_photons = 0;
    G4bool cerenkov_track_sec_first = false;
  } optProc;
  static inline SimConfig& Get() {
    static SimConfig cfg;
    return cfg;
  }
};
} // namespace Test
#endif