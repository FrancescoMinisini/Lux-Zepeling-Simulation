//

//
/// \file Test/include/PrimaryGeneratorAction.hh
/// \brief Definition of the B1::PrimaryGeneratorAction class

#ifndef B1PrimaryGeneratorAction_h
#define B1PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "globals.hh"
#include "SimConfig.hh"

class G4ParticleGun;
class G4Event;
class G4Box;

namespace Test
{

  /// The primary generator action class with particle gun.
  ///
  /// The default kinematic is a 6 MeV gamma, randomly distribued
  /// in front of the phantom across 80% of the (X,Y) phantom size.

  enum class EventCategory
  {
    Single = 0,
    DoubleNear = 1,
    DoubleFar = 2,
    Triple = 3
  };

  class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
  {
  public:
    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction() override;

    // method from the base class
    void GeneratePrimaries(G4Event *) override;

    // method to access particle gun
    const G4ParticleGun *GetParticleGun() const { return fParticleGun; }
    // helper per generator
    void GenerateCategory(EventCategory cat, G4Event *evt);
    G4ThreeVector SamplePosInLXe(const SimConfig::Geometry &g);

  private:
    G4ParticleGun *fParticleGun = nullptr; // pointer a to G4 gun class
    G4Box *fEnvelopeBox = nullptr;
  };

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
