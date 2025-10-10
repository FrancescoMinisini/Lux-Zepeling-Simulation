#ifndef LZSIM_STEPPINGACTION_HH
#define LZSIM_STEPPINGACTION_HH

#include "G4UserSteppingAction.hh"
#include "G4LogicalVolume.hh"
#include "globals.hh"

namespace LZSim {
class EventAction;  // Move forward declaration inside namespace

class SteppingAction : public G4UserSteppingAction {
public:
  SteppingAction(EventAction* eventAction);
  void UserSteppingAction(const G4Step*) override;

private:
  EventAction* fEventAction = nullptr;
  G4LogicalVolume* fScoringVolume = nullptr;
};
} // namespace LZSim

#endif