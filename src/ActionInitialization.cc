#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"
#include "G4RunManager.hh"
#include "G4AnalysisManager.hh"

namespace LZSim {
void ActionInitialization::BuildForMaster() const {
  (void) G4AnalysisManager::Instance();
  SetUserAction(new RunAction());
}

void ActionInitialization::Build() const {
  (void) G4AnalysisManager::Instance();
  SetUserAction(new PrimaryGeneratorAction());
  auto runAction = new RunAction();
  SetUserAction(runAction);
  auto eventAction = new EventAction(runAction);
  SetUserAction(eventAction);
  SetUserAction(new SteppingAction(eventAction));
}
} // namespace LZSim