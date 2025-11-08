// Source/astrochicken/UI/ShipManagement/PowerGridPanel.cpp

#include "PowerGridPanel.h"
#include "../../Infrastructure/ShipInfrastructureManager.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/Slider.h"

// ===== UPowerGridPanel =====

void UPowerGridPanel::NativeConstruct()
{
	Super::NativeConstruct();

	// Get infrastructure manager
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		InfraManager = GameInstance->GetSubsystem<UShipInfrastructureManager>();
	}

	RefreshDisplay();
}

void UPowerGridPanel::RefreshDisplay()
{
	if (!InfraManager)
	{
		return;
	}

	UpdateSummary();
	PopulateReactorList();
	PopulatePowerNodeList();
}

void UPowerGridPanel::UpdateSummary()
{
	if (!InfraManager)
	{
		return;
	}

	FElectricalGrid Grid = InfraManager->GetElectricalGrid();

	// Total generation
	if (TotalGenerationText)
	{
		TotalGenerationText->SetText(FText::FromString(
			FString::Printf(TEXT("%.1f kW"), Grid.TotalGeneration)));
	}

	// Total consumption
	if (TotalConsumptionText)
	{
		TotalConsumptionText->SetText(FText::FromString(
			FString::Printf(TEXT("%.1f kW"), Grid.TotalConsumption)));
	}

	// Power balance
	if (PowerBalanceText && PowerBalanceBar)
	{
		float Balance = Grid.TotalGeneration - Grid.TotalConsumption;
		bool bDeficit = Balance < 0.0f;

		PowerBalanceText->SetText(FText::FromString(
			FString::Printf(TEXT("%s%.1f kW"), bDeficit ? TEXT("") : TEXT("+"), Balance)));

		// Color based on balance
		FLinearColor Color = bDeficit ? FLinearColor::Red :
		                     Balance < 10.0f ? FLinearColor::Yellow :
		                     FLinearColor::Green;
		PowerBalanceText->SetColorAndOpacity(Color);

		// Bar shows load percentage
		float LoadPercent = (Grid.TotalGeneration > 0.0f) ?
			FMath::Clamp(Grid.TotalConsumption / Grid.TotalGeneration, 0.0f, 1.0f) : 0.0f;
		PowerBalanceBar->SetPercent(LoadPercent);
		PowerBalanceBar->SetFillColorAndOpacity(Color);
	}

	// Grid efficiency
	if (GridEfficiencyText)
	{
		GridEfficiencyText->SetText(FText::FromString(
			FString::Printf(TEXT("%.1f%%"), Grid.GridEfficiency)));
	}

	// Offline nodes
	if (OfflineNodesText)
	{
		OfflineNodesText->SetText(FText::FromString(
			FString::Printf(TEXT("%d Offline"), Grid.OfflineNodes)));

		FLinearColor Color = Grid.OfflineNodes > 0 ? FLinearColor::Red : FLinearColor::Green;
		OfflineNodesText->SetColorAndOpacity(Color);
	}

	// Overloaded conduits
	if (OverloadedConduitsText)
	{
		OverloadedConduitsText->SetText(FText::FromString(
			FString::Printf(TEXT("%d Overloaded"), Grid.OverloadedConduits)));

		FLinearColor Color = Grid.OverloadedConduits > 0 ? FLinearColor::Red : FLinearColor::Green;
		OverloadedConduitsText->SetColorAndOpacity(Color);
	}
}

void UPowerGridPanel::PopulateReactorList()
{
	if (!InfraManager || !ReactorListScrollBox || !ReactorWidgetClass)
	{
		return;
	}

	ReactorListScrollBox->ClearChildren();

	FElectricalGrid Grid = InfraManager->GetElectricalGrid();

	for (const auto& Pair : Grid.Reactors)
	{
		UReactorWidget* ReactorWidget = CreateWidget<UReactorWidget>(this, ReactorWidgetClass);
		if (ReactorWidget)
		{
			ReactorWidget->SetReactorData(Pair.Value, Pair.Key);
			ReactorListScrollBox->AddChild(ReactorWidget);
		}
	}
}

