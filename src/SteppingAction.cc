#include "SteppingAction.hh"
#include "EventAction.hh"
#include "DetectorConstruction.hh"
#include "G4Step.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4LogicalVolume.hh"
#include "SimConfig.hh"

namespace Test {
SteppingAction::SteppingAction(EventAction* eventAction)
: fEventAction(eventAction) {}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  if (!fScoringVolume) {
    const auto detConstruction = static_cast<const DetectorConstruction*>(
      G4RunManager::GetRunManager()->GetUserDetectorConstruction());
    fScoringVolume = detConstruction->GetScoringVolume();
  }
  G4LogicalVolume* volume = step->GetPreStepPoint()->GetTouchableHandle()
      ->GetVolume()->GetLogicalVolume();
  if (volume != fScoringVolume) return;
  G4double edepStep = step->GetTotalEnergyDeposit();
  if (edepStep > 0) {
    G4cout << "Energy deposited in LXe: " << edepStep/keV << " keV by "
           << step->GetTrack()->GetParticleDefinition()->GetParticleName()
           << " at (" << step->GetPreStepPoint()->GetPosition().x()/mm << ","
           << step->GetPreStepPoint()->GetPosition().y()/mm << ","
           << step->GetPreStepPoint()->GetPosition().z()/mm << ") mm"
           << G4endl;
  }
  fEventAction->AddEdep(edepStep);
}
} // namespace Test