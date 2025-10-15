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
#include "G4UniformElectricField.hh"
#include "G4FieldManager.hh"
#include "G4ChordFinder.hh"
#include "G4EqMagElectricField.hh"
#include "G4ClassicalRK4.hh"
#include "G4MagIntegratorDriver.hh"
#include "G4TransportationManager.hh"

namespace LZSim
{
  G4VPhysicalVolume *DetectorConstruction::Construct()
  {
    auto &cfg = SimConfig::Get();
    auto &geom = cfg.geom;
    auto &o = cfg.opt;
    G4NistManager *nist = G4NistManager::Instance();
    G4bool checkOverlaps = true;

    // --- World ---
    G4double world_sizeXY = 1.2 * (2 * geom.gxe_radius + 20 * cm);
    G4double world_sizeZ = 1.2 * (geom.gxe_height + geom.pmt_thick + 20 * cm);
    G4Material *world_mat = nist->FindOrBuildMaterial("G4_AIR");
    auto solidWorld = new G4Box("World", 0.5 * world_sizeXY, 0.5 * world_sizeXY, 0.5 * world_sizeZ);
    auto logicWorld = new G4LogicalVolume(solidWorld, world_mat, "World");
    auto physWorld = new G4PVPlacement(nullptr, {}, logicWorld, "World", nullptr, false, 0, checkOverlaps);

    // --- Materials ---
    G4double gas_density = 0.0055 * g / cm3;
    auto *gxe_mat = new G4Material("GXe", 54., 131.29 * g / mole, gas_density, kStateGas, 293. * kelvin, 1. * atmosphere);
    gxe_mat->GetIonisation()->SetMeanExcitationEnergy(22 * eV);
    auto *pmt_mat = nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE");
    auto *ptfe_mat = nist->FindOrBuildMaterial("G4_TEFLON");

    // --- Optical properties ---
    if (cfg.opt.enable_optics)
    {
      // GXe with scintillation
      {
        auto* mpt = new G4MaterialPropertiesTable();
        const G4int N = 2;
        G4double E[N] = {o.eV_min, o.eV_max};
        G4double RIndex[N] = {o.rindex, o.rindex};  // Ora 1.46
        G4double AbsLen[N] = {o.abs_length, o.abs_length};
        G4double Rayleigh[N] = {o.rayleigh_length, o.rayleigh_length};
        G4double ScintComp[N] = {1., 1.};
        G4double ScintYield[N] = {o.scint_yield_perMeV, o.scint_yield_perMeV};
        mpt->AddProperty("RINDEX", E, RIndex, N, true);
        mpt->AddProperty("ABSLENGTH", E, AbsLen, N, true);
        mpt->AddProperty("RAYLEIGH", E, Rayleigh, N, true);
        mpt->AddProperty("SCINTILLATIONCOMPONENT1", E, ScintComp, N, true);
        mpt->AddProperty("ELECTRONSCINTILLATIONYIELD", E, ScintYield, N, true);
        mpt->AddConstProperty("RESOLUTIONSCALE", 1.0, true);
        mpt->AddConstProperty("SCINTILLATIONTIMECONSTANT1", o.scint_fast_time, true);
        mpt->AddConstProperty("SCINTILLATIONYIELDRATIO1", o.scint_yield_ratio, true);
        mpt->AddConstProperty("SCINTILLATIONYIELD", o.scint_yield_perMeV, true);
        gxe_mat->SetMaterialPropertiesTable(mpt);
        mpt->DumpTable();
      }
      // RINDEX for World/PMT
      {
        const G4int N = 2;
        G4double E[N] = {o.eV_min, o.eV_max};
        // World
        {
          auto *mpt = new G4MaterialPropertiesTable();
          G4double nW[N] = {1.0003, 1.0003};
          mpt->AddProperty("RINDEX", E, nW, N, true);
          world_mat->SetMaterialPropertiesTable(mpt);
        }
        // PMT (SiO2)
        {
          auto *mpt = new G4MaterialPropertiesTable();
          G4double nQ[N] = {1.46, 1.46};
          mpt->AddProperty("RINDEX", E, nQ, N, true);
          pmt_mat->SetMaterialPropertiesTable(mpt);
        }
      }
    }

    // --- Geometry ---
    auto solidGXe = new G4Tubs("GXeSolid", 0. * cm, geom.gxe_radius, 0.5 * geom.gxe_height, 0. * deg, 360. * deg);
    auto logicGXe = new G4LogicalVolume(solidGXe, gxe_mat, "GXe");
    auto physGXe = new G4PVPlacement(nullptr, {0, 0, 0}, logicGXe, "GXe", logicWorld, false, 0, checkOverlaps);

    auto solidTopPMT = new G4Tubs("TopPMTSolid", 0. * cm, geom.gxe_radius, 0.5 * geom.pmt_thick, 0. * deg, 360. * deg);
    auto logicTopPMT = new G4LogicalVolume(solidTopPMT, pmt_mat, "TopPMT");
    auto physTopPMT = new G4PVPlacement(nullptr, {0, 0, 0.5 * geom.gxe_height + 0.5 * geom.pmt_thick},
                                        logicTopPMT, "TopPMT", logicWorld, false, 0, checkOverlaps);
    // --- Reflective PTFE wall ---
    auto solidWall = new G4Tubs("PTFEWallSolid", geom.gxe_radius, geom.gxe_radius + geom.wall_thick,
                                0.5 * geom.gxe_height, 0. * deg, 360. * deg);
    auto logicWall = new G4LogicalVolume(solidWall, ptfe_mat, "PTFEWall");
    auto physWall = new G4PVPlacement(nullptr, {0, 0, 0}, logicWall, "PTFEWall", logicWorld, false, 0, checkOverlaps);

    // --- PMT optical surface --- (Fix: Come S1)
    if (cfg.opt.enable_optics)
    {
      auto *opTopPMT = new G4OpticalSurface("TopPMTSurf");
      opTopPMT->SetType(dielectric_dielectric);  // Fix: Permetti ingresso fotoni
      opTopPMT->SetModel(unified);
      opTopPMT->SetFinish(polished);
      auto *mptTopPMT = new G4MaterialPropertiesTable();
      const G4int N = 2;
      G4double E[N] = {o.eV_min, o.eV_max};
      G4double eff[N] = {o.pmt_qe, o.pmt_qe};
      G4double trans[N] = {1.0, 1.0};  // Fix: Full transmittance
      mptTopPMT->AddProperty("EFFICIENCY", E, eff, N);
      mptTopPMT->AddProperty("TRANSMITTANCE", E, trans, N);
      opTopPMT->SetMaterialPropertiesTable(mptTopPMT);
      new G4LogicalBorderSurface("GXeTopPMTBorder", physGXe, physTopPMT, opTopPMT);

      // Wall: Reflective come S1
      auto *opWall = new G4OpticalSurface("WallSurf");
      opWall->SetType(dielectric_metal);
      opWall->SetModel(unified);
      opWall->SetFinish(ground);
      auto *mptWall = new G4MaterialPropertiesTable();
      G4double refl[N] = {0.95, 0.95};  // Fix: Reflective
      mptWall->AddProperty("REFLECTIVITY", E, refl, N);
      opWall->SetMaterialPropertiesTable(mptWall);
      new G4LogicalBorderSurface("GXeWallBorder", physGXe, physWall, opWall);
    }

    // --- Visuals ---
    logicWorld->SetVisAttributes(new G4VisAttributes(G4Colour(0, 0, 0, 0)));
    auto visGXe = new G4VisAttributes(G4Colour(0.7, 0.9, 1.0, 0.2));
    visGXe->SetForceSolid(true);
    logicGXe->SetVisAttributes(visGXe);
    auto visTop = new G4VisAttributes(G4Colour(1.0, 1.0, 0.2, 0.7));
    visTop->SetForceSolid(true);
    logicTopPMT->SetVisAttributes(visTop);

    fScoringVolume = logicGXe;
    return physWorld;
  }

