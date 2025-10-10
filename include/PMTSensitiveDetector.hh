#ifndef LZSIM_PMTSENSITIVEDETECTOR_HH
#define LZSIM_PMTSENSITIVEDETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include <vector>

class G4Step;
class G4HCofThisEvent;

namespace LZSim {
class PMTSensitiveDetector : public G4VSensitiveDetector {
public:
  PMTSensitiveDetector(const G4String& name);
  void Initialize(G4HCofThisEvent*) override;
  G4bool ProcessHits(G4Step*, G4TouchableHistory*) override;
  void EndOfEvent(G4HCofThisEvent*) override;
  void Clear();
  G4int GetNTop() const { return times_top_.size(); }
  const std::vector<G4double>& TimesTop() const { return times_top_; }

private:
  std::vector<G4double> times_top_;
};
} // namespace LZSim

#endif