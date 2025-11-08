// Source/astrochicken/GameStart/GameStartFlow.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../Race/RaceData.h"
#include "GameStartFlow.generated.h"

class URaceManager;
class UTechTreeManager;
class USaveSystem;

/**
 * Game start phases
 */
UENUM(BlueprintType)
enum class EGameStartPhase : uint8
{
	None UMETA(DisplayName = "None"),
	RaceSelection UMETA(DisplayName = "Race Selection"),
	TechPointAllocation UMETA(DisplayName = "Tech Point Allocation"),
	GameInitialization UMETA(DisplayName = "Game Initialization"),
	Complete UMETA(DisplayName = "Complete")
};

/**
 * Manages the game start flow sequence
 * Coordinates race selection, tech allocation, and game initialization
 */
UCLASS()
class ASTROCHICKEN_API UGameStartFlow : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Start the game creation flow
	UFUNCTION(BlueprintCallable, Category = "Game Start")
	void BeginNewGameFlow();

	// Phase management
	UFUNCTION(BlueprintCallable, Category = "Game Start")
	void AdvanceToNextPhase();

	UFUNCTION(BlueprintCallable, Category = "Game Start")
	EGameStartPhase GetCurrentPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintCallable, Category = "Game Start")
	bool IsFlowActive() const { return CurrentPhase != EGameStartPhase::None && CurrentPhase != EGameStartPhase::Complete; }

	// Phase completion callbacks
	UFUNCTION(BlueprintCallable, Category = "Game Start")
	void OnRaceSelected(EPhylum SelectedPhylum, const FString& ColonyName);

	UFUNCTION(BlueprintCallable, Category = "Game Start")
	void OnTechPointsAllocated();

	// Game data access
	UFUNCTION(BlueprintCallable, Category = "Game Start")
	FString GetColonyName() const { return ColonyName; }

	UFUNCTION(BlueprintCallable, Category = "Game Start")
	EPhylum GetSelectedPhylum() const { return SelectedPhylum; }

	UFUNCTION(BlueprintCallable, Category = "Game Start")
	int32 GetBonusTechPoints() const { return BonusTechPoints; }

	// Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, EGameStartPhase, NewPhase);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStartComplete);

	UPROPERTY(BlueprintAssignable, Category = "Game Start")
	FOnPhaseChanged OnPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Game Start")
	FOnGameStartComplete OnGameStartComplete;

private:
	// Initialize game with all selected options
	bool InitializeGame();

	// Apply race bonuses to tech tree
	void ApplyRaceBonusesToTechTree();

	// Create initial save
	bool CreateInitialSave();

	// Calculate bonus tech points based on difficulty
	int32 CalculateBonusTechPoints();

	UPROPERTY()
	URaceManager* RaceManager;

	UPROPERTY()
	UTechTreeManager* TechTreeManager;

	UPROPERTY()
	USaveSystem* SaveSystem;

	// Current flow state
	UPROPERTY()
	EGameStartPhase CurrentPhase = EGameStartPhase::None;

	// Player selections
	UPROPERTY()
	FString ColonyName;

	UPROPERTY()
	EPhylum SelectedPhylum = EPhylum::Chordata;

	UPROPERTY()
	int32 BonusTechPoints = 20;

	// Difficulty modifier (future feature)
	UPROPERTY()
	float DifficultyMultiplier = 1.0f;
};
