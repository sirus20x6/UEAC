// Source/astrochicken/TechTree/TechPointAllocationWidget.cpp

#include "TechPointAllocationWidget.h"
#include "TechTreeManager.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UTechPointAllocationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get tech tree manager
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		TechTreeManager = GameInstance->GetSubsystem<UTechTreeManager>();
	}

	// Bind button events - Physics
	if (PhysicsIncrementButton)
		PhysicsIncrementButton->OnClicked.AddDynamic(this, &UTechPointAllocationWidget::OnPhysicsIncrement);
	if (PhysicsDecrementButton)
		PhysicsDecrementButton->OnClicked.AddDynamic(this, &UTechPointAllocationWidget::OnPhysicsDecrement);

	// Engineering
	if (EngineeringIncrementButton)
		EngineeringIncrementButton->OnClicked.AddDynamic(this, &UTechPointAllocationWidget::OnEngineeringIncrement);
	if (EngineeringDecrementButton)
		EngineeringDecrementButton->OnClicked.AddDynamic(this, &UTechPointAllocationWidget::OnEngineeringDecrement);

	// Energy
	if (EnergyIncrementButton)
		EnergyIncrementButton->OnClicked.AddDynamic(this, &UTechPointAllocationWidget::OnEnergyIncrement);
	if (EnergyDecrementButton)
		EnergyDecrementButton->OnClicked.AddDynamic(this, &UTechPointAllocationWidget::OnEnergyDecrement);

	// Computing
	if (ComputingIncrementButton)
		ComputingIncrementButton->OnClicked.AddDynamic(this, &UTechPointAllocationWidget::OnComputingIncrement);
	if (ComputingDecrementButton)
		ComputingDecrementButton->OnClicked.AddDynamic(this, &UTechPointAllocationWidget::OnComputingDecrement);

	// Biology
	if (BiologyIncrementButton)
		BiologyIncrementButton->OnClicked.AddDynamic(this, &UTechPointAllocationWidget::OnBiologyIncrement);
	if (BiologyDecrementButton)
		BiologyDecrementButton->OnClicked.AddDynamic(this, &UTechPointAllocationWidget::OnBiologyDecrement);

	// Sensors
	if (SensorsIncrementButton)
		SensorsIncrementButton->OnClicked.AddDynamic(this, &UTechPointAllocationWidget::OnSensorsIncrement);
	if (SensorsDecrementButton)
		SensorsDecrementButton->OnClicked.AddDynamic(this, &UTechPointAllocationWidget::OnSensorsDecrement);

	// Action buttons
	if (ConfirmButton)
		ConfirmButton->OnClicked.AddDynamic(this, &UTechPointAllocationWidget::OnConfirmClicked);
	if (ResetButton)
		ResetButton->OnClicked.AddDynamic(this, &UTechPointAllocationWidget::OnResetClicked);
	if (BackButton)
		BackButton->OnClicked.AddDynamic(this, &UTechPointAllocationWidget::OnBackClicked);

	// Initialize bonus allocation map
	BonusAllocation.Empty();
	BonusAllocation.Add(ETechCategory::Physics, 0);
	BonusAllocation.Add(ETechCategory::Engineering, 0);
	BonusAllocation.Add(ETechCategory::Energy, 0);
	BonusAllocation.Add(ETechCategory::Computing, 0);
	BonusAllocation.Add(ETechCategory::Biology, 0);
	BonusAllocation.Add(ETechCategory::Sensors, 0);
}

void UTechPointAllocationWidget::Initialize(int32 BonusPoints)
{
	TotalBonusPoints = BonusPoints;
	AllocatedBonusPoints = 0;

	// Reset bonus allocation
	for (auto& Pair : BonusAllocation)
	{
		Pair.Value = 0;
	}

	RefreshDisplay();
}

void UTechPointAllocationWidget::RefreshDisplay()
{
	UpdateAllDisplays();
	UpdateConfirmButtonState();
}

void UTechPointAllocationWidget::UpdateAllDisplays()
{
	if (!TechTreeManager)
	{
		return;
	}

	// Update header
	if (TitleText)
	{
		TitleText->SetText(FText::FromString(TEXT("Allocate Research Points")));
	}

	if (InstructionsText)
	{
		InstructionsText->SetText(FText::FromString(
			TEXT("Distribute bonus research points across technology disciplines.\n")
			TEXT("These points determine which technologies you can unlock.")
		));
	}

	if (RemainingPointsText)
	{
		int32 Remaining = TotalBonusPoints - AllocatedBonusPoints;
		RemainingPointsText->SetText(FText::FromString(
			FString::Printf(TEXT("Remaining Points: %d / %d"), Remaining, TotalBonusPoints)
		));
	}

	// Update each category
	UpdateCategoryDisplay(ETechCategory::Physics);
	UpdateCategoryDisplay(ETechCategory::Engineering);
	UpdateCategoryDisplay(ETechCategory::Energy);
	UpdateCategoryDisplay(ETechCategory::Computing);
	UpdateCategoryDisplay(ETechCategory::Biology);
	UpdateCategoryDisplay(ETechCategory::Sensors);
}

