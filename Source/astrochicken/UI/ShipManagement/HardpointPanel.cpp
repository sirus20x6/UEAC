// Source/astrochicken/UI/ShipManagement/HardpointPanel.cpp

#include "HardpointPanel.h"
#include "../../Infrastructure/ShipInfrastructureManager.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

void UHardpointPanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		InfraManager = GameInstance->GetSubsystem<UShipInfrastructureManager>();
	}

	RefreshDisplay();
}

void UHardpointPanel::RefreshDisplay()
{
	if (!InfraManager || !HardpointListScrollBox || !HardpointWidgetClass)
	{
		return;
	}

	FHardpointLayout Layout = InfraManager->GetHardpointLayout();

	if (TotalHardpointsText)
	{
		TotalHardpointsText->SetText(FText::FromString(FString::Printf(TEXT("%d Total"), Layout.TotalHardpoints)));
	}

	if (OccupiedHardpointsText)
	{
		OccupiedHardpointsText->SetText(FText::FromString(FString::Printf(TEXT("%d Occupied"), Layout.OccupiedHardpoints)));
	}

	HardpointListScrollBox->ClearChildren();

	for (const auto& Pair : Layout.Hardpoints)
	{
		UHardpointItemWidget* Widget = CreateWidget<UHardpointItemWidget>(this, HardpointWidgetClass);
		if (Widget)
		{
			Widget->SetHardpointData(Pair.Value);
			HardpointListScrollBox->AddChild(Widget);
		}
	}
}

void UHardpointItemWidget::SetHardpointData(const FHardpoint& InHardpointData)
{
	HardpointData = InHardpointData;

	if (HardpointNameText)
	{
		HardpointNameText->SetText(FText::FromString(HardpointData.HardpointName));
	}

	if (StatusText)
	{
		FString Status = HardpointData.IsEmpty() ? TEXT("Empty") :
		                 HardpointData.Status == EHardpointStatus::Occupied ? TEXT("Occupied") :
		                 TEXT("Damaged");
		StatusText->SetText(FText::FromString(Status));
	}

	if (EquipmentText)
	{
		if (HardpointData.HasMountedEquipment())
		{
			EquipmentText->SetText(FText::FromString(HardpointData.MountedEquipment.EquipmentName));
		}
		else
		{
			EquipmentText->SetText(FText::FromString(TEXT("(None)")));
		}
	}
}
