//

//
/// \file Test/src/PrimaryGeneratorAction.cc
/// \brief Implementation of the B1::PrimaryGeneratorAction class

#include "PrimaryGeneratorAction.hh"

#include "SimConfig.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4RunManager.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

// for ions
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
  fParticleGun  = new G4ParticleGun(1);

  // Safe placeholder; real definition set per event
  auto* gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
  fParticleGun->SetParticleDefinition(gamma);
  fParticleGun->SetParticleEnergy(10.*keV);
  fParticleGun->SetParticleMomentumDirection({0,0,1});
  fParticleGun->SetParticlePosition({0,0,0});
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  auto& cfg  = SimConfig::Get();
  auto& g    = cfg.geom;
  auto& gen  = cfg.gen;

  if (gen.use_wimp_proxy) {
    // Ensure ion definition exists
    auto* ion = G4IonTable::GetIonTable()->GetIon(gen.ion_Z, gen.ion_A, 0.*keV);

    if (ion) {
      fParticleGun->SetParticleDefinition(ion);
      fParticleGun->SetParticleCharge(0.*eplus);

      // --- Sample position uniformly in the LXe cylinder
      const G4double R = g.lxe_radius;
      const G4double H = g.lxe_height;

      G4double u   = G4UniformRand();
      G4double r   = R * std::sqrt(u);
      G4double phi = 2.*pi * G4UniformRand();
      G4double x0  = r * std::cos(phi);
      G4double y0  = r * std::sin(phi);
      G4double z0  = (G4UniformRand() - 0.5) * H;
      fParticleGun->SetParticlePosition(G4ThreeVector(x0,y0,z0));

      // --- Isotropic direction
      G4double cost  = 2.*G4UniformRand() - 1.;
      G4double sint  = std::sqrt(1. - cost*cost);
      G4double phiD  = 2.*pi * G4UniformRand();
      fParticleGun->SetParticleMomentumDirection(
        G4ThreeVector(sint*std::cos(phiD), sint*std::sin(phiD), cost));

      // --- Flat energy in [Emin, Emax]
      G4double Enr = gen.E_min + (gen.E_max - gen.E_min)*G4UniformRand();
      fParticleGun->SetParticleEnergy(Enr);
    } else {
      // Fallback to gamma if ion somehow not ready
      auto* gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
      fParticleGun->SetParticleDefinition(gamma);
      fParticleGun->SetParticleEnergy(gen.gamma_energy);
      fParticleGun->SetParticlePosition(gen.gamma_position);
      fParticleGun->SetParticleMomentumDirection(gen.gamma_direction);
    }
  } else {
    // Gamma calibration mode from config
    auto* gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
    fParticleGun->SetParticleDefinition(gamma);
    fParticleGun->SetParticleEnergy(gen.gamma_energy);
    fParticleGun->SetParticlePosition(gen.gamma_position);
    fParticleGun->SetParticleMomentumDirection(gen.gamma_direction);
  }

  fParticleGun->GeneratePrimaryVertex(anEvent);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

} // namespace Test
