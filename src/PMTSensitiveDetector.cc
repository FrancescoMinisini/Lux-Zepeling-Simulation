//
/// \file Test/src/PMTSensitiveDetector.cc
#include "PMTSensitiveDetector.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"
#include "G4ios.hh"

namespace Test
{

PMTSensitiveDetector::PMTSensitiveDetector(const G4String& name)
: G4VSensitiveDetector(name) {}

G4bool PMTSensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory*) {
  auto* trk = step->GetTrack();
  if (trk->GetDefinition() != G4OpticalPhoton::Definition()) return false;

  auto name = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetName();
  auto t    = trk->GetGlobalTime();

  if (name == "TopPMT")    { ++fTop; fTimesTop.push_back(t); }
  if (name == "BottomPMT") { ++fBot; fTimesBot.push_back(t); }

  trk->SetTrackStatus(fStopAndKill);
  return true;
}

void PMTSensitiveDetector::EndOfEvent(G4HCofThisEvent*) {
  G4cout << "[PMT] Top=" << fTop << "  Bottom=" << fBot << G4endl;
  // non pulire qui: lo farà EventAction dopo aver letto
}

} // namespace Test
