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
  for (G4int i = 0; i < gen.n_electrons; ++i) {
    // Posizione random per simulare pacchetto (con diffusione iniziale piccola)
    G4double dx = G4RandGauss::shoot(0., 0.1 * mm);  // Diffusione trasversale iniziale
    G4double dy = G4RandGauss::shoot(0., 0.1 * mm);
    G4ThreeVector pos = gen.electron_position + G4ThreeVector(dx, dy, 0.);
    fParticleGun->SetParticlePosition(pos);
  
    // Direzione con piccola spread
    G4double theta = G4RandGauss::shoot(0., 0.01);  // Piccolo angolo
    G4ThreeVector dir = G4ThreeVector(gen.electron_direction).rotate(theta, G4ThreeVector(1,0,0));
    fParticleGun->SetParticleMomentumDirection(dir);
  
    // Energia random per variabilità (opzionale)
    G4double e_energy = G4RandGauss::shoot(gen.electron_energy, 0.1 * gen.electron_energy);
    fParticleGun->SetParticleEnergy(e_energy);
  
    fParticleGun->GeneratePrimaryVertex(anEvent);
  }
}
} // namespace LZSim