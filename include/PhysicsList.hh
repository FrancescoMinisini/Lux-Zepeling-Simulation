#pragma once
#include "G4VModularPhysicsList.hh"

namespace Test {

class PhysicsList : public G4VModularPhysicsList {
public:
  PhysicsList();
  ~PhysicsList() override = default;

  // Costruisce i processi dalle physics registrate
  void ConstructProcess() override;

  // Tagli di produzione (deve esistere anche nell'header!)
  void SetCuts() override;
};

} // namespace Test
