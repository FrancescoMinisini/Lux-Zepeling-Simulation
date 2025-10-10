#include "PrimaryGeneratorAction.hh"
#include "SimConfig.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

namespace LZSim {
PrimaryGeneratorAction::PrimaryGeneratorAction() {
  fParticleGun = new G4ParticleGun(1);
  auto* electron = G4ParticleTable::GetParticleTable()->FindParticle("e-");
  fParticleGun->SetParticleDefinition(electron);
  fParticleGun->SetParticleEnergy(5. * keV);  // Default, override via config
  fParticleGun->SetParticleMomentumDirection({0, 0, 1});
  fParticleGun->SetParticlePosition({0, 0, -0.5 * cm});  // Bottom
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
  delete fParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
  auto& cfg = SimConfig::Get();
  auto& gen = cfg.gen;
  fParticleGun->SetParticleEnergy(gen.electron_energy);
  fParticleGun->SetParticlePosition(gen.electron_position);
  fParticleGun->SetParticleMomentumDirection(gen.electron_direction);
  fParticleGun->GeneratePrimaryVertex(anEvent);
}
} // namespace LZSim