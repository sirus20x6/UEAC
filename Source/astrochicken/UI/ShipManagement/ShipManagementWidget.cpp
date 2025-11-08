// Source/astrochicken/UI/ShipManagement/ShipManagementWidget.cpp

#include "ShipManagementWidget.h"
#include "PowerGridPanel.h"
#include "CompartmentStatusPanel.h"
#include "HardpointPanel.h"
#include "ResourcePanel.h"
#include "../../Infrastructure/ShipInfrastructureManager.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/NamedSlot.h"

void UShipManagementWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get infrastructure manager
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		InfraManager = GameInstance->GetSubsystem<UShipInfrastructureManager>();
	}

	// Bind tab buttons
	if (PowerTabButton)
		PowerTabButton->OnClicked.AddDynamic(this, &UShipManagementWidget::OnPowerTabClicked);
	if (CompartmentsTabButton)
		CompartmentsTabButton->OnClicked.AddDynamic(this, &UShipManagementWidget::OnCompartmentsTabClicked);
	if (HardpointsTabButton)
		HardpointsTabButton->OnClicked.AddDynamic(this, &UShipManagementWidget::OnHardpointsTabClicked);
	if (ResourcesTabButton)
		ResourcesTabButton->OnClicked.AddDynamic(this, &UShipManagementWidget::OnResourcesTabClicked);
	if (AlertsTabButton)
		AlertsTabButton->OnClicked.AddDynamic(this, &UShipManagementWidget::OnAlertsTabClicked);

	// Create panel widgets
	CreatePanels();

	// Initial update
	UpdateQuickStatus();
	UpdateTabButtonStates();

	UE_LOG(LogTemp, Log, TEXT("ShipManagementWidget: Constructed"));
}

void UShipManagementWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!InfraManager)
	{
		return;
	}

	// Update ship systems
	InfraManager->UpdateSystems(InDeltaTime);

	// Update UI periodically
	UpdateAccumulator += InDeltaTime;
	if (UpdateAccumulator >= UpdateInterval)
	{
		UpdateQuickStatus();
		UpdateAlertCount();
		UpdateAccumulator = 0.0f;
	}
}

void UShipManagementWidget::CreatePanels()
{
	// Create Power Grid Panel
	if (PowerGridPanelClass && PowerPanelSlot)
	{
		PowerPanel = CreateWidget<UPowerGridPanel>(this, PowerGridPanelClass);
		if (PowerPanel)
		{
			PowerPanelSlot->ClearChildren();
			PowerPanelSlot->AddChild(PowerPanel);
		}
	}

	// Create Compartment Status Panel
	if (CompartmentStatusPanelClass && CompartmentsPanelSlot)
	{
		CompartmentPanel = CreateWidget<UCompartmentStatusPanel>(this, CompartmentStatusPanelClass);
		if (CompartmentPanel)
		{
			CompartmentsPanelSlot->ClearChildren();
			CompartmentsPanelSlot->AddChild(CompartmentPanel);
		}
	}

	// Create Hardpoint Panel
	if (HardpointPanelClass && HardpointsPanelSlot)
	{
		HardpointPanelWidget = CreateWidget<UHardpointPanel>(this, HardpointPanelClass);
		if (HardpointPanelWidget)
		{
			HardpointsPanelSlot->ClearChildren();
			HardpointsPanelSlot->AddChild(HardpointPanelWidget);
		}
	}

	// Create Resource Panel
	if (ResourcePanelClass && ResourcesPanelSlot)
	{
		ResourcePanelWidget = CreateWidget<UResourcePanel>(this, ResourcePanelClass);
		if (ResourcePanelWidget)
		{
			ResourcesPanelSlot->ClearChildren();
			ResourcesPanelSlot->AddChild(ResourcePanelWidget);
		}
	}
}

