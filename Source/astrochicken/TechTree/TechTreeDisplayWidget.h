// Source/astrochicken/TechTree/TechTreeDisplayWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TechData.h"
#include "TechTreeDisplayWidget.generated.h"

class UTechTreeManager;
class UScrollBox;
class UVerticalBox;
class UTextBlock;
class UButton;
class UImage;
class UTechNodeWidget;

/**
 * Displays the tech tree and allows browsing/researching technologies
 */
UCLASS()
class ASTROCHICKEN_API UTechTreeDisplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// Populate the tech tree display
	UFUNCTION(BlueprintCallable, Category = "Tech Tree Display")
	void PopulateTechTree();

	// Filter by category
	UFUNCTION(BlueprintCallable, Category = "Tech Tree Display")
	void FilterByCategory(ETechCategory Category);

	// Show all categories
	UFUNCTION(BlueprintCallable, Category = "Tech Tree Display")
	void ShowAllCategories();

	// Refresh the display
	UFUNCTION(BlueprintCallable, Category = "Tech Tree Display")
	void RefreshDisplay();

protected:
	// Main containers
	UPROPERTY(meta = (BindWidget))
	UScrollBox* TechListScrollBox;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* CategoryButtonsContainer;

	// Category point display
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PhysicsPointsText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* EngineeringPointsText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* EnergyPointsText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ComputingPointsText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BiologyPointsText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SensorsPointsText;

	// Category filter buttons
	UPROPERTY(meta = (BindWidget))
	UButton* FilterPhysicsButton;

	UPROPERTY(meta = (BindWidget))
	UButton* FilterEngineeringButton;

	UPROPERTY(meta = (BindWidget))
	UButton* FilterEnergyButton;

	UPROPERTY(meta = (BindWidget))
	UButton* FilterComputingButton;

	UPROPERTY(meta = (BindWidget))
	UButton* FilterBiologyButton;

	UPROPERTY(meta = (BindWidget))
	UButton* FilterSensorsButton;

	UPROPERTY(meta = (BindWidget))
	UButton* FilterAllButton;

	// Detail panel
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SelectedTechNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SelectedTechDescriptionText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SelectedTechEffectText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SelectedTechPrereqText;

	UPROPERTY(meta = (BindWidget))
	UButton* ResearchButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CloseButton;

	// Blueprint-assignable widget class for tech nodes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Tree Display")
	TSubclassOf<UTechNodeWidget> TechNodeWidgetClass;

private:
	UFUNCTION()
	void OnFilterPhysics();
	UFUNCTION()
	void OnFilterEngineering();
	UFUNCTION()
	void OnFilterEnergy();
	UFUNCTION()
	void OnFilterComputing();
	UFUNCTION()
	void OnFilterBiology();
	UFUNCTION()
	void OnFilterSensors();
	UFUNCTION()
	void OnFilterAll();
	UFUNCTION()
	void OnResearchClicked();
	UFUNCTION()
	void OnCloseClicked();

	void OnTechNodeClicked(const FString& TechID);
	void DisplayTechDetails(const FTechNode& Tech);
	void UpdateCategoryPoints();
	void PopulateTechsByCategory(ETechCategory Category);
	void PopulateAllTechs();

	UPROPERTY()
	UTechTreeManager* TechTreeManager;

	UPROPERTY()
	TArray<UTechNodeWidget*> TechNodeWidgets;

	FString SelectedTechID;
	ETechCategory CurrentFilter = ETechCategory::Physics;
	bool bShowingAllCategories = true;
};

/**
 * Individual tech node display widget
 */
UCLASS()
class ASTROCHICKEN_API UTechNodeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Tech Node")
	void SetTechData(const FTechNode& InTechData);

	UFUNCTION(BlueprintCallable, Category = "Tech Node")
	FString GetTechID() const { return TechData.TechID; }

	// Delegate for when clicked
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTechNodeClicked, FString, TechID);

	UPROPERTY(BlueprintAssignable, Category = "Tech Node")
	FOnTechNodeClicked OnTechClicked;

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* TechButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TechNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TechTierText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TechCategoryText;

	UPROPERTY(meta = (BindWidget))
	UImage* StatusIcon;

private:
	UFUNCTION()
	void OnButtonClicked();

	void UpdateVisualState();

	UPROPERTY()
	FTechNode TechData;
};
