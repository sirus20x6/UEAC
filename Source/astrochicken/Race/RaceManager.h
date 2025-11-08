// Source/astrochicken/Race/RaceManager.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RaceData.h"
#include "RaceManager.generated.h"

/**
 * Manages all race data, unlocks, and player race selection
 * Implemented as a GameInstanceSubsystem for persistence across level loads
 */
UCLASS()
class ASTROCHICKEN_API URaceManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Race data access
	UFUNCTION(BlueprintCallable, Category = "Race")
	const TArray<FRaceData>& GetAllRaces() const { return AllRaces; }

	UFUNCTION(BlueprintCallable, Category = "Race")
	TArray<FRaceData> GetUnlockedRaces() const;

	UFUNCTION(BlueprintCallable, Category = "Race")
	FRaceData GetRaceData(EPhylum Phylum) const;

	UFUNCTION(BlueprintCallable, Category = "Race")
	bool IsRaceUnlocked(EPhylum Phylum) const;

	// Player selection
	UFUNCTION(BlueprintCallable, Category = "Race")
	void SetPlayerRaceSelection(EPhylum Phylum, const FString& ColonyName);

	UFUNCTION(BlueprintCallable, Category = "Race")
	FPlayerRaceSelection GetPlayerRaceSelection() const { return PlayerSelection; }

	UFUNCTION(BlueprintCallable, Category = "Race")
	bool HasPlayerSelectedRace() const { return bHasSelection; }

	// Unlock management
	UFUNCTION(BlueprintCallable, Category = "Race")
	void UnlockRace(EPhylum Phylum);

	UFUNCTION(BlueprintCallable, Category = "Race")
	void ResetUnlocks();

	// Save/Load support
	UFUNCTION(BlueprintCallable, Category = "Race")
	void SaveUnlockState();

	UFUNCTION(BlueprintCallable, Category = "Race")
	void LoadUnlockState();

private:
	// Initialize all race data
	void InitializeRaceDatabase();

	// Helper to create race data
	FRaceData CreateRaceData(
		EPhylum Phylum,
		const FString& RaceName,
		const FString& PhylumName,
		const FString& Description,
		const FText& Lore,
		const FRaceAttributes& Attributes,
		const FTechBonuses& TechBonuses,
		bool bUnlocked = false
	);

	UPROPERTY()
	TArray<FRaceData> AllRaces;

	UPROPERTY()
	FPlayerRaceSelection PlayerSelection;

	UPROPERTY()
	bool bHasSelection = false;
};
