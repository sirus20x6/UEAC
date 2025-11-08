// Source/astrochicken/UI/ShipManagement/CompartmentStatusPanel.cpp

#include "CompartmentStatusPanel.h"
#include "../../Infrastructure/ShipInfrastructureManager.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UCompartmentStatusPanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		InfraManager = GameInstance->GetSubsystem<UShipInfrastructureManager>();
	}

	RefreshDisplay();
}

void UCompartmentStatusPanel::RefreshDisplay()
{
	if (!InfraManager)
	{
		return;
	}

	UpdateSummary();
	PopulateCompartmentList();
}

void UCompartmentStatusPanel::UpdateSummary()
{
	if (!InfraManager)
	{
		return;
	}

	FShipLayout Layout = InfraManager->GetShipLayout();

	int32 TotalCompartments = Layout.Compartments.Num();
	int32 DamagedCount = 0;
	int32 DepressurizedCount = 0;

	for (const auto& Pair : Layout.Compartments)
	{
		if (Pair.Value.Status == ECompartmentStatus::Damaged ||
		    Pair.Value.Status == ECompartmentStatus::Critical)
		{
			DamagedCount++;
		}

		if (Pair.Value.Status == ECompartmentStatus::Depressurized ||
		    Pair.Value.Status == ECompartmentStatus::Breached)
		{
			DepressurizedCount++;
		}
	}

	if (TotalCompartmentsText)
	{
		TotalCompartmentsText->SetText(FText::FromString(FString::Printf(TEXT("%d Total"), TotalCompartments)));
	}

	if (DamagedCompartmentsText)
	{
		DamagedCompartmentsText->SetText(FText::FromString(FString::Printf(TEXT("%d Damaged"), DamagedCount)));
		FLinearColor Color = DamagedCount > 0 ? FLinearColor::Red : FLinearColor::Green;
		DamagedCompartmentsText->SetColorAndOpacity(Color);
	}

	if (DepressurizedCompartmentsText)
	{
		DepressurizedCompartmentsText->SetText(FText::FromString(FString::Printf(TEXT("%d Depressurized"), DepressurizedCount)));
		FLinearColor Color = DepressurizedCount > 0 ? FLinearColor::Red : FLinearColor::Green;
		DepressurizedCompartmentsText->SetColorAndOpacity(Color);
	}
}

void UCompartmentStatusPanel::PopulateCompartmentList()
{
	if (!InfraManager || !CompartmentListScrollBox || !CompartmentWidgetClass)
	{
		return;
	}

	CompartmentListScrollBox->ClearChildren();

	FShipLayout Layout = InfraManager->GetShipLayout();

	for (const auto& Pair : Layout.Compartments)
	{
		UCompartmentWidget* Widget = CreateWidget<UCompartmentWidget>(this, CompartmentWidgetClass);
		if (Widget)
		{
			Widget->SetCompartmentData(Pair.Value);
			CompartmentListScrollBox->AddChild(Widget);
		}
	}
}

// ===== UCompartmentWidget =====

void UCompartmentWidget::SetCompartmentData(const FShipCompartment& InCompartmentData)
{
	CompartmentData = InCompartmentData;

	if (CompartmentNameText)
	{
		CompartmentNameText->SetText(FText::FromString(CompartmentData.CompartmentName));
	}

	if (CompartmentTypeText)
	{
		FString TypeName = UEnum::GetValueAsString(CompartmentData.Type);
		CompartmentTypeText->SetText(FText::FromString(TypeName));
	}

	if (StatusText)
	{
		FString Status = UEnum::GetValueAsString(CompartmentData.Status);
		StatusText->SetText(FText::FromString(Status));

		FLinearColor Color = CompartmentData.Status == ECompartmentStatus::Operational ? FLinearColor::Green :
		                     CompartmentData.Status == ECompartmentStatus::Damaged ? FLinearColor::Yellow :
		                     FLinearColor::Red;
		StatusText->SetColorAndOpacity(Color);
	}

	if (IntegrityBar && IntegrityText)
	{
		IntegrityBar->SetPercent(CompartmentData.Integrity / 100.0f);
		IntegrityText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), CompartmentData.Integrity)));
	}

	if (OxygenBar && OxygenText)
	{
		OxygenBar->SetPercent(CompartmentData.Atmosphere.OxygenLevel / 100.0f);
		OxygenText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), CompartmentData.Atmosphere.OxygenLevel)));
	}

	if (PressureBar)
	{
		PressureBar->SetPercent(CompartmentData.Atmosphere.Pressure / 100.0f);
	}

	if (TemperatureText)
	{
		TemperatureText->SetText(FText::FromString(FString::Printf(TEXT("%.1f°C"), CompartmentData.Atmosphere.Temperature)));
	}

	if (CrewCountText)
	{
		CrewCountText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d Crew"),
			CompartmentData.CrewIDs.Num(), CompartmentData.MaxCrewCapacity)));
	}
}
