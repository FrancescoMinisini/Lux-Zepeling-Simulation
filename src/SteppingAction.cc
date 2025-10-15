#include "SteppingAction.hh"
#include "EventAction.hh"
#include "DetectorConstruction.hh"
#include "G4Step.hh"
#include "G4RunManager.hh"
#include "G4LogicalVolume.hh"
#include "SimConfig.hh"
#include "G4OpticalPhoton.hh"

namespace LZSim {
SteppingAction::SteppingAction(EventAction* eventAction) : fEventAction(eventAction) {}

void SteppingAction::UserSteppingAction(const G4Step* step) {
  if (!fScoringVolume) {
    const auto detConstruction = static_cast<const DetectorConstruction*>(
      G4RunManager::GetRunManager()->GetUserDetectorConstruction());
    fScoringVolume = detConstruction->GetScoringVolume();
  }
  G4LogicalVolume* volume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume();
  if (volume != fScoringVolume) return;
  G4double edepStep = step->GetTotalEnergyDeposit();
  if (edepStep > 0) {
    G4cout << "Energy deposited in GXe: " << edepStep / keV << " keV by "
           << step->GetTrack()->GetParticleDefinition()->GetParticleName()
           << " at (" << step->GetPreStepPoint()->GetPosition().x() / mm << ","
           << step->GetPreStepPoint()->GetPosition().y() / mm << ","
           << step->GetPreStepPoint()->GetPosition().z() / mm << ") mm" << G4endl;
  }
  fEventAction->AddEdep(edepStep);

  // New: Log photon positions if opticalphoton
  if (step->GetTrack()->GetParticleDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
    G4ThreeVector pos = step->GetPreStepPoint()->GetPosition();
    G4cout << "Photon position: (" << pos.x() / mm << ", " << pos.y() / mm << ", " << pos.z() / mm << ") mm" << G4endl;
  }
}
}