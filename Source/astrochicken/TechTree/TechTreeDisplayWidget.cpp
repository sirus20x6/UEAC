// Source/astrochicken/TechTree/TechTreeDisplayWidget.cpp

#include "TechTreeDisplayWidget.h"
#include "TechTreeManager.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"

// ===== UTechTreeDisplayWidget =====

void UTechTreeDisplayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get tech tree manager
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		TechTreeManager = GameInstance->GetSubsystem<UTechTreeManager>();
	}

	// Bind filter buttons
	if (FilterPhysicsButton)
		FilterPhysicsButton->OnClicked.AddDynamic(this, &UTechTreeDisplayWidget::OnFilterPhysics);
	if (FilterEngineeringButton)
		FilterEngineeringButton->OnClicked.AddDynamic(this, &UTechTreeDisplayWidget::OnFilterEngineering);
	if (FilterEnergyButton)
		FilterEnergyButton->OnClicked.AddDynamic(this, &UTechTreeDisplayWidget::OnFilterEnergy);
	if (FilterComputingButton)
		FilterComputingButton->OnClicked.AddDynamic(this, &UTechTreeDisplayWidget::OnFilterComputing);
	if (FilterBiologyButton)
		FilterBiologyButton->OnClicked.AddDynamic(this, &UTechTreeDisplayWidget::OnFilterBiology);
	if (FilterSensorsButton)
		FilterSensorsButton->OnClicked.AddDynamic(this, &UTechTreeDisplayWidget::OnFilterSensors);
	if (FilterAllButton)
		FilterAllButton->OnClicked.AddDynamic(this, &UTechTreeDisplayWidget::OnFilterAll);

	// Bind action buttons
	if (ResearchButton)
		ResearchButton->OnClicked.AddDynamic(this, &UTechTreeDisplayWidget::OnResearchClicked);
	if (CloseButton)
		CloseButton->OnClicked.AddDynamic(this, &UTechTreeDisplayWidget::OnCloseClicked);

	// Initial population
	PopulateTechTree();
	UpdateCategoryPoints();
}

void UTechTreeDisplayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update research progress if actively researching
	if (TechTreeManager)
	{
		TechTreeManager->UpdateResearch(InDeltaTime);
	}
}

void UTechTreeDisplayWidget::PopulateTechTree()
{
	if (bShowingAllCategories)
	{
		PopulateAllTechs();
	}
	else
	{
		PopulateTechsByCategory(CurrentFilter);
	}
}

void UTechTreeDisplayWidget::PopulateAllTechs()
{
	if (!TechTreeManager || !TechListScrollBox || !TechNodeWidgetClass)
	{
		return;
	}

	TechListScrollBox->ClearChildren();
	TechNodeWidgets.Empty();

	TArray<FTechNode> AllTechs = TechTreeManager->GetAllTechs();

	// Organize by category and tier
	TMap<ETechCategory, TArray<FTechNode>> TechsByCategory;

	for (const FTechNode& Tech : AllTechs)
	{
		if (!TechsByCategory.Contains(Tech.Category))
		{
			TechsByCategory.Add(Tech.Category, TArray<FTechNode>());
		}
		TechsByCategory[Tech.Category].Add(Tech);
	}

	// Display each category
	for (const auto& Pair : TechsByCategory)
	{
		// TODO: Add category header
		for (const FTechNode& Tech : Pair.Value)
		{
			CreateTechNodeWidget(Tech);
		}
	}
}

void UTechTreeDisplayWidget::PopulateTechsByCategory(ETechCategory Category)
{
	if (!TechTreeManager || !TechListScrollBox || !TechNodeWidgetClass)
	{
		return;
	}

	TechListScrollBox->ClearChildren();
	TechNodeWidgets.Empty();

	TArray<FTechNode> CategoryTechs = TechTreeManager->GetTechsByCategory(Category);

	for (const FTechNode& Tech : CategoryTechs)
	{
		CreateTechNodeWidget(Tech);
	}
}

void UTechTreeDisplayWidget::CreateTechNodeWidget(const FTechNode& Tech)
{
	if (!TechNodeWidgetClass || !TechListScrollBox)
	{
		return;
	}

	UTechNodeWidget* TechWidget = CreateWidget<UTechNodeWidget>(this, TechNodeWidgetClass);
	if (TechWidget)
	{
		TechWidget->SetTechData(Tech);
		TechWidget->OnTechClicked.AddDynamic(this, &UTechTreeDisplayWidget::OnTechNodeClicked);

		TechListScrollBox->AddChild(TechWidget);
		TechNodeWidgets.Add(TechWidget);
	}
}

