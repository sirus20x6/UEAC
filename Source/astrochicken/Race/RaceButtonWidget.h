// Source/astrochicken/Race/RaceButtonWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RaceData.h"
#include "RaceButtonWidget.generated.h"

class UButton;
class UBorder;
class UImage;
class UTextBlock;

/**
 * Individual race button widget for selection list
 * Displays race icon, name, and brief info
 */
UCLASS()
class ASTROCHICKEN_API URaceButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// Set the race data to display
	UFUNCTION(BlueprintCallable, Category = "Race Button")
	void SetRaceData(const FRaceData& InRaceData);

	// Set visual selection state
	UFUNCTION(BlueprintCallable, Category = "Race Button")
	void SetSelected(bool bSelected);

	// Get the phylum this button represents
	UFUNCTION(BlueprintCallable, Category = "Race Button")
	EPhylum GetPhylum() const { return RaceData.Phylum; }

	// Delegate for when this button is clicked
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceButtonClicked, EPhylum, Phylum);

	UPROPERTY(BlueprintAssignable, Category = "Race Button")
	FOnRaceButtonClicked OnRaceClicked;

protected:
	// Widget references (bind in Blueprint)
	UPROPERTY(meta = (BindWidget))
	UButton* RaceButton;

	UPROPERTY(meta = (BindWidget))
	UBorder* RaceBorder;

	UPROPERTY(meta = (BindWidget))
	UImage* RaceImage;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RaceNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RacePhylumText;

private:
	UFUNCTION()
	void OnButtonClicked();

	UPROPERTY()
	FRaceData RaceData;

	bool bIsSelected = false;
};
