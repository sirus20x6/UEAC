// Source/astrochicken/Race/RaceSelectionWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RaceData.h"
#include "RaceSelectionWidget.generated.h"

class URaceManager;
class URaceButtonWidget;
class UVerticalBox;
class UTextBlock;
class UEditableText;
class UButton;
class UScrollBox;
class UImage;

/**
 * Main race selection screen
 * Displays available races and allows player to choose and name their colony
 */
UCLASS()
class ASTROCHICKEN_API URaceSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// Populate the race list
	UFUNCTION(BlueprintCallable, Category = "Race Selection")
	void PopulateRaceList();

	// Delegates for external communication
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaceSelected, EPhylum, SelectedPhylum, FString, ColonyName);

	UPROPERTY(BlueprintAssignable, Category = "Race Selection")
	FOnRaceSelected OnRaceConfirmed;

protected:
	// Widget references (bind these in Blueprint)
	UPROPERTY(meta = (BindWidget))
	UScrollBox* RaceListScrollBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RaceNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RaceDescriptionText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RaceLoreText;

	UPROPERTY(meta = (BindWidget))
	UImage* RaceIconImage;

	UPROPERTY(meta = (BindWidget))
	UEditableText* ColonyNameInput;

	UPROPERTY(meta = (BindWidget))
	UButton* ConfirmButton;

	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

	// Attribute display
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AdaptabilityText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ResilienceText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* IntelligenceText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SocialText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* EfficiencyText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SpaceAdaptationText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ResourceExtractionText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TechAffinityText;

	// Tech bonus display
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TechBonusesText;

	// Blueprint-assignable widget class for race buttons
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Selection")
	TSubclassOf<URaceButtonWidget> RaceButtonWidgetClass;

private:
	UFUNCTION()
	void OnConfirmButtonClicked();

	UFUNCTION()
	void OnBackButtonClicked();

	void OnRaceButtonClicked(EPhylum Phylum);
	void DisplayRaceDetails(const FRaceData& RaceData);
	void UpdateConfirmButtonState();

	UPROPERTY()
	URaceManager* RaceManager;

	UPROPERTY()
	TArray<URaceButtonWidget*> RaceButtons;

	EPhylum CurrentlySelectedPhylum = EPhylum::Chordata;
	bool bHasSelectedRace = false;
};
