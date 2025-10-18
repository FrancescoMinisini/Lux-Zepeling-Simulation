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

#include "G4IonTable.hh"
#include "G4ThreeVector.hh"
#include "G4PhysicalConstants.hh"

namespace Test
{

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
  fParticleGun  = new G4ParticleGun(1);
  auto* gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
  fParticleGun->SetParticleDefinition(gamma);
  fParticleGun->SetParticleEnergy(1.*GeV);
  // fParticleGun->SetParticleEnergy(10.*keV);
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
    auto* ion = G4IonTable::GetIonTable()->GetIon(gen.ion_Z, gen.ion_A, 0.*keV);
    if (ion) {
      fParticleGun->SetParticleDefinition(ion);

      // IMPORTANT: usa carica +1e per assicurare dE/dx -> scintillazione
      // (proxy WIMP finché non colleghiamo un modello NEST)
      fParticleGun->SetParticleCharge(+1.*eplus);

      // posizione uniforme nel cilindro LXe
      const G4double R = g.lxe_radius;
      const G4double H = g.lxe_height;
      G4double u   = G4UniformRand();
      G4double r   = R * std::sqrt(u);
      G4double phi = 2.*pi * G4UniformRand();
      G4double x0  = r * std::cos(phi);
      G4double y0  = r * std::sin(phi);
      G4double z0  = (G4UniformRand() - 0.5) * H;
      fParticleGun->SetParticlePosition(G4ThreeVector(x0,y0,z0));

      // direzione isotropa
      G4double cost  = 2.*G4UniformRand() - 1.;
      G4double sint  = std::sqrt(1. - cost*cost);
      G4double phiD  = 2.*pi * G4UniformRand();
      fParticleGun->SetParticleMomentumDirection(
        G4ThreeVector(sint*std::cos(phiD), sint*std::sin(phiD), cost));

      // energia piatta in [Emin, Emax]
      G4double Enr = gen.E_min + (gen.E_max - gen.E_min)*G4UniformRand();
      fParticleGun->SetParticleEnergy(Enr);
    } else {
      auto* gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
      fParticleGun->SetParticleDefinition(gamma);
      fParticleGun->SetParticleEnergy(gen.gamma_energy);
      fParticleGun->SetParticlePosition(gen.gamma_position);
      fParticleGun->SetParticleMomentumDirection(gen.gamma_direction);
    }
  } else {
    auto* gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
    fParticleGun->SetParticleDefinition(gamma);
    fParticleGun->SetParticleEnergy(gen.gamma_energy);
    fParticleGun->SetParticlePosition(gen.gamma_position);
    fParticleGun->SetParticleMomentumDirection(gen.gamma_direction);
  }

  fParticleGun->GeneratePrimaryVertex(anEvent);
}

} // namespace Test
