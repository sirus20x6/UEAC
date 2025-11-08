// MyProject/Private/Player/MenuPlayerController.cpp

#include "Player/MenuPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UI/MainMenuWidget.h" // Include our widget header

void AMenuPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Check if the MainMenuWidgetClass has been set in the editor
    if (MainMenuWidgetClass)
    {
        // Create the widget
        MainMenuWidgetInstance = CreateWidget<UMainMenuWidget>(this, MainMenuWidgetClass);

        if (MainMenuWidgetInstance)
        {
            // Add it to the viewport
            MainMenuWidgetInstance->AddToViewport();

            // Set up input mode for UI only
            FInputModeUIOnly InputModeData;
            InputModeData.SetWidgetToFocus(MainMenuWidgetInstance->TakeWidget());
            InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            
            SetInputMode(InputModeData);

            // Show the mouse cursor
            bShowMouseCursor = true;
        }
    }
}