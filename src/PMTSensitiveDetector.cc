#include "PMTSensitiveDetector.hh"
#include "SimConfig.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"
#include "G4TouchableHistory.hh"
#include "G4SystemOfUnits.hh"
#include "G4LogicalVolume.hh"
#include "G4VProcess.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include <fstream>

namespace Test {

PMTSensitiveDetector::PMTSensitiveDetector(const G4String& name)
: G4VSensitiveDetector(name)
{
  G4SDManager::GetSDMpointer()->SetVerboseLevel(1);
}

void PMTSensitiveDetector::Initialize(G4HCofThisEvent*)
{
  times_top_.clear();
  times_bot_.clear();
}

G4bool PMTSensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory*)
{
  auto* track = step->GetTrack();
  if (track->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition()) {
    return false;
  }
  const auto* pre = step->GetPreStepPoint();
  const auto* lv = pre->GetTouchableHandle()->GetVolume()->GetLogicalVolume();
  const auto& name = lv->GetName();

  G4double t_ns = pre->GetGlobalTime()/ns;
  G4double energy_eV = pre->GetKineticEnergy()/eV;
  G4String creator = track->GetCreatorProcess() ? track->GetCreatorProcess()->GetProcessName() : "none";

  G4cout << "Photon hit " << name << " at t=" << t_ns << " ns, energy=" 
         << energy_eV << " eV, creator process=" << creator 
         << ", position=(" << pre->GetPosition().x()/mm << "," 
         << pre->GetPosition().y()/mm << "," << pre->GetPosition().z()/mm << ") mm" 
         << G4endl;

  // Assume photons reaching PMT are detected (EFFICIENCY handled by surface)
  G4cout << "Photon DETECTED in " << name << " at t=" << t_ns << " ns" << G4endl;
  if (name == "TopPMT") {
    times_top_.push_back(t_ns);
  } else if (name == "BottomPMT") {
    times_bot_.push_back(t_ns);
  }
  track->SetTrackStatus(fStopAndKill);
  return true;
}

void PMTSensitiveDetector::EndOfEvent(G4HCofThisEvent*)
{
  // Get event ID
  G4int eventID = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();

  // Save photon times to CSV
  std::ofstream outFile("../outputs/photon_times.csv", std::ios::app);
  if (!outFile.is_open()) {
    G4cerr << "ERROR: Cannot open photon_times.csv" << G4endl;
    return;
  }
  if (!times_top_.empty()) {
    for (const auto& t : times_top_) {
      outFile << eventID << ",TopPMT," << t << "\n";
    }
    G4cout << "Top PMT times (ns): ";
    for (const auto& t : times_top_) G4cout << t << ", ";
    G4cout << G4endl;
  }
  if (!times_bot_.empty()) {
    for (const auto& t : times_bot_) {
      outFile << eventID << ",BottomPMT," << t << "\n";
    }
    G4cout << "Bottom PMT times (ns): ";
    for (const auto& t : times_bot_) G4cout << t << ", ";
    G4cout << G4endl;
  }
  outFile.close();
}

void PMTSensitiveDetector::Clear()
{
  times_top_.clear();
  times_bot_.clear();
}

} // namespace Test