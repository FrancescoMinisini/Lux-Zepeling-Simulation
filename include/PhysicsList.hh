#ifndef LZSIM_PHYSICSLIST_HH
#define LZSIM_PHYSICSLIST_HH

#include "G4VModularPhysicsList.hh"

namespace LZSim {
class PhysicsList : public G4VModularPhysicsList {
public:
  PhysicsList();
  void SetCuts() override;
  void ConstructProcess() override;
};
} // namespace LZSim

#endif