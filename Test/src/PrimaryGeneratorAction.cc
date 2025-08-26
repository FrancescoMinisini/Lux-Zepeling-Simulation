//

//
#include "PrimaryGeneratorAction.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4RunManager.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

// --- added for Xe nuclear recoils (WIMP-like proxy)
#include "G4IonTable.hh"
#include "G4ThreeVector.hh"
#include "G4PhysicalConstants.hh"

namespace Test
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// PrimaryGeneratorAction::PrimaryGeneratorAction()
// {
//   // number of primaries per event
//   G4int n_particle = 1;
//   fParticleGun  = new G4ParticleGun(n_particle);

//   // default particle kinematic (WIMP-like proxy: xenon nuclear recoil)
//   //
//   // We generate a neutral Xe ion (e.g., Xe-131) that recoils with keV energy.
//   // Direction, energy and position will be (re)assigned each event in GeneratePrimaries().
//   G4int Z = 54;          // Xenon
//   G4int A = 131;         // isotope (choice not critical here)
//   auto* ion = G4IonTable::GetIonTable()->GetIon(Z, A, 0.*keV);

//   fParticleGun->SetParticleDefinition(ion);
//   fParticleGun->SetParticleCharge(0.*eplus);
//   fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0.,0.,1.));
//   fParticleGun->SetParticleEnergy(10.*keV);       // placeholder; overwritten per event
//   fParticleGun->SetParticlePosition(G4ThreeVector(0.,0.,0.));
// }

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
  fParticleGun = new G4ParticleGun(1);

  // Set a temporary safe particle (gamma) — will be replaced per event.
  auto* gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
  fParticleGun->SetParticleDefinition(gamma);
  fParticleGun->SetParticleEnergy(10.*keV);
  fParticleGun->SetParticleMomentumDirection({0,0,1});
  fParticleGun->SetParticlePosition({0,0,0});
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fParticleGun;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  // this function is called at the beginning of each event
  // Here we sample directly inside the LXe cylinder we defined
  // in DetectorConstruction (radius = 30 cm, height = 50 cm).

  // Ensure ion definition exists (workers are initialized now)
  const G4int Z = 54, A = 131;
  auto* ion = G4IonTable::GetIonTable()->GetIon(Z, A, 0.*keV);

  if (ion) {
    fParticleGun->SetParticleDefinition(ion);
    fParticleGun->SetParticleCharge(0.*eplus);

    G4cout << "-------- Succesfully used ION ----------" << G4endl;

  } else {
    // Fallback: if ion not ready for some reason, use gamma so we don’t crash
    auto* gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
    fParticleGun->SetParticleDefinition(gamma);
        G4cout << "-------- UnSuccesfully used ION ----------" << G4endl;
  }


  // --- Geometry parameters of the LXe active volume (keep in sync with DetectorConstruction)
  const G4double rTPC = 30.*cm;
  const G4double hLXe = 50.*cm;

  // --- 1) Sample a position uniformly inside a cylinder
  // radial pdf ~ r (uniform in area): r = R * sqrt(u), phi in [0, 2pi)
  G4double u   = G4UniformRand();
  G4double r   = rTPC * std::sqrt(u);
  G4double phi = 2.*pi * G4UniformRand();
  G4double x0  = r * std::cos(phi);
  G4double y0  = r * std::sin(phi);
  G4double z0  = (G4UniformRand() - 0.5) * hLXe;  // uniform in [-h/2, +h/2]

  fParticleGun->SetParticlePosition(G4ThreeVector(x0,y0,z0));

  // --- 2) Sample an isotropic recoil direction
  G4double cost  = 2.*G4UniformRand() - 1.;
  G4double sint  = std::sqrt(1. - cost*cost);
  G4double phiD  = 2.*pi * G4UniformRand();
  G4ThreeVector dir(sint*std::cos(phiD), sint*std::sin(phiD), cost);
  fParticleGun->SetParticleMomentumDirection(dir);

  // --- 3) Sample a nuclear-recoil energy in [1, 30] keV_nr (toy flat spectrum)
  G4double Emin = 1.*keV;
  G4double Emax = 30.*keV;
  G4double Enr  = Emin + (Emax - Emin) * G4UniformRand();
  fParticleGun->SetParticleEnergy(Enr);

  // fire the primary
  fParticleGun->GeneratePrimaryVertex(anEvent);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

}