  void DetectorConstruction::ConstructSDandField()
  {
    auto *sdman = G4SDManager::GetSDMpointer();
    auto *pmtSD = new PMTSensitiveDetector("PMTSD");
    sdman->AddNewDetector(pmtSD);
    auto *topLV = G4LogicalVolumeStore::GetInstance()->GetVolume("TopPMT");
    if (topLV)
      topLV->SetSensitiveDetector(pmtSD);

    // Campo (mantenuto)
    auto &cfg = SimConfig::Get();
    auto *logicGXe = G4LogicalVolumeStore::GetInstance()->GetVolume("GXe");
    if (logicGXe && cfg.field.gas_field != 0.)  // Cambiato >0 a !=0
    {
      G4ElectricField *electricField = new G4UniformElectricField(G4ThreeVector(0., 0., cfg.field.gas_field));
      G4EqMagElectricField *equation = new G4EqMagElectricField(electricField);
      G4int nvar = 8;
      G4MagIntegratorStepper *stepper = new G4ClassicalRK4(equation, nvar);
      G4double minStep = 0.01 * mm;
      G4VIntegrationDriver *driver = new G4MagInt_Driver(minStep, stepper, stepper->GetNumberOfVariables());
      G4ChordFinder *chordFinder = new G4ChordFinder(driver);
      G4FieldManager *fieldMgr = new G4FieldManager(electricField);
      fieldMgr->SetDetectorField(electricField);
      fieldMgr->SetChordFinder(chordFinder);
      logicGXe->SetFieldManager(fieldMgr, true);
    }
  }
} // namespace LZSim