#ifndef LZSIM_EVENTACTION_HH
#define LZSIM_EVENTACTION_HH

#include "G4UserEventAction.hh"
#include "globals.hh"

namespace LZSim {
class RunAction;  // Move forward declaration inside namespace

class EventAction : public G4UserEventAction {
public:
  EventAction(RunAction* runAction);
  void BeginOfEventAction(const G4Event* event) override;
  void EndOfEventAction(const G4Event* event) override;
  void AddEdep(G4double edep) { fEdep += edep; }

private:
  RunAction* fRunAction = nullptr;
  G4double fEdep = 0.;
};
} // namespace LZSim

#endif