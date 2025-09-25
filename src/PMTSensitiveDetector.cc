#include "PMTSensitiveDetector.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"
#include "G4ios.hh"

namespace Test {

PMTSensitiveDetector::PMTSensitiveDetector(const G4String& name)
: G4VSensitiveDetector(name) {}

G4bool PMTSensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory*)
{
  auto* trk = step->GetTrack();
  if (trk->GetDefinition() != G4OpticalPhoton::Definition()) return false;

  auto name = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetName();
  G4double t = step->GetPostStepPoint()->GetGlobalTime(); // ns

  if (name == "TopPMT")    { ++fTop; fTimesTop.push_back(t); }
  if (name == "BottomPMT") { ++fBot; fTimesBot.push_back(t); }

  trk->SetTrackStatus(fStopAndKill); // assorbi al photocathode
  return true;
}

void PMTSensitiveDetector::EndOfEvent(G4HCofThisEvent*) {
  // nessuna stampa; il fill è in EventAction
}

void PMTSensitiveDetector::Clear() {
  fTop = fBot = 0;
  fTimesTop.clear();
  fTimesBot.clear();
}

} // namespace Test