void UShipManagementWidget::UpdateQuickStatus()
{
	if (!InfraManager)
	{
		return;
	}

	FShipLayout Layout = InfraManager->GetShipLayout();
	FElectricalGrid Grid = InfraManager->GetElectricalGrid();
	FResourceNetwork Resources = InfraManager->GetResourceNetwork();

	// Ship name
	if (ShipNameText)
	{
		ShipNameText->SetText(FText::FromString(Layout.ShipName));
	}

	// Overall integrity
	if (OverallIntegrityBar && OverallIntegrityText)
	{
		float Integrity = Layout.OverallIntegrity;
		OverallIntegrityBar->SetPercent(Integrity / 100.0f);
		OverallIntegrityText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), Integrity)));

		// Color based on integrity
		FLinearColor Color = Integrity > 75.0f ? FLinearColor::Green :
		                     Integrity > 50.0f ? FLinearColor::Yellow :
		                     Integrity > 25.0f ? FLinearColor(1.0f, 0.5f, 0.0f) : // Orange
		                     FLinearColor::Red;
		OverallIntegrityBar->SetFillColorAndOpacity(Color);
	}

	// Power status
	if (PowerBar && PowerText)
	{
		float Generation = Grid.TotalGeneration;
		float Consumption = Grid.TotalConsumption;
		float Percent = (Generation > 0.0f) ? FMath::Clamp(Consumption / Generation, 0.0f, 1.0f) : 0.0f;

		PowerBar->SetPercent(Percent);
		PowerText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f kW"), Consumption, Generation)));

		// Color based on power status
		FLinearColor Color = Grid.HasPowerDeficit() ? FLinearColor::Red :
		                     Percent > 0.9f ? FLinearColor::Yellow :
		                     FLinearColor::Green;
		PowerBar->SetFillColorAndOpacity(Color);
	}

	// Oxygen status
	if (OxygenBar && OxygenText)
	{
		float OxygenLevel = Resources.TotalOxygen;
		float OxygenPercent = FMath::Clamp(OxygenLevel / 100.0f, 0.0f, 1.0f);

		OxygenBar->SetPercent(OxygenPercent);
		OxygenText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), OxygenLevel)));

		FLinearColor Color = OxygenLevel > 80.0f ? FLinearColor::Green :
		                     OxygenLevel > 50.0f ? FLinearColor::Yellow :
		                     FLinearColor::Red;
		OxygenBar->SetFillColorAndOpacity(Color);
	}
}

void UShipManagementWidget::UpdateAlertCount()
{
	if (!InfraManager)
	{
		return;
	}

	AlertCount = 0;

	FElectricalGrid Grid = InfraManager->GetElectricalGrid();
	FResourceNetwork Resources = InfraManager->GetResourceNetwork();

	// Count alerts
	if (Grid.HasPowerDeficit())
		AlertCount++;

	if (Grid.OverloadedConduits > 0)
		AlertCount++;

	if (Resources.LeakingPipes > 0)
		AlertCount++;

	if (!InfraManager->IsLifeSupportOperational())
		AlertCount++;

	// Update UI
	if (AlertCountText)
	{
		if (AlertCount > 0)
		{
			AlertCountText->SetText(FText::FromString(FString::Printf(TEXT("⚠ %d"), AlertCount)));
			AlertCountText->SetColorAndOpacity(FLinearColor::Red);
		}
		else
		{
			AlertCountText->SetText(FText::FromString(TEXT("✓ All Systems Nominal")));
			AlertCountText->SetColorAndOpacity(FLinearColor::Green);
		}
	}
}

void UShipManagementWidget::RefreshAllPanels()
{
	if (PowerPanel)
		PowerPanel->RefreshDisplay();

	if (CompartmentPanel)
		CompartmentPanel->RefreshDisplay();

	if (HardpointPanelWidget)
		HardpointPanelWidget->RefreshDisplay();

	if (ResourcePanelWidget)
		ResourcePanelWidget->RefreshDisplay();

	UpdateQuickStatus();
	UpdateAlertCount();
}

void UShipManagementWidget::SwitchToTab(int32 TabIndex)
{
	if (!TabSwitcher)
	{
		return;
	}

	CurrentTabIndex = TabIndex;
	TabSwitcher->SetActiveWidgetIndex(TabIndex);
	UpdateTabButtonStates();

	UE_LOG(LogTemp, Log, TEXT("ShipManagementWidget: Switched to tab %d"), TabIndex);
}

void UShipManagementWidget::UpdateTabButtonStates()
{
	// TODO: Update button visual states to show which tab is active
	// This would typically be done with button styles in Blueprint
}

// Tab button callbacks
void UShipManagementWidget::OnPowerTabClicked()
{
	SwitchToTab(0);
	if (PowerPanel)
		PowerPanel->RefreshDisplay();
}

void UShipManagementWidget::OnCompartmentsTabClicked()
{
	SwitchToTab(1);
	if (CompartmentPanel)
		CompartmentPanel->RefreshDisplay();
}

void UShipManagementWidget::OnHardpointsTabClicked()
{
	SwitchToTab(2);
	if (HardpointPanelWidget)
		HardpointPanelWidget->RefreshDisplay();
}

void UShipManagementWidget::OnResourcesTabClicked()
{
	SwitchToTab(3);
	if (ResourcePanelWidget)
		ResourcePanelWidget->RefreshDisplay();
}

void UShipManagementWidget::OnAlertsTabClicked()
{
	SwitchToTab(4);
	// TODO: Alerts panel
}
