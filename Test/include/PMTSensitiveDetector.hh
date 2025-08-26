//
/// \file T/include/PMTSensitiveDetector.hh
#ifndef Test_PMTSensitiveDetector_h
#define Test_PMTSensitiveDetector_h 1

#include "G4VSensitiveDetector.hh"

namespace Test {

class PMTSensitiveDetector : public G4VSensitiveDetector {
public:
  explicit PMTSensitiveDetector(const G4String& name);
  ~PMTSensitiveDetector() override = default;

  G4bool ProcessHits(G4Step*, G4TouchableHistory*) override;
  void EndOfEvent(G4HCofThisEvent*) override;

private:
  G4int fTop = 0;
  G4int fBot = 0;
};

} // namespace Test
#endif
