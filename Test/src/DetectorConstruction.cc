//

//
/// \file B1/src/DetectorConstruction.cc
/// \brief Implementation of the B1::DetectorConstruction class

#include "DetectorConstruction.hh"

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

// SD
#include "PMTSensitiveDetector.hh"

namespace Test
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();

  // Option to switch on/off checking of volumes overlaps
  //
  G4bool checkOverlaps = true;

  //
  // Dimensions for mini-TPC
  //
  G4double lxe_radius = 30*cm;
  G4double lxe_height = 50*cm;
  G4double gxe_height = 3*cm;
  G4double wall_thick = 1*cm;
  G4double pmt_thick  = 2*mm;

  //
  // World
  //
  G4double world_sizeXY = 1.2 * (2*(lxe_radius + wall_thick) + 20*cm);
  G4double world_sizeZ  = 1.2 * (lxe_height + gxe_height + 2*pmt_thick + 20*cm);
  G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");

  auto solidWorld = new G4Box("World",
    0.5 * world_sizeXY, 0.5 * world_sizeXY, 0.5 * world_sizeZ);

  auto logicWorld = new G4LogicalVolume(solidWorld, world_mat, "World");

  auto physWorld = new G4PVPlacement(nullptr, G4ThreeVector(),
    logicWorld, "World", nullptr, false, 0, checkOverlaps);

  //
  // Materials
  //
  // Liquid xenon (base material; optical properties set below)
  G4Material* lxe_mat = new G4Material("LXe", 54., 131.29*g/mole, 2.90*g/cm3);

  // Gaseous xenon (NIST)
  G4Material* gxe_mat = nist->FindOrBuildMaterial("G4_Xe");

  // PTFE (Teflon) as structural wall (optical surface will be added later if desired)
  G4Material* ptfe_mat = nist->FindOrBuildMaterial("G4_TEFLON");

  // PMT window proxy (quartz/fused silica)
  G4Material* pmt_mat = nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE");

  //
  // Minimal optical/scintillation properties for LXe (S1 production)
  //
  {
    auto* mpt = new G4MaterialPropertiesTable();

    // Photon energy grid around ~178 nm (~7.0 eV)
    const G4int N = 2;
    G4double E[N] = {6.8*eV, 7.2*eV};

    // Index of refraction ~1.69 (flat placeholder)
    G4double RIndex[N]   = {1.69, 1.69};
    // Absorption length placeholder (depends on purity)
    G4double AbsLen[N]   = {10.*m, 10.*m};
    // Rayleigh scattering placeholder (~30 cm)
    G4double Rayleigh[N] = {30.*cm, 30.*cm};
    // Scintillation spectrum (flat unit spectrum)
    G4double FastComp[N] = {1.0, 1.0};

    mpt->AddProperty("RINDEX",         E, RIndex,   N, true);
    mpt->AddProperty("ABSLENGTH",      E, AbsLen,   N, true);
    mpt->AddProperty("RAYLEIGH",       E, Rayleigh, N, true);
    mpt->AddProperty("FASTCOMPONENT",  E, FastComp, N, true);

    // Yields/timings (placeholders; tune later)
    mpt->AddConstProperty("SCINTILLATIONYIELD", 42000./MeV , true);
    mpt->AddConstProperty("RESOLUTIONSCALE",    1.0, true);
    mpt->AddConstProperty("FASTTIMECONSTANT",   2.2*ns, true);
    mpt->AddConstProperty("YIELDRATIO",         1.0, true); // all fast

    lxe_mat->SetMaterialPropertiesTable(mpt);
  }

  //
  // LXe active volume (central cylinder)
  //
  auto solidLXe = new G4Tubs("LXeSolid", 0.*cm, lxe_radius,
                             0.5*lxe_height, 0.*deg, 360.*deg);
  auto logicLXe = new G4LogicalVolume(solidLXe, lxe_mat, "LXe");

  new G4PVPlacement(nullptr, G4ThreeVector(0,0,0),
    logicLXe, "LXe", logicWorld, false, 0, checkOverlaps);

  //
  // GXe pocket (thin cylinder above LXe)
  //
  auto solidGXe = new G4Tubs("GXeSolid", 0.*cm, lxe_radius,
                             0.5*gxe_height, 0.*deg, 360.*deg);
  auto logicGXe = new G4LogicalVolume(solidGXe, gxe_mat, "GXe");

  new G4PVPlacement(nullptr,
    G4ThreeVector(0,0, 0.5*lxe_height + 0.5*gxe_height),
    logicGXe, "GXe", logicWorld, false, 0, checkOverlaps);

  //
  // PTFE wall (hollow cylindrical shell surrounding LXe + GXe)
  //
  auto solidWall = new G4Tubs("PTFEWallSolid",
    lxe_radius, lxe_radius + wall_thick,
    0.5 * (lxe_height + gxe_height), 0.*deg, 360.*deg);

  auto logicWall = new G4LogicalVolume(solidWall, ptfe_mat, "PTFEWall");

  new G4PVPlacement(nullptr,
    G4ThreeVector(0,0, 0.5*gxe_height),
    logicWall, "PTFEWall", logicWorld, false, 0, checkOverlaps);

  //
  // PMT planes (top & bottom) as thin disks
  //
  auto solidTopPMT = new G4Tubs("TopPMTSolid", 0.*cm, lxe_radius,
                                0.5*pmt_thick, 0.*deg, 360.*deg);
  auto logicTopPMT = new G4LogicalVolume(solidTopPMT, pmt_mat, "TopPMT");

  new G4PVPlacement(nullptr,
    G4ThreeVector(0,0, 0.5*lxe_height + gxe_height + 0.5*pmt_thick),
    logicTopPMT, "TopPMT", logicWorld, false, 0, checkOverlaps);

  auto solidBotPMT = new G4Tubs("BottomPMTSolid", 0.*cm, lxe_radius,
                                0.5*pmt_thick, 0.*deg, 360.*deg);
  auto logicBotPMT = new G4LogicalVolume(solidBotPMT, pmt_mat, "BottomPMT");

  new G4PVPlacement(nullptr,
    G4ThreeVector(0,0, -(0.5*lxe_height + 0.5*pmt_thick)),
    logicBotPMT, "BottomPMT", logicWorld, false, 0, checkOverlaps);

  //
  // Visualization attributes
  //
  auto visWorld = new G4VisAttributes(G4Colour(0,0,0,0));
  logicWorld->SetVisAttributes(visWorld);

  auto visLXe = new G4VisAttributes(G4Colour(0.2,0.4,1.0,0.3));
  visLXe->SetForceSolid(true);
  logicLXe->SetVisAttributes(visLXe);

  auto visGXe = new G4VisAttributes(G4Colour(0.7,0.9,1.0,0.2));
  visGXe->SetForceSolid(true);
  logicGXe->SetVisAttributes(visGXe);

  auto visWall = new G4VisAttributes(G4Colour(0.9,0.9,0.9,0.5));
  visWall->SetForceSolid(true);
  logicWall->SetVisAttributes(visWall);

  auto visTop = new G4VisAttributes(G4Colour(1.0,1.0,0.2,0.7));
  visTop->SetForceSolid(true);
  logicTopPMT->SetVisAttributes(visTop);

  auto visBot = new G4VisAttributes(G4Colour(1.0,1.0,0.2,0.7));
  visBot->SetForceSolid(true);
  logicBotPMT->SetVisAttributes(visBot);

  //
  // Set the scoring volume (useful for actions; here we use LXe)
  //
  fScoringVolume = logicLXe;

  //
  // always return the physical World
  //
  return physWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::ConstructSDandField()
{
  auto* sdman = G4SDManager::GetSDMpointer();

  // One SD handles both planes
  auto* pmtSD = new Test::PMTSensitiveDetector("PMTSD");
  sdman->AddNewDetector(pmtSD);

  // Attach SD to PMT logical volumes by name
  auto* topLV = G4LogicalVolumeStore::GetInstance()->GetVolume("TopPMT");
  auto* botLV = G4LogicalVolumeStore::GetInstance()->GetVolume("BottomPMT");

  if (topLV) topLV->SetSensitiveDetector(pmtSD);
  if (botLV) botLV->SetSensitiveDetector(pmtSD);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

} // namespace Test