void UPowerGridPanel::PopulatePowerNodeList()
{
	if (!InfraManager || !PowerNodeListScrollBox || !PowerNodeWidgetClass)
	{
		return;
	}

	PowerNodeListScrollBox->ClearChildren();

	FElectricalGrid Grid = InfraManager->GetElectricalGrid();

	// Show only important nodes (consumers and junctions)
	for (const auto& Pair : Grid.Nodes)
	{
		const FPowerNode& Node = Pair.Value;

		// Skip reactor nodes (shown in reactor list)
		if (Node.Type == EPowerNodeType::Reactor)
		{
			continue;
		}

		UPowerNodeWidget* NodeWidget = CreateWidget<UPowerNodeWidget>(this, PowerNodeWidgetClass);
		if (NodeWidget)
		{
			NodeWidget->SetNodeData(Node);
			PowerNodeListScrollBox->AddChild(NodeWidget);
		}
	}
}

// ===== UReactorWidget =====

void UReactorWidget::SetReactorData(const FReactor& InReactorData, const FString& InReactorID)
{
	ReactorData = InReactorData;
	ReactorID = InReactorID;

	// Reactor name
	if (ReactorNameText)
	{
		ReactorNameText->SetText(FText::FromString(ReactorID));
	}

	// Reactor type
	if (ReactorTypeText)
	{
		FString TypeName;
		switch (ReactorData.ReactorType)
		{
		case EReactorType::Fission: TypeName = TEXT("Fission"); break;
		case EReactorType::Fusion: TypeName = TEXT("Fusion"); break;
		case EReactorType::Antimatter: TypeName = TEXT("Antimatter"); break;
		case EReactorType::Solar: TypeName = TEXT("Solar"); break;
		case EReactorType::Chemical: TypeName = TEXT("Chemical"); break;
		}
		ReactorTypeText->SetText(FText::FromString(TypeName));
	}

	// Power output
	if (PowerOutputBar && PowerOutputText)
	{
		float Efficiency = ReactorData.GetEfficiency();
		PowerOutputBar->SetPercent(Efficiency);
		PowerOutputText->SetText(FText::FromString(
			FString::Printf(TEXT("%.0f%% (%.1f efficiency)"), ReactorData.PowerOutputPercent, Efficiency * 100.0f)));

		FLinearColor Color = Efficiency > 0.8f ? FLinearColor::Green :
		                     Efficiency > 0.5f ? FLinearColor::Yellow :
		                     FLinearColor::Red;
		PowerOutputBar->SetFillColorAndOpacity(Color);
	}

	// Output slider
	if (OutputSlider)
	{
		OutputSlider->SetValue(ReactorData.PowerOutputPercent / 100.0f);
		OutputSlider->OnValueChanged.AddDynamic(this, &UReactorWidget::OnOutputSliderChanged);
	}

	// Fuel
	if (FuelBar && FuelText)
	{
		FuelBar->SetPercent(ReactorData.FuelLevel / 100.0f);
		FuelText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), ReactorData.FuelLevel)));

		FLinearColor Color = ReactorData.FuelLevel > 50.0f ? FLinearColor::Green :
		                     ReactorData.FuelLevel > 20.0f ? FLinearColor::Yellow :
		                     FLinearColor::Red;
		FuelBar->SetFillColorAndOpacity(Color);
	}

	// Temperature
	if (TemperatureBar && TemperatureText)
	{
		float TempPercent = ReactorData.Temperature / ReactorData.MaxSafeTemperature;
		TemperatureBar->SetPercent(FMath::Clamp(TempPercent, 0.0f, 1.0f));
		TemperatureText->SetText(FText::FromString(
			FString::Printf(TEXT("%.0f°C"), ReactorData.Temperature)));

		FLinearColor Color = ReactorData.IsCritical() ? FLinearColor::Red :
		                     ReactorData.IsOverheating() ? FLinearColor(1.0f, 0.5f, 0.0f) :
		                     FLinearColor::Green;
		TemperatureBar->SetFillColorAndOpacity(Color);
	}

	// Status
	if (StatusText)
	{
		FString Status;
		FLinearColor Color = FLinearColor::Green;

		if (ReactorData.bEmergencyShutdown)
		{
			Status = TEXT("EMERGENCY SHUTDOWN");
			Color = FLinearColor::Red;
		}
		else if (ReactorData.IsCritical())
		{
			Status = TEXT("CRITICAL - OVERHEATING");
			Color = FLinearColor::Red;
		}
		else if (ReactorData.IsOverheating())
		{
			Status = TEXT("Warning: Overheating");
			Color = FLinearColor::Yellow;
		}
		else if (ReactorData.FuelLevel < 20.0f)
		{
			Status = TEXT("Warning: Low Fuel");
			Color = FLinearColor::Yellow;
		}
		else
		{
			Status = TEXT("Operational");
		}

		StatusText->SetText(FText::FromString(Status));
		StatusText->SetColorAndOpacity(Color);
	}
}

