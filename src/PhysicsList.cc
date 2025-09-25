// Test/src/PhysicsList.cc
#include "PhysicsList.hh"
#include "G4SystemOfUnits.hh"

// Decay + EM
#include "G4DecayPhysics.hh"
#include "G4EmStandardPhysics_option4.hh"

// Ottica
#include "G4OpticalPhysics.hh"
#include "G4OpticalParameters.hh"

namespace Test {

PhysicsList::PhysicsList() : G4VModularPhysicsList() {
  defaultCutValue = 0.1*mm;
  SetVerboseLevel(1);

  // Fisica "di base"
  RegisterPhysics(new G4DecayPhysics());
  RegisterPhysics(new G4EmStandardPhysics_option4());

  // Ottica
  auto* opt = new G4OpticalPhysics();
  RegisterPhysics(opt);
}

void PhysicsList::SetCuts() {
  SetCutsWithDefault();
  if (verboseLevel > 0) DumpCutValuesTable();
}

void PhysicsList::ConstructProcess() {
  // Costruisce i processi da tutte le physics registrate
  G4VModularPhysicsList::ConstructProcess();

  // Parametri ottici per Geant4 11.2.x
  auto* op = G4OpticalParameters::Instance();

  // Scintillazione per tipo di particella
  op->SetScintByParticleType(false);

  // Traccia prima le secondarie (utile per tempi)
  op->SetScintTrackSecondariesFirst(true);
  op->SetCerenkovTrackSecondariesFirst(true);

  // Verbose basso
  op->SetScintVerboseLevel(0);
  op->SetCerenkovVerboseLevel(0);
}

} // namespace Test
