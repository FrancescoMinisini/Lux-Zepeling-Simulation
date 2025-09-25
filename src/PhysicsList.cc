#include "PhysicsList.hh"
#include "G4IonPhysics.hh"
#include "SimConfig.hh"
#include "G4DecayPhysics.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4OpticalPhysics.hh"
#include "G4OpticalParameters.hh"
#include "G4GenericIon.hh"

namespace Test {

PhysicsList::PhysicsList() {
  RegisterPhysics(new G4EmStandardPhysics_option4());
  RegisterPhysics(new G4DecayPhysics());
  RegisterPhysics(new G4IonPhysics());

  auto* opt = new G4OpticalPhysics();
  RegisterPhysics(opt);

  auto* op = G4OpticalParameters::Instance();
  auto& cfg = SimConfig::Get().optProc;
  op->SetScintByParticleType(cfg.scint_by_particle_type);
  op->SetScintTrackSecondariesFirst(cfg.scint_track_sec_first);
  op->SetCerenkovMaxPhotonsPerStep(cfg.cerenkov_enable ? cfg.cerenkov_max_photons : 0);
  op->SetCerenkovTrackSecondariesFirst(cfg.cerenkov_track_sec_first);
}

void PhysicsList::ConstructParticle() {
  G4VModularPhysicsList::ConstructParticle();
  G4GenericIon::GenericIonDefinition();
}

void PhysicsList::ConstructProcess() {
  G4VModularPhysicsList::ConstructProcess();
}

} // namespace Test
