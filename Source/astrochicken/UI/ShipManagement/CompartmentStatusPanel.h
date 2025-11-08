// Source/astrochicken/UI/ShipManagement/CompartmentStatusPanel.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../../Infrastructure/ShipStructureData.h"
#include "CompartmentStatusPanel.generated.h"

class UShipInfrastructureManager;
class UScrollBox;
class UTextBlock;
class UProgressBar;

/**
 * Compartment status display panel
 * Shows all compartments, their integrity, atmosphere, and crew
 */
UCLASS()
class ASTROCHICKEN_API UCompartmentStatusPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Compartments")
	void RefreshDisplay();

protected:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* CompartmentListScrollBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalCompartmentsText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DamagedCompartmentsText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DepressurizedCompartmentsText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compartments")
	TSubclassOf<UUserWidget> CompartmentWidgetClass;

private:
	void UpdateSummary();
	void PopulateCompartmentList();

	UPROPERTY()
	UShipInfrastructureManager* InfraManager;
};

/**
 * Individual compartment widget
 */
UCLASS()
class ASTROCHICKEN_API UCompartmentWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Compartment")
	void SetCompartmentData(const FShipCompartment& InCompartmentData);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CompartmentNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CompartmentTypeText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatusText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* IntegrityBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* IntegrityText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* OxygenBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* OxygenText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PressureBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TemperatureText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CrewCountText;

private:
	FShipCompartment CompartmentData;
};
