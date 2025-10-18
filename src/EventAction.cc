//
/// \file Test/src/EventAction.cc
/// \brief Implementation of the B1::EventAction class

#include "EventAction.hh"
#include "RunAction.hh"
#include "PMTSensitiveDetector.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"

// C++ std
#include <algorithm>
#include <numeric>
#include <limits>

// Analysis
#include "G4AnalysisManager.hh"

namespace Test
{

EventAction::EventAction(RunAction* runAction)
: fRunAction(runAction)
{}

void EventAction::BeginOfEventAction(const G4Event*)
{
  fEdep = 0.;
}

void EventAction::EndOfEventAction(const G4Event* evt)
{
  // accumula energia nel run action
  fRunAction->AddEdep(fEdep);

  // --- prendi il SD corrente (thread-local) ---
  auto* sdman = G4SDManager::GetSDMpointer();
  auto* sd = dynamic_cast<Test::PMTSensitiveDetector*>(sdman->FindSensitiveDetector("PMTSD"));

  G4int nTop = 0, nBot = 0;
  G4double t_first_top = 0., t_first_bot = 0.;
  G4double t_mean_top  = 0., t_mean_bot  = 0.;

  if (sd) {
    nTop = sd->GetNTop();
    nBot = sd->GetNBot();

    const auto& vt = sd->TimesTop();
    const auto& vb = sd->TimesBot();

    if (!vt.empty()) {
      t_first_top = *std::min_element(vt.begin(), vt.end())/ns;
      t_mean_top  = std::accumulate(vt.begin(), vt.end(), 0.0)/vt.size()/ns;
    } else {
      t_first_top = t_mean_top = std::numeric_limits<double>::quiet_NaN();
    }

    if (!vb.empty()) {
      t_first_bot = *std::min_element(vb.begin(), vb.end())/ns;
      t_mean_bot  = std::accumulate(vb.begin(), vb.end(), 0.0)/vb.size()/ns;
    } else {
      t_first_bot = t_mean_bot = std::numeric_limits<double>::quiet_NaN();
    }
  }

  // --- analysis ntuple ---
  auto* ana = G4AnalysisManager::Instance();

  // ntuple 0: "single" (creata in RunAction)
  int ntupleId = 0;

  ana->FillNtupleIColumn(ntupleId, 0, evt->GetEventID());
  ana->FillNtupleIColumn(ntupleId, 1, nTop);
  ana->FillNtupleIColumn(ntupleId, 2, nBot);
  ana->FillNtupleDColumn(ntupleId, 3, fEdep/MeV);
  ana->FillNtupleDColumn(ntupleId, 4, t_first_top);
  ana->FillNtupleDColumn(ntupleId, 5, t_first_bot);
  ana->FillNtupleDColumn(ntupleId, 6, t_mean_top);
  ana->FillNtupleDColumn(ntupleId, 7, t_mean_bot);
  ana->AddNtupleRow(ntupleId);

  // pulisci lo stato del SD per l’evento successivo
  if (sd) sd->Clear();
}

} // namespace Test
