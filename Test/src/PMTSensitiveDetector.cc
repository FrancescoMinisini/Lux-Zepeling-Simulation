//
/// \file B1/src/PMTSensitiveDetector.cc
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

  auto name = step->GetPreStepPoint()->GetTouchableHandle()
                ->GetVolume()->GetName();

  if (name == "TopPMT")    ++fTop;
  if (name == "BottomPMT") ++fBot;

  trk->SetTrackStatus(fStopAndKill); // absorb at photocathode
  return true;
}

void PMTSensitiveDetector::EndOfEvent(G4HCofThisEvent*) {
  G4cout << "[PMT] Top=" << fTop << "  Bottom=" << fBot << G4endl;
  fTop = fBot = 0;
}

} // namespace Test
