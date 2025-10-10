#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "ActionInitialization.hh"
#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"

int main(int argc, char** argv) {
  G4UIExecutive* ui = (argc == 1) ? new G4UIExecutive(argc, argv) : nullptr;
  auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
  runManager->SetUserInitialization(new LZSim::DetectorConstruction());
  runManager->SetUserInitialization(new LZSim::PhysicsList());
  runManager->SetUserInitialization(new LZSim::ActionInitialization());
  auto* visManager = new G4VisExecutive();
  visManager->Initialize();
  auto* UImanager = G4UImanager::GetUIpointer();
  if (ui) {
    UImanager->ApplyCommand("/control/execute init_vis.mac");
    ui->SessionStart();
    delete ui;
  } else {
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command + fileName);
  }
  delete visManager;
  delete runManager;
  return 0;
}