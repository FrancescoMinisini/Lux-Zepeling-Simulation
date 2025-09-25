#include "EventAction.hh"
#include "RunAction.hh"
#include "SimConfig.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4AnalysisManager.hh"

#include "PMTSensitiveDetector.hh"

#include <algorithm>
#include <limits>

namespace {
  inline G4double mean_or_nan(const std::vector<G4double>& v){
    if (v.empty()) return std::numeric_limits<G4double>::quiet_NaN();
    double s=0; for (auto x: v) s+=x; return s/v.size();
  }
  inline G4double first_or_nan(const std::vector<G4double>& v){
    if (v.empty()) return std::numeric_limits<G4double>::quiet_NaN();
    return *std::min_element(v.begin(), v.end());
  }
}

namespace Test {

EventAction::EventAction(RunAction* runAction)
: fRunAction(runAction)
{
  auto* sdman = G4SDManager::GetSDMpointer();
  fPMTSD = static_cast<PMTSensitiveDetector*>(sdman->FindSensitiveDetector("PMTSD"));
}

void EventAction::BeginOfEventAction(const G4Event*)
{
  fEdep = 0.;
}

void EventAction::EndOfEventAction(const G4Event* evt)
{
  G4int nTop = fPMTSD ? fPMTSD->GetNTop() : 0;
  G4int nBot = fPMTSD ? fPMTSD->GetNBot() : 0;
  const auto& tTop = fPMTSD ? fPMTSD->TimesTop() : *(new std::vector<G4double>());
  const auto& tBot = fPMTSD ? fPMTSD->TimesBot() : *(new std::vector<G4double>());

  G4double t_first_top = first_or_nan(tTop);
  G4double t_first_bot = first_or_nan(tBot);
  G4double t_mean_top  = mean_or_nan(tTop);
  G4double t_mean_bot  = mean_or_nan(tBot);

  auto& cfg = SimConfig::Get();
  int cat = cfg.gen.event_category; // 0..3

  auto* ana = G4AnalysisManager::Instance();
  auto fill = [&](int ntupleId){
    ana->SetNtupleIColumn(ntupleId, 0, evt->GetEventID());
    ana->SetNtupleIColumn(ntupleId, 1, nTop);
    ana->SetNtupleIColumn(ntupleId, 2, nBot);
    ana->SetNtupleDColumn(ntupleId, 3, fEdep/MeV);
    ana->SetNtupleDColumn(ntupleId, 4, t_first_top/ns);
    ana->SetNtupleDColumn(ntupleId, 5, t_first_bot/ns);
    ana->SetNtupleDColumn(ntupleId, 6, t_mean_top/ns);
    ana->SetNtupleDColumn(ntupleId, 7, t_mean_bot/ns);
    ana->AddNtupleRow(ntupleId);
  };

  if      (cat==0) fill(0);
  else if (cat==1) fill(1);
  else if (cat==2) fill(2);
  else if (cat==3) fill(3);

  if (fPMTSD) fPMTSD->Clear();

  fRunAction->AddEdep(fEdep);
}

} // namespace Test
