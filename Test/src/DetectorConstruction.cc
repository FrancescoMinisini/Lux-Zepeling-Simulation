//

//
/// \file B1/src/DetectorConstruction.cc
/// \brief Implementation of the B1::DetectorConstruction class

#include "DetectorConstruction.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Orb.hh"
#include "G4Sphere.hh"
#include "G4Trd.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Tubs.hh"


namespace Test
{

  //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

  G4VPhysicalVolume *DetectorConstruction::Construct()
  {
    // Get nist material manager
    G4NistManager *nist = G4NistManager::Instance();

    // Envelope parameters
    //
    G4double env_sizeXY = 20 * cm, env_sizeZ = 30 * cm;

    // Option to switch on/off checking of volumes overlaps
    //
    G4bool checkOverlaps = true;

    //
    // World
    //
    G4double world_sizeXY = 1.2 * env_sizeXY;
    G4double world_sizeZ = 1.2 * env_sizeZ;
    G4Material *world_mat = nist->FindOrBuildMaterial("G4_AIR");

    auto solidWorld = new G4Box("World",                                                    // its name
                                0.5 * world_sizeXY, 0.5 * world_sizeXY, 0.5 * world_sizeZ); // its size

    auto logicWorld = new G4LogicalVolume(solidWorld, // its solid
                                          world_mat,  // its material
                                          "World");   // its name

    auto physWorld = new G4PVPlacement(nullptr,         // no rotation
                                       G4ThreeVector(), // at (0,0,0)
                                       logicWorld,      // its logical volume
                                       "World",         // its name
                                       nullptr,         // its mother  volume
                                       false,           // no boolean operation
                                       0,               // copy number
                                       checkOverlaps);  // overlaps checking

    //
    // LXe active volume (first shape)
    //
    // Define a simple Liquid Xenon material (placeholder without optical properties)
    G4Material *lxe_mat = new G4Material("LXe", 54., 131.29 * g / mole, 2.90 * g / cm3);

    // Cylinder dimensions for the LXe (TPC-like)
    G4double lxe_radius = 30 * cm;
    G4double lxe_height = 50 * cm;

    auto solidLXe = new G4Tubs("LXeSolid",            // its name
                               0. * cm,               // inner radius (solid)
                               lxe_radius,            // outer radius
                               0.5 * lxe_height,      // half-length (z)
                               0. * deg, 360. * deg); // full 360 degrees

    auto logicLXe = new G4LogicalVolume(solidLXe, // its solid
                                        lxe_mat,  // its material
                                        "LXe");   // its name

    new G4PVPlacement(nullptr,                // no rotation
                      G4ThreeVector(0, 0, 0), // at (0,0,0)
                      logicLXe,               // its logical volume
                      "LXe",                  // its name
                      logicWorld,             // its mother volume
                      false,                  // no boolean operation
                      0,                      // copy number
                      checkOverlaps);         // overlaps checking

    // Set LXe as scoring volume (if actions rely on it)
    //
    fScoringVolume = logicLXe;


    //
    // always return the physical World
    //
    return physWorld;
  }

  //....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}
