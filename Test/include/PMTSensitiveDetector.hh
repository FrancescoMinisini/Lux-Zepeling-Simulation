#ifndef Test_PMTSensitiveDetector_h
#define Test_PMTSensitiveDetector_h 1

#include "G4VSensitiveDetector.hh"
#include <vector>

namespace Test {

class PMTSensitiveDetector : public G4VSensitiveDetector {
public:
  explicit PMTSensitiveDetector(const G4String& name);
  ~PMTSensitiveDetector() override = default;

  G4bool ProcessHits(G4Step*, G4TouchableHistory*) override;
  void EndOfEvent(G4HCofThisEvent*) override;

  // Estensioni per feature
  inline G4int GetNTop() const { return fTop; }
  inline G4int GetNBot() const { return fBot; }
  inline const std::vector<G4double>& TimesTop() const { return fTimesTop; }
  inline const std::vector<G4double>& TimesBot() const { return fTimesBot; }
  void Clear();

private:
  G4int fTop = 0;
  G4int fBot = 0;
  std::vector<G4double> fTimesTop;
  std::vector<G4double> fTimesBot;
};

} // namespace Test
#endif
