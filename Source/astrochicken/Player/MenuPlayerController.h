// MyProject/Public/Player/MenuPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MenuPlayerController.generated.h"

class UMainMenuWidget;

// ADD THIS LINE
UCLASS() 
class ASTROCHICKEN_API AMenuPlayerController : public APlayerController
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> MainMenuWidgetClass; 

    UPROPERTY()
    UUserWidget* MainMenuWidgetInstance;
};