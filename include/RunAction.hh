#ifndef LZSIM_RUNACTION_HH
#define LZSIM_RUNACTION_HH

#include "G4UserRunAction.hh"
#include "G4Accumulable.hh"
#include "globals.hh"

namespace LZSim {
class RunAction : public G4UserRunAction {
public:
  RunAction();
  G4Run* GenerateRun() override;
  void BeginOfRunAction(const G4Run*) override;
  void EndOfRunAction(const G4Run*) override;
  void AddEdep(G4double edep);

private:
  G4Accumulable<G4double> fEdep = 0.;
  G4Accumulable<G4double> fEdep2 = 0.;
};
} // namespace LZSim

#endif