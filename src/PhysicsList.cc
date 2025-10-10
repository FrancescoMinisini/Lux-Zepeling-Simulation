#include "PhysicsList.hh"
#include "G4SystemOfUnits.hh"
// Decay + EM
#include "G4DecayPhysics.hh"
#include "G4EmStandardPhysics_option4.hh"
// Optical
#include "G4OpticalPhysics.hh"
#include "G4OpticalParameters.hh"
// Ion physics
#include "G4IonPhysics.hh"

namespace Test {
PhysicsList::PhysicsList() : G4VModularPhysicsList() {
  defaultCutValue = 0.1*mm;
  SetVerboseLevel(1);
  // Basic physics
  RegisterPhysics(new G4DecayPhysics());
  RegisterPhysics(new G4EmStandardPhysics_option4());
  // Ion physics for Xe131
  RegisterPhysics(new G4IonPhysics());
  // Optical physics
  auto* opt = new G4OpticalPhysics();
  opt->SetVerboseLevel(1);
  RegisterPhysics(opt);
}
void PhysicsList::SetCuts() {
  SetCutsWithDefault();
  if (verboseLevel > 0) DumpCutValuesTable();
}
void PhysicsList::ConstructProcess() {
  G4VModularPhysicsList::ConstructProcess();
  auto* op = G4OpticalParameters::Instance();
  op->SetScintByParticleType(false);
  op->SetScintTrackSecondariesFirst(true);
  op->SetCerenkovTrackSecondariesFirst(false);
  op->SetCerenkovMaxPhotonsPerStep(0); // Disable Cerenkov
  op->SetScintVerboseLevel(2); // Detailed scintillation debug
  op->SetCerenkovVerboseLevel(0);
}
} // namespace Test