void UTechTreeDisplayWidget::FilterByCategory(ETechCategory Category)
{
	CurrentFilter = Category;
	bShowingAllCategories = false;
	PopulateTechTree();
}

void UTechTreeDisplayWidget::ShowAllCategories()
{
	bShowingAllCategories = true;
	PopulateTechTree();
}

void UTechTreeDisplayWidget::RefreshDisplay()
{
	PopulateTechTree();
	UpdateCategoryPoints();

	// Refresh selected tech details if one is selected
	if (!SelectedTechID.IsEmpty() && TechTreeManager)
	{
		FTechNode Tech = TechTreeManager->GetTechByID(SelectedTechID);
		DisplayTechDetails(Tech);
	}
}

void UTechTreeDisplayWidget::UpdateCategoryPoints()
{
	if (!TechTreeManager)
	{
		return;
	}

	if (PhysicsPointsText)
		PhysicsPointsText->SetText(FText::FromString(
			FString::Printf(TEXT("Physics: %d"), TechTreeManager->GetCategoryPoints(ETechCategory::Physics))));

	if (EngineeringPointsText)
		EngineeringPointsText->SetText(FText::FromString(
			FString::Printf(TEXT("Engineering: %d"), TechTreeManager->GetCategoryPoints(ETechCategory::Engineering))));

	if (EnergyPointsText)
		EnergyPointsText->SetText(FText::FromString(
			FString::Printf(TEXT("Energy: %d"), TechTreeManager->GetCategoryPoints(ETechCategory::Energy))));

	if (ComputingPointsText)
		ComputingPointsText->SetText(FText::FromString(
			FString::Printf(TEXT("Computing: %d"), TechTreeManager->GetCategoryPoints(ETechCategory::Computing))));

	if (BiologyPointsText)
		BiologyPointsText->SetText(FText::FromString(
			FString::Printf(TEXT("Biology: %d"), TechTreeManager->GetCategoryPoints(ETechCategory::Biology))));

	if (SensorsPointsText)
		SensorsPointsText->SetText(FText::FromString(
			FString::Printf(TEXT("Sensors: %d"), TechTreeManager->GetCategoryPoints(ETechCategory::Sensors))));
}

void UTechTreeDisplayWidget::OnTechNodeClicked(const FString& TechID)
{
	SelectedTechID = TechID;

	if (!TechTreeManager)
	{
		return;
	}

	FTechNode Tech = TechTreeManager->GetTechByID(TechID);
	DisplayTechDetails(Tech);
}

void UTechTreeDisplayWidget::DisplayTechDetails(const FTechNode& Tech)
{
	if (SelectedTechNameText)
	{
		FString CategoryName;
		switch (Tech.Category)
		{
		case ETechCategory::Physics: CategoryName = TEXT("Physics"); break;
		case ETechCategory::Engineering: CategoryName = TEXT("Engineering"); break;
		case ETechCategory::Energy: CategoryName = TEXT("Energy"); break;
		case ETechCategory::Computing: CategoryName = TEXT("Computing"); break;
		case ETechCategory::Biology: CategoryName = TEXT("Biology"); break;
		case ETechCategory::Sensors: CategoryName = TEXT("Sensors"); break;
		}

		FString DisplayName = FString::Printf(TEXT("%s [%s]"), *Tech.TechName, *CategoryName);
		SelectedTechNameText->SetText(FText::FromString(DisplayName));
	}

	if (SelectedTechDescriptionText)
	{
		SelectedTechDescriptionText->SetText(Tech.Description);
	}

	if (SelectedTechEffectText)
	{
		SelectedTechEffectText->SetText(Tech.DetailedEffect);
	}

	if (SelectedTechPrereqText)
	{
		FString PrereqText = TEXT("Prerequisites:\n");
		if (Tech.Prerequisites.Num() == 0)
		{
			PrereqText += TEXT("None");
		}
		else
		{
			for (const FTechPrerequisite& Prereq : Tech.Prerequisites)
			{
				FString CategoryName;
				switch (Prereq.Category)
				{
				case ETechCategory::Physics: CategoryName = TEXT("Physics"); break;
				case ETechCategory::Engineering: CategoryName = TEXT("Engineering"); break;
				case ETechCategory::Energy: CategoryName = TEXT("Energy"); break;
				case ETechCategory::Computing: CategoryName = TEXT("Computing"); break;
				case ETechCategory::Biology: CategoryName = TEXT("Biology"); break;
				case ETechCategory::Sensors: CategoryName = TEXT("Sensors"); break;
				}

				PrereqText += FString::Printf(TEXT("- %s: %d points\n"), *CategoryName, Prereq.MinimumPoints);
			}
		}

		SelectedTechPrereqText->SetText(FText::FromString(PrereqText));
	}

	// Update research button state
	if (ResearchButton && TechTreeManager)
	{
		bool bCanResearch = TechTreeManager->CanUnlockTech(Tech.TechID);
		bool bAlreadyUnlocked = Tech.bIsUnlocked;

		ResearchButton->SetIsEnabled(bCanResearch && !bAlreadyUnlocked);

		if (bAlreadyUnlocked)
		{
			// TODO: Change button text to "Unlocked"
		}
	}
}

