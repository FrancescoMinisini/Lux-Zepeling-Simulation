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

// SD
#include "PMTSensitiveDetector.hh"

namespace Test
{

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  auto& cfg = SimConfig::Get();
  auto& geom = cfg.geom;
  auto& o    = cfg.opt;

  G4NistManager* nist = G4NistManager::Instance();
  G4bool checkOverlaps = true;

  // --- World ---
  G4double world_sizeXY = 1.2 * (2*(geom.lxe_radius + geom.wall_thick) + 20*cm);
  G4double world_sizeZ  = 1.2 * (geom.lxe_height + geom.gxe_height + 2*geom.pmt_thick + 20*cm);
  G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");

  auto solidWorld = new G4Box("World", 0.5*world_sizeXY, 0.5*world_sizeXY, 0.5*world_sizeZ);
  auto logicWorld = new G4LogicalVolume(solidWorld, world_mat, "World");
  auto physWorld  = new G4PVPlacement(nullptr, {}, logicWorld, "World", nullptr, false, 0, checkOverlaps);

  // --- Materials ---
  auto* lxe_mat = new G4Material("LXe", 54., 131.29*g/mole, 2.90*g/cm3);
  auto* gxe_mat = nist->FindOrBuildMaterial("G4_Xe");
  auto* ptfe_mat = nist->FindOrBuildMaterial("G4_TEFLON");
  auto* pmt_mat  = nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE");

  // --- Optical properties ---
  if (cfg.opt.enable_optics) {
    // LXe with scintillation
    {
      auto* mpt = new G4MaterialPropertiesTable();

      const G4int N = 2;
      G4double E[N]        = {o.eV_min, o.eV_max};
      G4double RIndex[N]   = {o.rindex, o.rindex};
      G4double AbsLen[N]   = {o.abs_length, o.abs_length};
      G4double Rayleigh[N] = {o.rayleigh_length, o.rayleigh_length};
      G4double FastComp[N] = {1., 1.};

      mpt->AddProperty("RINDEX",        E, RIndex,   N, true);
      mpt->AddProperty("ABSLENGTH",     E, AbsLen,   N, true);
      mpt->AddProperty("RAYLEIGH",      E, Rayleigh, N, true);
      mpt->AddProperty("FASTCOMPONENT", E, FastComp, N, true);

      mpt->AddConstProperty("SCINTILLATIONYIELD", o.scint_yield_perMeV, true);
      mpt->AddConstProperty("RESOLUTIONSCALE",    1.0,                  true);
      mpt->AddConstProperty("FASTTIMECONSTANT",   o.scint_fast_time,    true);
      mpt->AddConstProperty("YIELDRATIO",         o.scint_yield_ratio,  true);

      lxe_mat->SetMaterialPropertiesTable(mpt);
    }

    // Give RINDEX to World/GXe/PMT so optical photons can propagate
    {
      const G4int N = 2;
      G4double E[N] = {o.eV_min, o.eV_max};

      // World (air)
      {
        auto* mpt = new G4MaterialPropertiesTable();
        G4double nW[N] = {1.0003, 1.0003};
        mpt->AddProperty("RINDEX", E, nW, N, true);
        world_mat->SetMaterialPropertiesTable(mpt);
      }
      // GXe
      {
        auto* mpt = new G4MaterialPropertiesTable();
        G4double nG[N]  = {1.46, 1.46}; // Match SiO2 to reduce TIR
        G4double abs[N] = {o.abs_length, o.abs_length};
        mpt->AddProperty("RINDEX",    E, nG,  N, true);
        mpt->AddProperty("ABSLENGTH", E, abs, N, true);
        gxe_mat->SetMaterialPropertiesTable(mpt);
      }
      // PMT window (SiO2)
      {
        auto* mpt = new G4MaterialPropertiesTable();
        G4double nQ[N] = {1.46, 1.46};
        mpt->AddProperty("RINDEX", E, nQ, N, true);
        pmt_mat->SetMaterialPropertiesTable(mpt);
      }
    }
  }

  // --- Geometry ---
  auto solidLXe = new G4Tubs("LXeSolid", 0.*cm, geom.lxe_radius, 0.5*geom.lxe_height, 0.*deg, 360.*deg);
  auto logicLXe = new G4LogicalVolume(solidLXe, lxe_mat, "LXe");
  auto physLXe  = new G4PVPlacement(nullptr, {0,0,0}, logicLXe, "LXe", logicWorld, false, 0, checkOverlaps);

  auto solidGXe = new G4Tubs("GXeSolid", 0.*cm, geom.lxe_radius, 0.5*geom.gxe_height, 0.*deg, 360.*deg);
  auto logicGXe = new G4LogicalVolume(solidGXe, gxe_mat, "GXe");
  auto physGXe  = new G4PVPlacement(nullptr, {0,0, 0.5*geom.lxe_height + 0.5*geom.gxe_height}, logicGXe, "GXe", logicWorld, false, 0, checkOverlaps);

  auto solidWall = new G4Tubs("PTFEWallSolid", geom.lxe_radius, geom.lxe_radius + geom.wall_thick,
                              0.5*(geom.lxe_height + geom.gxe_height), 0.*deg, 360.*deg);
  auto logicWall = new G4LogicalVolume(solidWall, ptfe_mat, "PTFEWall");
  auto physWall  = new G4PVPlacement(nullptr, {0,0, 0.5*geom.gxe_height}, logicWall, "PTFEWall", logicWorld, false, 0, checkOverlaps);

