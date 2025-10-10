//
/// \file Test/src/ActionInitialization.cc
/// \brief Implementation of the Test::ActionInitialization class

#include "ActionInitialization.hh"

#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"

// Geant4
#include "G4RunManager.hh"

// Geant4 analysis (serve per registrare i comandi /analysis/* fin dall'avvio)
#include "G4AnalysisManager.hh"

namespace Test
{

void ActionInitialization::BuildForMaster() const
{
  // Istanzia l'AnalysisManager il prima possibile (registrazione dei comandi /analysis/*)
  (void) G4AnalysisManager::Instance();

  // Solo azioni di run lato master
  SetUserAction(new RunAction());
}

void ActionInitialization::Build() const
{
  // Istanzia l'AnalysisManager anche sui worker prima di eseguire macro /analysis/*
  (void) G4AnalysisManager::Instance();

  // Primary generator
  SetUserAction(new PrimaryGeneratorAction());

  // Run / Event / Step actions
  auto runAction   = new RunAction();
  SetUserAction(runAction);

  auto eventAction = new EventAction(runAction);
  SetUserAction(eventAction);

  SetUserAction(new SteppingAction(eventAction));
}

} // namespace Test
