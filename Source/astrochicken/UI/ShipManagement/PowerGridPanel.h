// Source/astrochicken/UI/ShipManagement/PowerGridPanel.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../../Infrastructure/ElectricalGridData.h"
#include "PowerGridPanel.generated.h"

class UShipInfrastructureManager;
class UScrollBox;
class UTextBlock;
class UProgressBar;
class UButton;
class USlider;

/**
 * Power grid management panel
 * Shows reactors, power nodes, conduits, and allows control
 */
UCLASS()
class ASTROCHICKEN_API UPowerGridPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Power Grid")
	void RefreshDisplay();

protected:
	// Summary displays
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalGenerationText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalConsumptionText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PowerBalanceText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PowerBalanceBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GridEfficiencyText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* OfflineNodesText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* OverloadedConduitsText;

	// Reactor list
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ReactorListScrollBox;

	// Power node list
	UPROPERTY(meta = (BindWidget))
	UScrollBox* PowerNodeListScrollBox;

	// Widget classes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Grid")
	TSubclassOf<UUserWidget> ReactorWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Grid")
	TSubclassOf<UUserWidget> PowerNodeWidgetClass;

private:
	void UpdateSummary();
	void PopulateReactorList();
	void PopulatePowerNodeList();

	UPROPERTY()
	UShipInfrastructureManager* InfraManager;
};

/**
 * Individual reactor display widget
 */
UCLASS()
class ASTROCHICKEN_API UReactorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Reactor")
	void SetReactorData(const FReactor& InReactorData, const FString& InReactorID);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ReactorNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ReactorTypeText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PowerOutputBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PowerOutputText;

	UPROPERTY(meta = (BindWidget))
	USlider* OutputSlider;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* FuelBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* FuelText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* TemperatureBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TemperatureText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatusText;

private:
	UFUNCTION()
	void OnOutputSliderChanged(float Value);

	FString ReactorID;
	FReactor ReactorData;
};

/**
 * Individual power node display widget
 */
UCLASS()
class ASTROCHICKEN_API UPowerNodeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Power Node")
	void SetNodeData(const FPowerNode& InNodeData);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NodeNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NodeTypeText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NodeStatusText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PowerFlowText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CompartmentText;

private:
	FPowerNode NodeData;
};
