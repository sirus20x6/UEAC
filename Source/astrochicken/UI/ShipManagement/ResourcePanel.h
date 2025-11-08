// Source/astrochicken/UI/ShipManagement/ResourcePanel.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResourcePanel.generated.h"

class UShipInfrastructureManager;
class UTextBlock;
class UProgressBar;

UCLASS()
class ASTROCHICKEN_API UResourcePanel : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	UFUNCTION(BlueprintCallable, Category = "Resources")
	void RefreshDisplay();

protected:
	// Life Support
	UPROPERTY(meta = (BindWidget))
	UTextBlock* LifeSupportStatusText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* OxygenProductionBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CrewSupportText;

	// Coolant System
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CoolantStatusText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* CoolantLevelBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CoolantLevelText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HeatLoadBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HeatLoadText;

	// Leaks
	UPROPERTY(meta = (BindWidget))
	UTextBlock* LeakingPipesText;

private:
	UPROPERTY()
	UShipInfrastructureManager* InfraManager;
};