void UTechPointAllocationWidget::UpdateCategoryDisplay(ETechCategory Category)
{
	if (!TechTreeManager)
	{
		return;
	}

	int32 BasePoints = TechTreeManager->GetCategoryPoints(Category);
	int32 BonusPoints = BonusAllocation.Contains(Category) ? BonusAllocation[Category] : 0;
	int32 TotalPoints = BasePoints + BonusPoints;

	FString DisplayText = FString::Printf(TEXT("%d (%d base + %d bonus)"), TotalPoints, BasePoints, BonusPoints);

	switch (Category)
	{
	case ETechCategory::Physics:
		if (PhysicsPointsText)
			PhysicsPointsText->SetText(FText::FromString(DisplayText));
		break;

	case ETechCategory::Engineering:
		if (EngineeringPointsText)
			EngineeringPointsText->SetText(FText::FromString(DisplayText));
		break;

	case ETechCategory::Energy:
		if (EnergyPointsText)
			EnergyPointsText->SetText(FText::FromString(DisplayText));
		break;

	case ETechCategory::Computing:
		if (ComputingPointsText)
			ComputingPointsText->SetText(FText::FromString(DisplayText));
		break;

	case ETechCategory::Biology:
		if (BiologyPointsText)
			BiologyPointsText->SetText(FText::FromString(DisplayText));
		break;

	case ETechCategory::Sensors:
		if (SensorsPointsText)
			SensorsPointsText->SetText(FText::FromString(DisplayText));
		break;
	}
}

void UTechPointAllocationWidget::ModifyPoints(ETechCategory Category, int32 Delta)
{
	if (!BonusAllocation.Contains(Category))
	{
		return;
	}

	int32 CurrentBonus = BonusAllocation[Category];
	int32 NewBonus = CurrentBonus + Delta;

	// Can't go negative
	if (NewBonus < 0)
	{
		return;
	}

	// Check if we have enough unallocated points
	if (Delta > 0)
	{
		int32 Remaining = TotalBonusPoints - AllocatedBonusPoints;
		if (Delta > Remaining)
		{
			return;
		}
	}

	// Apply change
	BonusAllocation[Category] = NewBonus;
	AllocatedBonusPoints += Delta;

	RefreshDisplay();
}

// Increment callbacks
void UTechPointAllocationWidget::OnPhysicsIncrement()
{
	ModifyPoints(ETechCategory::Physics, 1);
}

void UTechPointAllocationWidget::OnEngineeringIncrement()
{
	ModifyPoints(ETechCategory::Engineering, 1);
}

void UTechPointAllocationWidget::OnEnergyIncrement()
{
	ModifyPoints(ETechCategory::Energy, 1);
}

void UTechPointAllocationWidget::OnComputingIncrement()
{
	ModifyPoints(ETechCategory::Computing, 1);
}

void UTechPointAllocationWidget::OnBiologyIncrement()
{
	ModifyPoints(ETechCategory::Biology, 1);
}

void UTechPointAllocationWidget::OnSensorsIncrement()
{
	ModifyPoints(ETechCategory::Sensors, 1);
}

// Decrement callbacks
void UTechPointAllocationWidget::OnPhysicsDecrement()
{
	ModifyPoints(ETechCategory::Physics, -1);
}

void UTechPointAllocationWidget::OnEngineeringDecrement()
{
	ModifyPoints(ETechCategory::Engineering, -1);
}

void UTechPointAllocationWidget::OnEnergyDecrement()
{
	ModifyPoints(ETechCategory::Energy, -1);
}

void UTechPointAllocationWidget::OnComputingDecrement()
{
	ModifyPoints(ETechCategory::Computing, -1);
}

void UTechPointAllocationWidget::OnBiologyDecrement()
{
	ModifyPoints(ETechCategory::Biology, -1);
}

void UTechPointAllocationWidget::OnSensorsDecrement()
{
	ModifyPoints(ETechCategory::Sensors, -1);
}

void UTechPointAllocationWidget::UpdateConfirmButtonState()
{
	if (ConfirmButton)
	{
		// Can confirm if all bonus points are allocated
		bool bCanConfirm = (AllocatedBonusPoints == TotalBonusPoints);
		ConfirmButton->SetIsEnabled(bCanConfirm);
	}
}

void UTechPointAllocationWidget::OnConfirmClicked()
{
	if (!TechTreeManager)
	{
		return;
	}

	// Apply all bonus allocations to the tech tree manager
	for (const auto& Pair : BonusAllocation)
	{
		if (Pair.Value > 0)
		{
			TechTreeManager->AddCategoryPoints(Pair.Key, Pair.Value);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("TechPointAllocationWidget: Confirmed allocation of %d bonus points"), AllocatedBonusPoints);

	// Broadcast completion
	OnComplete.Broadcast();
}

void UTechPointAllocationWidget::OnResetClicked()
{
	// Reset all bonus allocations
	for (auto& Pair : BonusAllocation)
	{
		Pair.Value = 0;
	}

	AllocatedBonusPoints = 0;
	RefreshDisplay();
}

void UTechPointAllocationWidget::OnBackClicked()
{
	// Go back without confirming
	RemoveFromParent();
}
