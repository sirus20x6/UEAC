// Source/astrochicken/GameStart/GameStartFlow.cpp

#include "GameStartFlow.h"
#include "../Race/RaceManager.h"
#include "../TechTree/TechTreeManager.h"
#include "../SaveSystem/SaveSystem.h"

void UGameStartFlow::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("GameStartFlow: Initializing"));

	// Get references to other subsystems
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		RaceManager = GameInstance->GetSubsystem<URaceManager>();
		TechTreeManager = GameInstance->GetSubsystem<UTechTreeManager>();
		SaveSystem = GameInstance->GetSubsystem<USaveSystem>();
	}

	CurrentPhase = EGameStartPhase::None;
}

void UGameStartFlow::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("GameStartFlow: Deinitialized"));
}

void UGameStartFlow::BeginNewGameFlow()
{
	UE_LOG(LogTemp, Log, TEXT("GameStartFlow: Beginning new game flow"));

	// Reset state
	CurrentPhase = EGameStartPhase::None;
	ColonyName.Empty();
	SelectedPhylum = EPhylum::Chordata;

	// Calculate bonus tech points
	BonusTechPoints = CalculateBonusTechPoints();

	// Start with race selection
	CurrentPhase = EGameStartPhase::RaceSelection;
	OnPhaseChanged.Broadcast(CurrentPhase);

	UE_LOG(LogTemp, Log, TEXT("GameStartFlow: Phase 1 - Race Selection"));
}

void UGameStartFlow::AdvanceToNextPhase()
{
	switch (CurrentPhase)
	{
	case EGameStartPhase::RaceSelection:
		CurrentPhase = EGameStartPhase::TechPointAllocation;
		UE_LOG(LogTemp, Log, TEXT("GameStartFlow: Phase 2 - Tech Point Allocation"));
		break;

	case EGameStartPhase::TechPointAllocation:
		CurrentPhase = EGameStartPhase::GameInitialization;
		UE_LOG(LogTemp, Log, TEXT("GameStartFlow: Phase 3 - Game Initialization"));
		InitializeGame();
		break;

	case EGameStartPhase::GameInitialization:
		CurrentPhase = EGameStartPhase::Complete;
		UE_LOG(LogTemp, Log, TEXT("GameStartFlow: Game start complete!"));
		OnGameStartComplete.Broadcast();
		break;

	default:
		UE_LOG(LogTemp, Warning, TEXT("GameStartFlow: Cannot advance from current phase"));
		return;
	}

	OnPhaseChanged.Broadcast(CurrentPhase);
}

void UGameStartFlow::OnRaceSelected(EPhylum InSelectedPhylum, const FString& InColonyName)
{
	if (CurrentPhase != EGameStartPhase::RaceSelection)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameStartFlow: Race selection called in wrong phase"));
		return;
	}

	SelectedPhylum = InSelectedPhylum;
	ColonyName = InColonyName;

	// Set the selection in RaceManager
	if (RaceManager)
	{
		RaceManager->SetPlayerRaceSelection(SelectedPhylum, ColonyName);
	}

	// Apply race bonuses to tech tree
	ApplyRaceBonusesToTechTree();

	UE_LOG(LogTemp, Log, TEXT("GameStartFlow: Race selected - %s, Colony: %s"),
		*UEnum::GetValueAsString(SelectedPhylum), *ColonyName);

	// Advance to tech allocation
	AdvanceToNextPhase();
}

void UGameStartFlow::OnTechPointsAllocated()
{
	if (CurrentPhase != EGameStartPhase::TechPointAllocation)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameStartFlow: Tech allocation called in wrong phase"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("GameStartFlow: Tech points allocated"));

	// Advance to game initialization
	AdvanceToNextPhase();
}

void UGameStartFlow::ApplyRaceBonusesToTechTree()
{
	if (!RaceManager || !TechTreeManager)
	{
		UE_LOG(LogTemp, Error, TEXT("GameStartFlow: Missing required subsystems"));
		return;
	}

	// Get race data
	FRaceData RaceData = RaceManager->GetRaceData(SelectedPhylum);

	// Apply race tech bonuses to tech tree
	TechTreeManager->InitializeTechPointsWithRaceBonus(RaceData.TechBonuses);

	UE_LOG(LogTemp, Log, TEXT("GameStartFlow: Applied race bonuses to tech tree"));
	UE_LOG(LogTemp, Log, TEXT("  - Physics: +%d"), RaceData.TechBonuses.Physics);
	UE_LOG(LogTemp, Log, TEXT("  - Engineering: +%d"), RaceData.TechBonuses.Engineering);
	UE_LOG(LogTemp, Log, TEXT("  - Energy: +%d"), RaceData.TechBonuses.Energy);
	UE_LOG(LogTemp, Log, TEXT("  - Computing: +%d"), RaceData.TechBonuses.Computing);
	UE_LOG(LogTemp, Log, TEXT("  - Biology: +%d"), RaceData.TechBonuses.Biology);
	UE_LOG(LogTemp, Log, TEXT("  - Sensors: +%d"), RaceData.TechBonuses.Sensors);
}

bool UGameStartFlow::InitializeGame()
{
	if (!SaveSystem || !RaceManager)
	{
		UE_LOG(LogTemp, Error, TEXT("GameStartFlow: Missing required subsystems for initialization"));
		return false;
	}

	// Get race name for save
	FRaceData RaceData = RaceManager->GetRaceData(SelectedPhylum);
	FString RaceName = RaceData.RaceName;

	// Create initial save
	if (!CreateInitialSave())
	{
		UE_LOG(LogTemp, Error, TEXT("GameStartFlow: Failed to create initial save"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("GameStartFlow: Game initialized successfully"));
	UE_LOG(LogTemp, Log, TEXT("  - Colony: %s"), *ColonyName);
	UE_LOG(LogTemp, Log, TEXT("  - Race: %s"), *RaceName);
	UE_LOG(LogTemp, Log, TEXT("  - Game UUID: %s"), *SaveSystem->GetCurrentGameUUID());

	return true;
}

bool UGameStartFlow::CreateInitialSave()
{
	if (!SaveSystem || !RaceManager)
	{
		return false;
	}

	// Get race name
	FRaceData RaceData = RaceManager->GetRaceData(SelectedPhylum);
	FString RaceName = RaceData.RaceName;

	// Create new game (this creates the git repo and initial commit)
	if (!SaveSystem->CreateNewGame(ColonyName, RaceName))
	{
		UE_LOG(LogTemp, Error, TEXT("GameStartFlow: SaveSystem failed to create new game: %s"),
			*SaveSystem->GetLastError());
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("GameStartFlow: Initial save created (NewGame checkpoint)"));
	return true;
}

int32 UGameStartFlow::CalculateBonusTechPoints()
{
	// Base bonus points
	int32 BaseBonus = 20;

	// Apply difficulty modifier (future feature)
	int32 AdjustedBonus = FMath::RoundToInt(BaseBonus * DifficultyMultiplier);

	return AdjustedBonus;
}