void UReactorWidget::OnOutputSliderChanged(float Value)
{
	float NewOutputPercent = Value * 100.0f;

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UShipInfrastructureManager* InfraManager = GameInstance->GetSubsystem<UShipInfrastructureManager>())
		{
			InfraManager->SetReactorOutput(ReactorID, NewOutputPercent);
			UE_LOG(LogTemp, Log, TEXT("Reactor %s output set to %.1f%%"), *ReactorID, NewOutputPercent);
		}
	}
}

// ===== UPowerNodeWidget =====

void UPowerNodeWidget::SetNodeData(const FPowerNode& InNodeData)
{
	NodeData = InNodeData;

	// Node name
	if (NodeNameText)
	{
		NodeNameText->SetText(FText::FromString(NodeData.NodeName));
	}

	// Node type
	if (NodeTypeText)
	{
		FString TypeName;
		switch (NodeData.Type)
		{
		case EPowerNodeType::Reactor: TypeName = TEXT("Reactor"); break;
		case EPowerNodeType::Junction: TypeName = TEXT("Junction"); break;
		case EPowerNodeType::Distributor: TypeName = TEXT("Distributor"); break;
		case EPowerNodeType::Capacitor: TypeName = TEXT("Capacitor"); break;
		case EPowerNodeType::Consumer: TypeName = TEXT("Consumer"); break;
		}
		NodeTypeText->SetText(FText::FromString(TypeName));
	}

	// Node status
	if (NodeStatusText)
	{
		FString Status;
		FLinearColor Color = FLinearColor::Green;

		switch (NodeData.Status)
		{
		case EPowerNodeStatus::Operational:
			Status = TEXT("Operational");
			break;
		case EPowerNodeStatus::Overloaded:
			Status = TEXT("OVERLOADED");
			Color = FLinearColor::Red;
			break;
		case EPowerNodeStatus::Damaged:
			Status = TEXT("Damaged");
			Color = FLinearColor::Yellow;
			break;
		case EPowerNodeStatus::Offline:
			Status = TEXT("OFFLINE");
			Color = FLinearColor::Red;
			break;
		case EPowerNodeStatus::Emergency:
			Status = TEXT("Emergency Power");
			Color = FLinearColor::Yellow;
			break;
		}

		NodeStatusText->SetText(FText::FromString(Status));
		NodeStatusText->SetColorAndOpacity(Color);
	}

	// Health
	if (HealthBar)
	{
		HealthBar->SetPercent(NodeData.Health / 100.0f);

		FLinearColor Color = NodeData.Health > 75.0f ? FLinearColor::Green :
		                     NodeData.Health > 50.0f ? FLinearColor::Yellow :
		                     NodeData.Health > 25.0f ? FLinearColor(1.0f, 0.5f, 0.0f) :
		                     FLinearColor::Red;
		HealthBar->SetFillColorAndOpacity(Color);
	}

	// Power flow
	if (PowerFlowText)
	{
		if (NodeData.IsGenerator())
		{
			PowerFlowText->SetText(FText::FromString(
				FString::Printf(TEXT("Generating: %.1f kW"), NodeData.PowerGeneration)));
		}
		else if (NodeData.IsConsumer())
		{
			PowerFlowText->SetText(FText::FromString(
				FString::Printf(TEXT("Consuming: %.1f / %.1f kW"),
					NodeData.PowerConsumption, NodeData.RequiredPower)));
		}
		else
		{
			PowerFlowText->SetText(FText::FromString(
				FString::Printf(TEXT("Available: %.1f kW"), NodeData.AvailablePower)));
		}
	}

	// Compartment
	if (CompartmentText)
	{
		CompartmentText->SetText(FText::FromString(NodeData.CompartmentID));
	}
}
