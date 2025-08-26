// //
// /// \file Test/include/DetectorConstruction.hh
// /// \brief Definition of the Test::DetectorConstruction class
// #ifndef Test_DetectorConstruction_h
// #define Test_DetectorConstruction_h 1

// #include "G4VUserDetectorConstruction.hh"
// #include "globals.hh"

// class G4VPhysicalVolume;
// class G4LogicalVolume;

// namespace Test
// {

// class DetectorConstruction : public G4VUserDetectorConstruction
// {
//   public:
//     DetectorConstruction();
//     ~DetectorConstruction() override = default;

//     G4VPhysicalVolume* Construct() override;
//     void ConstructSDandField() override;

//     G4LogicalVolume* GetScoringVolume() const { return fScoringVolume; }

//   private:
//     G4LogicalVolume* fScoringVolume = nullptr;
// };

// }  // namespace Test

// #endif