// Filter button callbacks
void UTechTreeDisplayWidget::OnFilterPhysics() { FilterByCategory(ETechCategory::Physics); }
void UTechTreeDisplayWidget::OnFilterEngineering() { FilterByCategory(ETechCategory::Engineering); }
void UTechTreeDisplayWidget::OnFilterEnergy() { FilterByCategory(ETechCategory::Energy); }
void UTechTreeDisplayWidget::OnFilterComputing() { FilterByCategory(ETechCategory::Computing); }
void UTechTreeDisplayWidget::OnFilterBiology() { FilterByCategory(ETechCategory::Biology); }
void UTechTreeDisplayWidget::OnFilterSensors() { FilterByCategory(ETechCategory::Sensors); }
void UTechTreeDisplayWidget::OnFilterAll() { ShowAllCategories(); }

void UTechTreeDisplayWidget::OnResearchClicked()
{
	if (!TechTreeManager || SelectedTechID.IsEmpty())
	{
		return;
	}

	if (TechTreeManager->UnlockTech(SelectedTechID))
	{
		UE_LOG(LogTemp, Log, TEXT("TechTreeDisplayWidget: Unlocked tech %s"), *SelectedTechID);
		RefreshDisplay();
	}
}

void UTechTreeDisplayWidget::OnCloseClicked()
{
	RemoveFromParent();
}

// ===== UTechNodeWidget =====

void UTechNodeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TechButton)
	{
		TechButton->OnClicked.AddDynamic(this, &UTechNodeWidget::OnButtonClicked);
	}
}

void UTechNodeWidget::SetTechData(const FTechNode& InTechData)
{
	TechData = InTechData;

	if (TechNameText)
	{
		TechNameText->SetText(FText::FromString(TechData.TechName));
	}

	if (TechTierText)
	{
		FString TierName;
		switch (TechData.Tier)
		{
		case ETechTier::Tier1: TierName = TEXT("Tier 1"); break;
		case ETechTier::Tier2: TierName = TEXT("Tier 2"); break;
		case ETechTier::Tier3: TierName = TEXT("Tier 3"); break;
		case ETechTier::Tier4: TierName = TEXT("Tier 4"); break;
		case ETechTier::Tier5: TierName = TEXT("Tier 5"); break;
		}
		TechTierText->SetText(FText::FromString(TierName));
	}

	if (TechCategoryText)
	{
		FString CategoryName;
		switch (TechData.Category)
		{
		case ETechCategory::Physics: CategoryName = TEXT("Physics"); break;
		case ETechCategory::Engineering: CategoryName = TEXT("Engineering"); break;
		case ETechCategory::Energy: CategoryName = TEXT("Energy"); break;
		case ETechCategory::Computing: CategoryName = TEXT("Computing"); break;
		case ETechCategory::Biology: CategoryName = TEXT("Biology"); break;
		case ETechCategory::Sensors: CategoryName = TEXT("Sensors"); break;
		}
		TechCategoryText->SetText(FText::FromString(CategoryName));
	}

	UpdateVisualState();
}

void UTechNodeWidget::UpdateVisualState()
{
	// Update visual appearance based on unlock status
	// Color coding: Green for unlocked, Yellow for available, Gray for locked

	if (!TechButton)
	{
		return;
	}

	// TODO: Set colors based on state
	// This would typically be done with button styles in Blueprint
}

void UTechNodeWidget::OnButtonClicked()
{
	OnTechClicked.Broadcast(TechData.TechID);
}
