#include "RunAction.hh"
#include "G4Run.hh"
#include "G4AccumulableManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "SimConfig.hh"

namespace LZSim {
RunAction::RunAction() : G4UserRunAction() {}

G4Run* RunAction::GenerateRun() {
  return new G4Run();
}

void RunAction::BeginOfRunAction(const G4Run*) {
  G4AccumulableManager::Instance()->Reset();
  auto* ana = G4AnalysisManager::Instance();
  ana->SetVerboseLevel(0);
  ana->CreateNtuple("s2", "S2 Signal");
  ana->CreateNtupleIColumn(0, "event_id");
  ana->CreateNtupleIColumn(0, "nPhotTop");
  ana->CreateNtupleDColumn(0, "Edep_GXe");  // MeV
  ana->CreateNtupleDColumn(0, "t_first_top_ns");
  ana->CreateNtupleDColumn(0, "t_mean_top_ns");
  ana->FinishNtuple(0);
  ana->OpenFile();
}

void RunAction::EndOfRunAction(const G4Run* run) {
  if (run->GetNumberOfEvent() == 0) return;
  auto* ana = G4AnalysisManager::Instance();
  ana->Write();
  ana->CloseFile();
}

void RunAction::AddEdep(G4double edep) {
  fEdep += edep;
  fEdep2 += edep * edep;
  G4AccumulableManager::Instance()->Merge();
}
} // namespace LZSim