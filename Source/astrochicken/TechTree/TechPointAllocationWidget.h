// Source/astrochicken/TechTree/TechPointAllocationWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TechData.h"
#include "TechPointAllocationWidget.generated.h"

class UTechTreeManager;
class UVerticalBox;
class UTextBlock;
class UButton;
class USlider;
class UProgressBar;

/**
 * Widget for allocating bonus tech points during game start
 * Allows players to distribute extra research points across tech categories
 */
UCLASS()
class ASTROCHICKEN_API UTechPointAllocationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// Initialize with available bonus points
	UFUNCTION(BlueprintCallable, Category = "Tech Allocation")
	void Initialize(int32 BonusPoints);

	// Show current tech point distribution
	UFUNCTION(BlueprintCallable, Category = "Tech Allocation")
	void RefreshDisplay();

	// Delegate for completion
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllocationComplete);

	UPROPERTY(BlueprintAssignable, Category = "Tech Allocation")
	FOnAllocationComplete OnComplete;

protected:
	// Main container
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* AllocationContainer;

	// Header info
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TitleText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* InstructionsText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RemainingPointsText;

	// Category sliders/controls (6 categories)
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PhysicsPointsText;

	UPROPERTY(meta = (BindWidget))
	UButton* PhysicsIncrementButton;

	UPROPERTY(meta = (BindWidget))
	UButton* PhysicsDecrementButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* EngineeringPointsText;

	UPROPERTY(meta = (BindWidget))
	UButton* EngineeringIncrementButton;

	UPROPERTY(meta = (BindWidget))
	UButton* EngineeringDecrementButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* EnergyPointsText;

	UPROPERTY(meta = (BindWidget))
	UButton* EnergyIncrementButton;

	UPROPERTY(meta = (BindWidget))
	UButton* EnergyDecrementButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ComputingPointsText;

	UPROPERTY(meta = (BindWidget))
	UButton* ComputingIncrementButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ComputingDecrementButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BiologyPointsText;

	UPROPERTY(meta = (BindWidget))
	UButton* BiologyIncrementButton;

	UPROPERTY(meta = (BindWidget))
	UButton* BiologyDecrementButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SensorsPointsText;

	UPROPERTY(meta = (BindWidget))
	UButton* SensorsIncrementButton;

	UPROPERTY(meta = (BindWidget))
	UButton* SensorsDecrementButton;

	// Action buttons
	UPROPERTY(meta = (BindWidget))
	UButton* ConfirmButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ResetButton;

	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

private:
	// Button callbacks
	UFUNCTION()
	void OnPhysicsIncrement();
	UFUNCTION()
	void OnPhysicsDecrement();
	UFUNCTION()
	void OnEngineeringIncrement();
	UFUNCTION()
	void OnEngineeringDecrement();
	UFUNCTION()
	void OnEnergyIncrement();
	UFUNCTION()
	void OnEnergyDecrement();
	UFUNCTION()
	void OnComputingIncrement();
	UFUNCTION()
	void OnComputingDecrement();
	UFUNCTION()
	void OnBiologyIncrement();
	UFUNCTION()
	void OnBiologyDecrement();
	UFUNCTION()
	void OnSensorsIncrement();
	UFUNCTION()
	void OnSensorsDecrement();

	UFUNCTION()
	void OnConfirmClicked();

	UFUNCTION()
	void OnResetClicked();

	UFUNCTION()
	void OnBackClicked();

	// Helpers
	void ModifyPoints(ETechCategory Category, int32 Delta);
	void UpdateCategoryDisplay(ETechCategory Category);
	void UpdateAllDisplays();
	void UpdateConfirmButtonState();

	UPROPERTY()
	UTechTreeManager* TechTreeManager;

	// Track bonus allocation separately
	UPROPERTY()
	TMap<ETechCategory, int32> BonusAllocation;

	int32 TotalBonusPoints = 0;
	int32 AllocatedBonusPoints = 0;
};
