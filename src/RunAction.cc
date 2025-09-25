// Test/src/RunAction.cc
#include "RunAction.hh"
#include "G4Run.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"
#include "SimConfig.hh"

namespace Test {

RunAction::RunAction()
: G4UserRunAction(),
  fEdep(0.), fEdep2(0.)
{
  auto* ana = G4AnalysisManager::Instance();
  ana->SetVerboseLevel(1);

  const auto& cfg = SimConfig::Get();

  // Nome ntuple in base alla categoria
  G4String ntName = "single";
  if (cfg.gen.event_category == 1) ntName = "double_near";
  else if (cfg.gen.event_category == 2) ntName = "double_far";
  else if (cfg.gen.event_category == 3) ntName = "triple";

  ana->CreateNtuple(ntName, ntName);
  ana->CreateNtupleIColumn(0, "event_id");
  ana->CreateNtupleIColumn(0, "nPhotTop");
  ana->CreateNtupleIColumn(0, "nPhotBot");
  ana->CreateNtupleDColumn(0, "Edep_LXe");         // MeV
  ana->CreateNtupleDColumn(0, "t_first_top_ns");
  ana->CreateNtupleDColumn(0, "t_first_bot_ns");
  ana->CreateNtupleDColumn(0, "t_mean_top_ns");
  ana->CreateNtupleDColumn(0, "t_mean_bot_ns");
  ana->FinishNtuple(0);
}

void RunAction::BeginOfRunAction(const G4Run*)
{
  fEdep = 0.; fEdep2 = 0.;
  auto* ana = G4AnalysisManager::Instance();
  // Il nome file e il tipo vengono dal macro:
  //   /analysis/setDefaultFileType csv
  //   /analysis/setFileName ../outputs/wimp_single
  ana->OpenFile();
}

void RunAction::EndOfRunAction(const G4Run* run)
{
  if (run->GetNumberOfEvent() == 0) return;

  auto* ana = G4AnalysisManager::Instance();
  ana->Write();
  ana->CloseFile();
}

void RunAction::AddEdep(G4double edep)
{
  fEdep  += edep;
  fEdep2 += edep*edep;
}

} // namespace Test
