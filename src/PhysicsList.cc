#include "PhysicsList.hh"
#include "G4SystemOfUnits.hh"
#include "G4DecayPhysics.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4OpticalPhysics.hh"
#include "G4OpticalParameters.hh"
#include "G4IonPhysics.hh"
#include "G4EmParameters.hh"

namespace LZSim {
PhysicsList::PhysicsList() : G4VModularPhysicsList() {
  defaultCutValue = 0.1 * mm;
  SetVerboseLevel(0);
  RegisterPhysics(new G4DecayPhysics());
  RegisterPhysics(new G4EmStandardPhysics_option4());  // Good for low-E electrons
  RegisterPhysics(new G4IonPhysics());
  auto* opt = new G4OpticalPhysics();
  opt->SetVerboseLevel(0);
  RegisterPhysics(opt);
  G4EmParameters::Instance()->SetLateralDisplacement(true);  // Abilita random walk trasversale
  G4EmParameters::Instance()->SetMuHadLateralDisplacement(true);  // Opzionale per hadrons, ma utile
}

void PhysicsList::SetCuts() {
  SetCutsWithDefault();
  if (verboseLevel > 0) DumpCutValuesTable();
}

void PhysicsList::ConstructProcess() {
  G4VModularPhysicsList::ConstructProcess();
  auto* op = G4OpticalParameters::Instance();
  op->SetScintByParticleType(true);  // True per particle-specific, with warning Birks ok
  op->SetScintTrackSecondariesFirst(true);
  op->SetCerenkovTrackSecondariesFirst(false);
  op->SetCerenkovMaxPhotonsPerStep(0);
  op->SetScintVerboseLevel(0);
  op->SetCerenkovVerboseLevel(0);
  G4EmParameters::Instance()->SetNumberOfBinsPerDecade(100);  // Migliora precisione per low-E
}
} // namespace LZSim