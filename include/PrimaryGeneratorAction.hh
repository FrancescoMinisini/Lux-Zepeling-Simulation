#ifndef LZSIM_PRIMARYGENERATORACTION_HH
#define LZSIM_PRIMARYGENERATORACTION_HH

#include "G4VUserPrimaryGeneratorAction.hh"

class G4ParticleGun;

namespace LZSim {
class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
public:
  PrimaryGeneratorAction();
  ~PrimaryGeneratorAction() override;
  void GeneratePrimaries(G4Event* anEvent) override;

private:
  G4ParticleGun* fParticleGun = nullptr;
};
} // namespace LZSim

#endif