  auto solidTopPMT = new G4Tubs("TopPMTSolid", 0.*cm, geom.lxe_radius, 0.5*geom.pmt_thick, 0.*deg, 360.*deg);
  auto logicTopPMT = new G4LogicalVolume(solidTopPMT, pmt_mat, "TopPMT");
  auto physTopPMT  = new G4PVPlacement(nullptr, {0,0, 0.5*geom.lxe_height + geom.gxe_height + 0.5*geom.pmt_thick},
                                       logicTopPMT, "TopPMT", logicWorld, false, 0, checkOverlaps);

  auto solidBotPMT = new G4Tubs("BottomPMTSolid", 0.*cm, geom.lxe_radius, 0.5*geom.pmt_thick, 0.*deg, 360.*deg);
  auto logicBotPMT = new G4LogicalVolume(solidBotPMT, pmt_mat, "BottomPMT");
  auto physBotPMT  = new G4PVPlacement(nullptr, {0,0, -(0.5*geom.lxe_height + 0.5*geom.pmt_thick)},
                                       logicBotPMT, "BottomPMT", logicWorld, false, 0, checkOverlaps);

  // --- Reflective PTFE wall surface ---
  if (cfg.opt.enable_optics) {
    auto* opWall = new G4OpticalSurface("WallSurf");
    opWall->SetType(dielectric_metal);
    opWall->SetModel(unified);
    opWall->SetFinish(ground); // Diffuse reflection

    auto* mptWall = new G4MaterialPropertiesTable();
    const G4int N = 2;
    G4double E[N] = {o.eV_min, o.eV_max};
    G4double refl[N] = {0.95, 0.95};
    mptWall->AddProperty("REFLECTIVITY", E, refl, N);
    opWall->SetMaterialPropertiesTable(mptWall);

    new G4LogicalBorderSurface("LXeWallBorder", physLXe, physWall, opWall);
    new G4LogicalBorderSurface("GXeWallBorder", physGXe, physWall, opWall);

    // --- PMT optical surfaces (dielectric_dielectric) ---
    auto* opTopPMT = new G4OpticalSurface("TopPMTSurf");
    opTopPMT->SetType(dielectric_dielectric);
    opTopPMT->SetModel(unified);
    opTopPMT->SetFinish(polished);

    auto* mptTopPMT = new G4MaterialPropertiesTable();
    G4double eff[N] = {o.pmt_qe, o.pmt_qe};
    G4double trans[N] = {1.0, 1.0}; // Full transmission
    mptTopPMT->AddProperty("EFFICIENCY", E, eff, N);
    mptTopPMT->AddProperty("TRANSMITTANCE", E, trans, N);
    opTopPMT->SetMaterialPropertiesTable(mptTopPMT);

    auto* opBotPMT = new G4OpticalSurface("BotPMTSurf");
    opBotPMT->SetType(dielectric_dielectric);
    opBotPMT->SetModel(unified);
    opBotPMT->SetFinish(polished);

    auto* mptBotPMT = new G4MaterialPropertiesTable();
    mptBotPMT->AddProperty("EFFICIENCY", E, eff, N);
    mptBotPMT->AddProperty("TRANSMITTANCE", E, trans, N);
    opBotPMT->SetMaterialPropertiesTable(mptBotPMT);

    new G4LogicalBorderSurface("GXeTopPMTBorder", physGXe, physTopPMT, opTopPMT);
    new G4LogicalBorderSurface("LXeBotPMTBorder", physLXe, physBotPMT, opBotPMT);
  }

  // --- Visuals ---
  logicWorld->SetVisAttributes(new G4VisAttributes(G4Colour(0,0,0,0)));
  auto visLXe = new G4VisAttributes(G4Colour(0.2,0.4,1.0,0.3)); visLXe->SetForceSolid(true); logicLXe->SetVisAttributes(visLXe);
  auto visGXe = new G4VisAttributes(G4Colour(0.7,0.9,1.0,0.2)); visGXe->SetForceSolid(true); logicGXe->SetVisAttributes(visGXe);
  auto visWall= new G4VisAttributes(G4Colour(0.9,0.9,0.9,0.5)); visWall->SetForceSolid(true); logicWall->SetVisAttributes(visWall);
  auto visTop = new G4VisAttributes(G4Colour(1.0,1.0,0.2,0.7)); visTop->SetForceSolid(true); logicTopPMT->SetVisAttributes(visTop);
  auto visBot = new G4VisAttributes(G4Colour(1.0,1.0,0.2,0.7)); visBot->SetForceSolid(true); logicBotPMT->SetVisAttributes(visBot);

  fScoringVolume = logicLXe;
  return physWorld;
}

void DetectorConstruction::ConstructSDandField()
{
  auto* sdman = G4SDManager::GetSDMpointer();
  auto* pmtSD = new Test::PMTSensitiveDetector("PMTSD");
  sdman->AddNewDetector(pmtSD);

  auto* topLV = G4LogicalVolumeStore::GetInstance()->GetVolume("TopPMT");
  auto* botLV = G4LogicalVolumeStore::GetInstance()->GetVolume("BottomPMT");
  if (topLV) topLV->SetSensitiveDetector(pmtSD);
  if (botLV) botLV->SetSensitiveDetector(pmtSD);
}

} // namespace Test