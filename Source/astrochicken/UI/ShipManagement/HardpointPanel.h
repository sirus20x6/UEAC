// Source/astrochicken/UI/ShipManagement/HardpointPanel.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../../Infrastructure/HardpointData.h"
#include "HardpointPanel.generated.h"

class UShipInfrastructureManager;
class UScrollBox;
class UTextBlock;

UCLASS()
class ASTROCHICKEN_API UHardpointPanel : public UserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	UFUNCTION(BlueprintCallable, Category = "Hardpoints")
	void RefreshDisplay();

protected:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* HardpointListScrollBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalHardpointsText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* OccupiedHardpointsText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoints")
	TSubclassOf<UUserWidget> HardpointWidgetClass;

private:
	UPROPERTY()
	UShipInfrastructureManager* InfraManager;
};

UCLASS()
class ASTROCHICKEN_API UHardpointItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Hardpoint")
	void SetHardpointData(const FHardpoint& InHardpointData);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* HardpointNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatusText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* EquipmentText;

private:
	FHardpoint HardpointData;
};
