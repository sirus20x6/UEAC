// Source/astrochicken/GameStart/GameStartWidget.cpp

#include "GameStartWidget.h"
#include "GameStartFlow.h"
#include "../Race/RaceSelectionWidget.h"
#include "../TechTree/TechPointAllocationWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/NamedSlot.h"

void UGameStartWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get GameStartFlow subsystem
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		GameStartFlow = GameInstance->GetSubsystem<UGameStartFlow>();

		if (GameStartFlow)
		{
			// Bind to phase change events
			GameStartFlow->OnPhaseChanged.AddDynamic(this, &UGameStartWidget::OnPhaseChanged);
			GameStartFlow->OnGameStartComplete.AddDynamic(this, &UGameStartWidget::OnGameStartComplete);
		}
	}

	// Create child widgets
	CreateRaceSelectionWidget();
	CreateTechAllocationWidget();

	UE_LOG(LogTemp, Log, TEXT("GameStartWidget: Constructed"));
}

void UGameStartWidget::NativeDestruct()
{
	// Unbind from events
	if (GameStartFlow)
	{
		GameStartFlow->OnPhaseChanged.RemoveDynamic(this, &UGameStartWidget::OnPhaseChanged);
		GameStartFlow->OnGameStartComplete.RemoveDynamic(this, &UGameStartWidget::OnGameStartComplete);
	}

	Super::NativeDestruct();
}

void UGameStartWidget::BeginGameStart()
{
	if (!GameStartFlow)
	{
		UE_LOG(LogTemp, Error, TEXT("GameStartWidget: GameStartFlow not available"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("GameStartWidget: Beginning game start flow"));

	// Start the flow
	GameStartFlow->BeginNewGameFlow();
}

void UGameStartWidget::OnPhaseChanged(EGameStartPhase NewPhase)
{
	UE_LOG(LogTemp, Log, TEXT("GameStartWidget: Phase changed to %s"),
		*UEnum::GetValueAsString(NewPhase));

	ShowPhase(NewPhase);
	UpdatePhaseDisplay(NewPhase);
}

void UGameStartWidget::OnGameStartComplete()
{
	UE_LOG(LogTemp, Log, TEXT("GameStartWidget: Game start complete!"));

	// TODO: Transition to main game
	// For now, just remove this widget
	RemoveFromParent();
}

void UGameStartWidget::ShowPhase(EGameStartPhase Phase)
{
	if (!PhaseWidgetSwitcher)
	{
		return;
	}

	// Switch to appropriate widget index
	switch (Phase)
	{
	case EGameStartPhase::RaceSelection:
		PhaseWidgetSwitcher->SetActiveWidgetIndex(0);
		break;

	case EGameStartPhase::TechPointAllocation:
		PhaseWidgetSwitcher->SetActiveWidgetIndex(1);
		// Initialize tech allocation widget with bonus points
		if (TechAllocationWidget && GameStartFlow)
		{
			TechAllocationWidget->Initialize(GameStartFlow->GetBonusTechPoints());
		}
		break;

	case EGameStartPhase::GameInitialization:
		PhaseWidgetSwitcher->SetActiveWidgetIndex(2);
		if (InitializationStatusText)
		{
			InitializationStatusText->SetText(FText::FromString(TEXT("Initializing game...")));
		}
		break;

	case EGameStartPhase::Complete:
		if (InitializationStatusText)
		{
			InitializationStatusText->SetText(FText::FromString(TEXT("Game ready!")));
		}
		break;

	default:
		break;
	}
}

void UGameStartWidget::UpdatePhaseDisplay(EGameStartPhase Phase)
{
	// Update phase name
	if (PhaseNameText)
	{
		FString PhaseName;
		switch (Phase)
		{
		case EGameStartPhase::RaceSelection:
			PhaseName = TEXT("Phase 1: Choose Your Species");
			break;
		case EGameStartPhase::TechPointAllocation:
			PhaseName = TEXT("Phase 2: Allocate Research Points");
			break;
		case EGameStartPhase::GameInitialization:
			PhaseName = TEXT("Phase 3: Preparing Your Journey");
			break;
		case EGameStartPhase::Complete:
			PhaseName = TEXT("Ready to Begin!");
			break;
		default:
			PhaseName = TEXT("");
			break;
		}

		PhaseNameText->SetText(FText::FromString(PhaseName));
	}

	// Update progress bar
	if (PhaseProgressBar)
	{
		float Progress = 0.0f;
		switch (Phase)
		{
		case EGameStartPhase::RaceSelection:
			Progress = 0.33f;
			break;
		case EGameStartPhase::TechPointAllocation:
			Progress = 0.66f;
			break;
		case EGameStartPhase::GameInitialization:
		case EGameStartPhase::Complete:
			Progress = 1.0f;
			break;
		default:
			Progress = 0.0f;
			break;
		}

		PhaseProgressBar->SetPercent(Progress);
	}
}

void UGameStartWidget::CreateRaceSelectionWidget()
{
	if (!RaceSelectionWidgetClass || !RaceSelectionSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameStartWidget: RaceSelectionWidgetClass or slot not set"));
		return;
	}

	RaceSelectionWidget = CreateWidget<URaceSelectionWidget>(this, RaceSelectionWidgetClass);
	if (RaceSelectionWidget)
	{
		// Bind to race selection event
		RaceSelectionWidget->OnRaceConfirmed.AddDynamic(this, &UGameStartWidget::OnRaceSelected);

		// Add to slot
		RaceSelectionSlot->ClearChildren();
		RaceSelectionSlot->AddChild(RaceSelectionWidget);

		UE_LOG(LogTemp, Log, TEXT("GameStartWidget: Created RaceSelectionWidget"));
	}
}

void UGameStartWidget::CreateTechAllocationWidget()
{
	if (!TechAllocationWidgetClass || !TechAllocationSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameStartWidget: TechAllocationWidgetClass or slot not set"));
		return;
	}

	TechAllocationWidget = CreateWidget<UTechPointAllocationWidget>(this, TechAllocationWidgetClass);
	if (TechAllocationWidget)
	{
		// Bind to completion event
		TechAllocationWidget->OnComplete.AddDynamic(this, &UGameStartWidget::OnTechAllocationComplete);

		// Add to slot
		TechAllocationSlot->ClearChildren();
		TechAllocationSlot->AddChild(TechAllocationWidget);

		UE_LOG(LogTemp, Log, TEXT("GameStartWidget: Created TechAllocationWidget"));
	}
}

void UGameStartWidget::OnRaceSelected(EPhylum SelectedPhylum, FString ColonyName)
{
	UE_LOG(LogTemp, Log, TEXT("GameStartWidget: Race selected - %s, Colony: %s"),
		*UEnum::GetValueAsString(SelectedPhylum), *ColonyName);

	if (GameStartFlow)
	{
		GameStartFlow->OnRaceSelected(SelectedPhylum, ColonyName);
	}
}

void UGameStartWidget::OnTechAllocationComplete()
{
	UE_LOG(LogTemp, Log, TEXT("GameStartWidget: Tech allocation complete"));

	if (GameStartFlow)
	{
		GameStartFlow->OnTechPointsAllocated();
	}
}
