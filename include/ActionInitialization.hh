#ifndef LZSIM_ACTIONINITIALIZATION_HH
#define LZSIM_ACTIONINITIALIZATION_HH

#include "G4VUserActionInitialization.hh"

namespace LZSim {
class ActionInitialization : public G4VUserActionInitialization {
public:
  void BuildForMaster() const override;
  void Build() const override;
};
} // namespace LZSim

#endif