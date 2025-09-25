// Test/include/PMTSensitiveDetector.hh
#pragma once
#include "G4VSensitiveDetector.hh"
#include <vector>

class G4Step;

namespace Test {

class PMTSensitiveDetector : public G4VSensitiveDetector {
public:
  PMTSensitiveDetector(const G4String& name);
  ~PMTSensitiveDetector() override = default;

  void Initialize(G4HCofThisEvent*) override;
  G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override;
  void EndOfEvent(G4HCofThisEvent*) override;

  // Accessors per EventAction
  inline int GetNTop() const { return (int)times_top_.size(); }
  inline int GetNBot() const { return (int)times_bot_.size(); }
  inline const std::vector<double>& TimesTop() const { return times_top_; }
  inline const std::vector<double>& TimesBot() const { return times_bot_; }

  void Clear(); // pulizia manuale a fine evento (ridondante, ma comoda)

private:
  std::vector<double> times_top_; // tempi globali in ns
  std::vector<double> times_bot_;
};

} // namespace Test
