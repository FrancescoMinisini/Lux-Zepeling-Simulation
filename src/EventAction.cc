// src/EventAction.cc
#include "EventAction.hh"
#include "RunAction.hh"
#include "PMTSensitiveDetector.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include <algorithm>
#include <numeric>
#include <limits>
#include "G4AnalysisManager.hh"

namespace LZSim {
EventAction::EventAction(RunAction* runAction) : fRunAction(runAction) {}

void EventAction::BeginOfEventAction(const G4Event*) {
  fEdep = 0.;
}

void EventAction::EndOfEventAction(const G4Event* evt) {
  fRunAction->AddEdep(fEdep);
  auto* sdman = G4SDManager::GetSDMpointer();
  auto* sd = dynamic_cast<PMTSensitiveDetector*>(sdman->FindSensitiveDetector("PMTSD"));
  G4int nTop = 0;
  G4double t_first_top = 0., t_mean_top = 0.;
  if (sd) {
    nTop = sd->GetNTop();
    const auto& vt = sd->TimesTop();
    if (!vt.empty()) {
      t_first_top = *std::min_element(vt.begin(), vt.end()) / ns;
      t_mean_top = std::accumulate(vt.begin(), vt.end(), 0.0) / vt.size() / ns;
    } else {
      t_first_top = t_mean_top = 0.;  // Imposta a 0 invece di NaN
    }
  }
  auto* ana = G4AnalysisManager::Instance();
  int ntupleId = 0;
  ana->FillNtupleIColumn(ntupleId, 0, evt->GetEventID());
  ana->FillNtupleIColumn(ntupleId, 1, nTop);
  ana->FillNtupleDColumn(ntupleId, 2, fEdep / MeV);
  ana->FillNtupleDColumn(ntupleId, 3, t_first_top);
  ana->FillNtupleDColumn(ntupleId, 4, t_mean_top);
  ana->AddNtupleRow(ntupleId);
  if (sd) sd->Clear();
}
} // namespace LZSim