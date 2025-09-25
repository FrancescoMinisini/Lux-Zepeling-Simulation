//
/// \file Test/src/RunAction.cc
/// \brief Implementation of the B1::RunAction class

#include "RunAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"

#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4AccumulableManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"

// Analysis
#include "G4AnalysisManager.hh"

namespace Test
{

RunAction::RunAction()
{
  const G4double milligray = 1.e-3*gray;
  const G4double microgray = 1.e-6*gray;
  const G4double nanogray  = 1.e-9*gray;
  const G4double picogray  = 1.e-12*gray;

  new G4UnitDefinition("milligray", "milliGy" , "Dose", milligray);
  new G4UnitDefinition("microgray", "microGy" , "Dose", microgray);
  new G4UnitDefinition("nanogray" , "nanoGy"  , "Dose", nanogray);
  new G4UnitDefinition("picogray" , "picoGy"  , "Dose", picogray);

  auto* am = G4AccumulableManager::Instance();
  am->RegisterAccumulable(fEdep);
  am->RegisterAccumulable(fEdep2);
}

void RunAction::BeginOfRunAction(const G4Run*)
{
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);

  G4AccumulableManager::Instance()->Reset();

  // --- Analysis manager setup ---
  auto* ana = G4AnalysisManager::Instance();

  if (!ana->IsOpenFile()) {
    ana->OpenFile(); // usa nome e tipo impostati dalle macro
  }

  // Crea l’ntuple (id=0)
  ana->CreateNtuple("single", "single");
  ana->CreateNtupleIColumn("event_id");       // 0
  ana->CreateNtupleIColumn("nPhotTop");       // 1
  ana->CreateNtupleIColumn("nPhotBot");       // 2
  ana->CreateNtupleDColumn("Edep_LXe");       // 3 [MeV]
  ana->CreateNtupleDColumn("t_first_top_ns"); // 4
  ana->CreateNtupleDColumn("t_first_bot_ns"); // 5
  ana->CreateNtupleDColumn("t_mean_top_ns");  // 6
  ana->CreateNtupleDColumn("t_mean_bot_ns");  // 7
  ana->FinishNtuple(0);
}

void RunAction::EndOfRunAction(const G4Run* run)
{
  G4int nofEvents = run->GetNumberOfEvent();
  if (nofEvents == 0) return;

  G4AccumulableManager::Instance()->Merge();

  G4double edep  = fEdep.GetValue();
  G4double edep2 = fEdep2.GetValue();

  G4double rms = edep2 - edep*edep/nofEvents;
  if (rms > 0.) rms = std::sqrt(rms); else rms = 0.;

  const auto detConstruction = static_cast<const DetectorConstruction*>(
    G4RunManager::GetRunManager()->GetUserDetectorConstruction());
  G4double mass = detConstruction->GetScoringVolume()->GetMass();
  G4double dose = edep/mass;
  G4double rmsDose = rms/mass;

  const auto generatorAction = static_cast<const PrimaryGeneratorAction*>(
    G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction());
  G4String runCondition;
  if (generatorAction)
  {
    const G4ParticleGun* particleGun = generatorAction->GetParticleGun();
    runCondition += particleGun->GetParticleDefinition()->GetParticleName();
    runCondition += " of ";
    G4double particleEnergy = particleGun->GetParticleEnergy();
    runCondition += G4BestUnit(particleEnergy,"Energy");
  }

  if (IsMaster()) {
    G4cout << G4endl << "--------------------End of Global Run-----------------------";
  } else {
    G4cout << G4endl << "--------------------End of Local Run------------------------";
  }

  G4cout
     << G4endl
     << " The run consists of " << nofEvents << " "<< runCondition
     << G4endl
     << " Cumulated dose per run, in scoring volume : "
     << G4BestUnit(dose,"Dose") << " rms = " << G4BestUnit(rmsDose,"Dose")
     << G4endl
     << "------------------------------------------------------------"
     << G4endl << G4endl;

  auto* ana = G4AnalysisManager::Instance();
  if (ana->IsOpenFile()) {
    ana->Write();
    ana->CloseFile(true);
  }
}

void RunAction::AddEdep(G4double edep)
{
  fEdep  += edep;
  fEdep2 += edep*edep;
}

} // namespace Test
