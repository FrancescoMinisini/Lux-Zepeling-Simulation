// Test/src/PMTSensitiveDetector.cc
#include "PMTSensitiveDetector.hh"
// #include "SimConfig.hh" // quando avrai il campo QE nel config, riattiva

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"
#include "G4TouchableHistory.hh"
#include "G4SystemOfUnits.hh"
#include "G4LogicalVolume.hh"

namespace Test {

PMTSensitiveDetector::PMTSensitiveDetector(const G4String& name)
: G4VSensitiveDetector(name)
{}

void PMTSensitiveDetector::Initialize(G4HCofThisEvent*)
{
  times_top_.clear();
  times_bot_.clear();
}

G4bool PMTSensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory*)
{
  auto* track = step->GetTrack();
  if (track->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition())
    return false;

  const auto* pre = step->GetPreStepPoint();
  const auto* lv  = pre->GetTouchableHandle()->GetVolume()->GetLogicalVolume();
  const auto& name = lv->GetName();

  // QE temporanea: fissata a 1 per vedere subito conteggi
  // Sostituisci con lettura da config quando disponibile (es. cfg.opt.pmt_qe)
  const double qe = 1.0;

  bool detected = true;
  if (qe < 1.0) {
    // Bernoulli semplice (se vorrai random veri, usa G4UniformRand)
    // Qui evitiamo dipendenze extra: deterministico ma sufficiente
    unsigned long mix = (unsigned long)(track->GetTrackID()*1469598103934665603ULL) ^
                        (unsigned long)(pre->GetGlobalTime()/ns*1099511628211ULL);
    double u = ((mix % 1000000) + 0.5) / 1000000.0;
    detected = (u < qe);
  }

  if (detected) {
    double t_ns = pre->GetGlobalTime()/ns;
    if (name == "TopPMT") {
      times_top_.push_back(t_ns);
    } else if (name == "BottomPMT") {
      times_bot_.push_back(t_ns);
    }
    // Uccidi il fotone quando entra nel PMT
    track->SetTrackStatus(fStopAndKill);
  }

  return true;
}

void PMTSensitiveDetector::EndOfEvent(G4HCofThisEvent*) { /* no-op */ }

void PMTSensitiveDetector::Clear()
{
  times_top_.clear();
  times_bot_.clear();
}

} // namespace Test
