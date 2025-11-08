// Source/astrochicken/GameStart/GameStartWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameStartFlow.h"
#include "GameStartWidget.generated.h"

class UGameStartFlow;
class URaceSelectionWidget;
class UTechPointAllocationWidget;
class UWidgetSwitcher;
class UTextBlock;
class UProgressBar;

/**
 * Main widget for the game start flow
 * Displays race selection, tech allocation, and initialization screens
 */
UCLASS()
class ASTROCHICKEN_API UGameStartWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// Start the flow
	UFUNCTION(BlueprintCallable, Category = "Game Start")
	void BeginGameStart();

protected:
	// Main widget switcher for phases
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* PhaseWidgetSwitcher;

	// Progress display
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PhaseNameText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PhaseProgressBar;

	// Phase 1: Race Selection (container for dynamic widget)
	UPROPERTY(meta = (BindWidget))
	class UNamedSlot* RaceSelectionSlot;

	// Phase 2: Tech Point Allocation (container for dynamic widget)
	UPROPERTY(meta = (BindWidget))
	class UNamedSlot* TechAllocationSlot;

	// Phase 3: Initialization screen
	UPROPERTY(meta = (BindWidget))
	UTextBlock* InitializationStatusText;

	// Widget classes to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Start")
	TSubclassOf<URaceSelectionWidget> RaceSelectionWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Start")
	TSubclassOf<UTechPointAllocationWidget> TechAllocationWidgetClass;

private:
	// Phase change callback
	UFUNCTION()
	void OnPhaseChanged(EGameStartPhase NewPhase);

	UFUNCTION()
	void OnGameStartComplete();

	// Widget management
	void ShowPhase(EGameStartPhase Phase);
	void UpdatePhaseDisplay(EGameStartPhase Phase);
	void CreateRaceSelectionWidget();
	void CreateTechAllocationWidget();

	// Callbacks from child widgets
	UFUNCTION()
	void OnRaceSelected(EPhylum SelectedPhylum, FString ColonyName);

	UFUNCTION()
	void OnTechAllocationComplete();

	UPROPERTY()
	UGameStartFlow* GameStartFlow;

	UPROPERTY()
	URaceSelectionWidget* RaceSelectionWidget;

	UPROPERTY()
	UTechPointAllocationWidget* TechAllocationWidget;
};
