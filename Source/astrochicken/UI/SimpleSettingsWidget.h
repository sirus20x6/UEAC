// /thearray/git/UnrealEngine/astrochicken/Source/astrochicken/UI/SimpleSettingsWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SimpleSettingsWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;

UCLASS()
class ASTROCHICKEN_API USimpleSettingsWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

private:
    // Event handlers
    UFUNCTION()
    void OnBackButtonClicked();

public:
    // Delegate for back navigation
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackToMainMenu);
    UPROPERTY(BlueprintAssignable)
    FOnBackToMainMenu OnBackToMainMenu;
};
