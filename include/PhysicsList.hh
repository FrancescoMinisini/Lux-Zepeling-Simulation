
/// \file Test/include/PhysicsList.hh
/// \brief Definition of the Test::PhysicsList class
#ifndef Test_PhysicsList_h
#define Test_PhysicsList_h 1

#include "G4VModularPhysicsList.hh"

namespace Test
{

  class PhysicsList : public G4VModularPhysicsList
  {
  public:
    PhysicsList();
    ~PhysicsList() override = default;
    void ConstructParticle() override;
    void ConstructProcess() override;
  };

} // namespace Test
#endif
