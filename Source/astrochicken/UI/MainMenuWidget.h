// /thearray/git/UnrealEngine/astrochicken/Source/astrochicken/UI/MainMenuWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UDynamicSettingsWidget;
class UWidgetSwitcher;
class UVerticalBox;

// CONFIRM THIS LINE IS HERE
UCLASS() 
class ASTROCHICKEN_API UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

private:
    // Main menu buttons
    UPROPERTY(meta = (BindWidget))
    UButton* StartGameButton;

    UPROPERTY(meta = (BindWidget))
    UButton* SettingsButton;

    UPROPERTY(meta = (BindWidget))
    UButton* QuitButton;
    
    // Widget switcher to handle menu navigation
    UPROPERTY(meta = (BindWidget))
    UWidgetSwitcher* MenuSwitcher;
    
    // Settings container - just a VerticalBox that we'll populate dynamically
    UPROPERTY(meta = (BindWidget))
    UVerticalBox* SettingsContainer;
    
    // Settings widget reference - created dynamically, not bound
    UPROPERTY()
    UDynamicSettingsWidget* SettingsWidgetRef;
    
    // Button event handlers
    UFUNCTION()
    void OnStartGameClicked();

    UFUNCTION()
    void OnSettingsClicked();

    UFUNCTION()
    void OnQuitClicked();
    
    // Settings navigation
    UFUNCTION()
    void OnBackToMainMenu();
    
    // Menu state management
    void ShowMainMenu();
    void ShowSettingsMenu();
};
