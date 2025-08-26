//
/// \file Test/src/PhysicsList.cc
/// \brief Implementation of the Test::PhysicsList class
#include "PhysicsList.hh"
#include "G4IonPhysics.hh"          
#include "G4DecayPhysics.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4OpticalPhysics.hh"
#include "G4OpticalParameters.hh"

namespace Test
{

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PhysicsList::PhysicsList() {
  // EM physics tuned for low-energy transport
  RegisterPhysics(new G4EmStandardPhysics_option4());

  // Decays 
  RegisterPhysics(new G4DecayPhysics());

  RegisterPhysics(new G4IonPhysics());

  // Optics: scintillation, absorption, Rayleigh, boundaries, etc.
  auto* opt = new G4OpticalPhysics(); 
  RegisterPhysics(opt);
  
  // Configure optical options via the singleton (G4 11.x API)
  auto* op = G4OpticalParameters::Instance();
  op->SetScintByParticleType(true);          // was SetScintillationByParticleType
  op->SetScintTrackSecondariesFirst(true);   // replaces SetTrackSecondariesFirst(kScintillation, ...)
  op->SetCerenkovMaxPhotonsPerStep(0);       // optional: disable/limit Cherenkov if you don't need it
  op->SetCerenkovTrackSecondariesFirst(false);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

} // namespace Test
