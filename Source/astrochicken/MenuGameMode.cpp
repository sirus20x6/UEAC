// MyProject/Private/GameModes/MenuGameMode.cpp

#include "GameModes/MenuGameMode.h"
#include "Player/MenuPlayerController.h" // Include our controller header

AMenuGameMode::AMenuGameMode()
{
    // Set the default player controller to our custom C++ class
    PlayerControllerClass = AMenuPlayerController::StaticClass();
}