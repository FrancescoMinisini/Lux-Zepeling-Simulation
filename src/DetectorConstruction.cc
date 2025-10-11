// src/DetectorConstruction.cc
#include "DetectorConstruction.hh"
#include "SimConfig.hh"
#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4SDManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "PMTSensitiveDetector.hh"

namespace LZSim {
G4VPhysicalVolume* DetectorConstruction::Construct() {
  auto& cfg = SimConfig::Get();
  auto& geom = cfg.geom;
  auto& o = cfg.opt;
  G4NistManager* nist = G4NistManager::Instance();
  G4bool checkOverlaps = true;

  // --- World ---
  G4double world_sizeXY = 1.2 * (2 * geom.gxe_radius + 20 * cm);
  G4double world_sizeZ = 1.2 * (geom.gxe_height + geom.pmt_thick + 20 * cm);
  G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");
  auto solidWorld = new G4Box("World", 0.5 * world_sizeXY, 0.5 * world_sizeXY, 0.5 * world_sizeZ);
  auto logicWorld = new G4LogicalVolume(solidWorld, world_mat, "World");
  auto physWorld = new G4PVPlacement(nullptr, {}, logicWorld, "World", nullptr, false, 0, checkOverlaps);

  // --- Materials ---
  G4double gas_density = 0.0055 * g / cm3;  // Approx for Xe gas at 1 atm; adjust for LZ pressure
  auto* gxe_mat = new G4Material("GXe", 54., 131.29 * g / mole, gas_density, kStateGas, 293. * kelvin, 1. * atmosphere);
  auto* pmt_mat = nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE");

  // --- Optical properties ---
  if (cfg.opt.enable_optics) {
    // GXe with scintillation (high yield for S2 proxy)
    {
      auto* mpt = new G4MaterialPropertiesTable();
      const G4int N = 2;
      G4double E[N] = {o.eV_min, o.eV_max};
      G4double RIndex[N] = {o.rindex, o.rindex};
      G4double AbsLen[N] = {o.abs_length, o.abs_length};
      G4double Rayleigh[N] = {o.rayleigh_length, o.rayleigh_length};
      G4double FastComp[N] = {1., 1.};
      mpt->AddProperty("RINDEX", E, RIndex, N, true);
      mpt->AddProperty("ABSLENGTH", E, AbsLen, N, true);
      mpt->AddProperty("RAYLEIGH", E, Rayleigh, N, true);
      mpt->AddProperty("FASTCOMPONENT", E, FastComp, N, true);
      mpt->AddConstProperty("SCINTILLATIONYIELD", o.scint_yield_perMeV, true);
      mpt->AddConstProperty("RESOLUTIONSCALE", 1.0, true);
      mpt->AddConstProperty("FASTTIMECONSTANT", o.scint_fast_time, true);
      mpt->AddConstProperty("YIELDRATIO", o.scint_yield_ratio, true);
      gxe_mat->SetMaterialPropertiesTable(mpt);
    }
    // RINDEX for World/PMT
    {
      const G4int N = 2;
      G4double E[N] = {o.eV_min, o.eV_max};
      // World
      {
        auto* mpt = new G4MaterialPropertiesTable();
        G4double nW[N] = {1.0003, 1.0003};
        mpt->AddProperty("RINDEX", E, nW, N, true);
        world_mat->SetMaterialPropertiesTable(mpt);
      }
      // PMT (SiO2)
      {
        auto* mpt = new G4MaterialPropertiesTable();
        G4double nQ[N] = {1.46, 1.46};
        mpt->AddProperty("RINDEX", E, nQ, N, true);
        pmt_mat->SetMaterialPropertiesTable(mpt);
      }
    }
  }

  // --- Geometry: only GXe + Top PMT ---
  auto solidGXe = new G4Tubs("GXeSolid", 0. * cm, geom.gxe_radius, 0.5 * geom.gxe_height, 0. * deg, 360. * deg);
  auto logicGXe = new G4LogicalVolume(solidGXe, gxe_mat, "GXe");
  auto physGXe = new G4PVPlacement(nullptr, {0, 0, 0}, logicGXe, "GXe", logicWorld, false, 0, checkOverlaps);

  auto solidTopPMT = new G4Tubs("TopPMTSolid", 0. * cm, geom.gxe_radius, 0.5 * geom.pmt_thick, 0. * deg, 360. * deg);
  auto logicTopPMT = new G4LogicalVolume(solidTopPMT, pmt_mat, "TopPMT");
  auto physTopPMT = new G4PVPlacement(nullptr, {0, 0, 0.5 * geom.gxe_height + 0.5 * geom.pmt_thick},
                                      logicTopPMT, "TopPMT", logicWorld, false, 0, checkOverlaps);

  // --- PMT optical surface ---
  if (cfg.opt.enable_optics) {
    auto* opTopPMT = new G4OpticalSurface("TopPMTSurf");
    opTopPMT->SetType(dielectric_dielectric);
    opTopPMT->SetModel(unified);
    opTopPMT->SetFinish(polished);
    auto* mptTopPMT = new G4MaterialPropertiesTable();
    const G4int N = 2;
    G4double E[N] = {o.eV_min, o.eV_max};
    G4double eff[N] = {1.0, 1.0};  // EFFICIENCY=1.0
    G4double trans[N] = {1.0, 1.0};  // TRANSMITTANCE=1.0
    mptTopPMT->AddProperty("EFFICIENCY", E, eff, N);
    mptTopPMT->AddProperty("TRANSMITTANCE", E, trans, N);
    opTopPMT->SetMaterialPropertiesTable(mptTopPMT);
    new G4LogicalBorderSurface("GXeTopPMTBorder", physGXe, physTopPMT, opTopPMT);
  }

  // --- Visuals ---
  logicWorld->SetVisAttributes(new G4VisAttributes(G4Colour(0, 0, 0, 0)));
  auto visGXe = new G4VisAttributes(G4Colour(0.7, 0.9, 1.0, 0.2)); visGXe->SetForceSolid(true); logicGXe->SetVisAttributes(visGXe);
  auto visTop = new G4VisAttributes(G4Colour(1.0, 1.0, 0.2, 0.7)); visTop->SetForceSolid(true); logicTopPMT->SetVisAttributes(visTop);

  fScoringVolume = logicGXe;
  return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
  auto* sdman = G4SDManager::GetSDMpointer();
  auto* pmtSD = new PMTSensitiveDetector("PMTSD");
  sdman->AddNewDetector(pmtSD);
  auto* topLV = G4LogicalVolumeStore::GetInstance()->GetVolume("TopPMT");
  if (topLV) topLV->SetSensitiveDetector(pmtSD);
}
} // namespace LZSim