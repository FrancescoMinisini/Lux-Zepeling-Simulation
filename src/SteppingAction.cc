#include "SteppingAction.hh"
#include "EventAction.hh"
#include "DetectorConstruction.hh"
#include "G4Step.hh"
#include "G4RunManager.hh"
#include "G4LogicalVolume.hh"
#include "SimConfig.hh"
#include "G4OpticalPhoton.hh"
#include "G4Poisson.hh"
#include "G4RandomDirection.hh"
#include "G4Electron.hh"
#include "Randomize.hh"

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
  // if (edepStep > 0) {
  //   G4cout << "Energy deposited in GXe: " << edepStep / keV << " keV by "
  //          << step->GetTrack()->GetParticleDefinition()->GetParticleName()
  //          << " at (" << step->GetPreStepPoint()->GetPosition().x() / mm << ","
  //          << step->GetPreStepPoint()->GetPosition().y() / mm << ","
  //          << step->GetPreStepPoint()->GetPosition().z() / mm << ") mm" << G4endl;
  // }
  fEventAction->AddEdep(edepStep);

  auto& cfg = SimConfig::Get();
  auto& el = cfg.el;

  // Solo per e- in GXe (scoringVolume = GXe)
  auto* track = step->GetTrack();
  if (track->GetParticleDefinition() == G4Electron::ElectronDefinition() && 
      step->GetTotalEnergyDeposit() > 0.) {  // Solo step con depo (scattering)
  
    G4double step_length_mm = step->GetStepLength() / mm;
    G4int n_photons = G4Poisson(el.el_gain_per_mm * step_length_mm);  // Poisson per variabilità
  
    if (n_photons > 0) {
      G4double step_time_ns = step->GetDeltaTime() / ns;  // Tempo step base
    
      // Shift temporale da drift (dist / v_drift)
      G4double drift_delay_ns = step_length_mm / el.v_drift_mm_per_ns;
    
      // Broadening gaussiano da diffusione longitudinale
      G4double dist_cm = step->GetStepLength() / cm;
      G4double sigma_time_ns = std::sqrt(dist_cm / el.diffusion_const_cm2_per_s) * 1e9;  // Converti s a ns (approx, adjust factor)
    
      for (G4int i = 0; i < n_photons; ++i) {
        // Crea fotone ottico
        G4ParticleDefinition* photon_def = G4OpticalPhoton::OpticalPhotonDefinition();
        G4double photon_energy = G4RandFlat::shoot(cfg.opt.eV_min, cfg.opt.eV_max) * eV;  // Energia VUV
      
        // Posizione: Posizione media step
        G4ThreeVector pos = 0.5 * (step->GetPreStepPoint()->GetPosition() + step->GetPostStepPoint()->GetPosition());
      
        // Tempo: Tempo step + delay + gaussian broadening
        G4double phot_time_ns = step_time_ns + drift_delay_ns + G4RandGauss::shoot(0., sigma_time_ns);
      
        // Direzione isotropa (come EL)
        G4ThreeVector dir = G4RandomDirection();
      
        // Genera track fotone (usa G4Track per secondaries)
        G4Track* phot_track = new G4Track(new G4DynamicParticle(photon_def, dir, photon_energy), phot_time_ns * ns, pos);
        phot_track->SetParentID(track->GetTrackID());
        phot_track->SetTrackStatus(fAlive);
      
        // Aggiungi a stack (per processamento)
        G4EventManager::GetEventManager()->GetStackManager()->PushOneTrack(phot_track);
      }
    }
  }
}
} // namespace LZSim