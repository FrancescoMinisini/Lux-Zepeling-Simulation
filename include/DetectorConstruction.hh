#ifndef LZSIM_DETECTORCONSTRUCTION_HH
#define LZSIM_DETECTORCONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;

namespace LZSim {
class DetectorConstruction : public G4VUserDetectorConstruction {
public:
  G4VPhysicalVolume* Construct() override;
  void ConstructSDandField() override;
  G4LogicalVolume* GetScoringVolume() const { return fScoringVolume; }

protected:
  G4LogicalVolume* fScoringVolume = nullptr;
};
} // namespace LZSim

#endif