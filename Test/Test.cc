//
/// \file B1/Test.cc
/// \brief Main program of the B1 application

#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "ActionInitialization.hh"

#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"

int main(int argc, char** argv)
{
  // Detect interactive mode (if no arguments) and define UI session
  G4UIExecutive* ui = ( argc == 1 ) ? new G4UIExecutive(argc, argv) : nullptr;

  auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);

  // Set mandatory initialization classes
  runManager->SetUserInitialization(new Test::DetectorConstruction());
  runManager->SetUserInitialization(new Test::PhysicsList());
  runManager->SetUserInitialization(new Test::ActionInitialization());

  // Initialize visualization
  auto* visManager = new G4VisExecutive();
  visManager->Initialize();

  // Get the pointer to the UI manager and set verbosities
  auto* UImanager = G4UImanager::GetUIpointer();

  if (ui) {
    // interactive session
    UImanager->ApplyCommand("/control/execute init_vis.mac");
    ui->SessionStart();
    delete ui;
  } else {
    // batch mode
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command+fileName);
  }

  delete visManager;
  delete runManager;
  return 0;
}
