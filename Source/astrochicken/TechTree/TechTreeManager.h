// Source/astrochicken/TechTree/TechTreeManager.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TechData.h"
#include "TechTreeManager.generated.h"

/**
 * Manages the tech tree, tech progression, and tech point allocation
 * Implemented as a GameInstanceSubsystem for persistence
 */
UCLASS()
class ASTROCHICKEN_API UTechTreeManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tech tree access
	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	const TArray<FTechNode>& GetAllTechs() const { return AllTechs; }

	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	TArray<FTechNode> GetTechsByCategory(ETechCategory Category) const;

	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	TArray<FTechNode> GetTechsByTier(ETechTier Tier) const;

	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	FTechNode GetTechByID(const FString& TechID) const;

	// Tech unlocking and prerequisites
	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	bool CanUnlockTech(const FString& TechID) const;

	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	bool UnlockTech(const FString& TechID);

	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	bool IsTechUnlocked(const FString& TechID) const;

	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	TArray<FTechNode> GetAvailableTechs() const;

	// Tech point management
	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	FTechPoints GetTechPoints() const { return PlayerTechState.AllocatedPoints; }

	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	int32 GetCategoryPoints(ETechCategory Category) const;

	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	void SetCategoryPoints(ETechCategory Category, int32 Points);

	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	void AddCategoryPoints(ETechCategory Category, int32 Points);

	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	int32 GetTotalTechPoints() const { return PlayerTechState.AllocatedPoints.GetTotalPoints(); }

	// Initialize tech points with race bonuses
	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	void InitializeTechPointsWithRaceBonus(const struct FTechBonuses& RaceBonuses);

	// Bonus points allocation (during game start)
	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	void AllocateBonusPoints(ETechCategory Category, int32 Points, int32& RemainingBonusPoints);

	// Research system (for active gameplay)
	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	bool StartResearch(const FString& TechID);

	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	void UpdateResearch(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	FString GetCurrentResearch() const { return PlayerTechState.CurrentlyResearchingTech; }

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	void SaveTechState();

	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	void LoadTechState();

	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	FPlayerTechState GetPlayerTechState() const { return PlayerTechState; }

	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	void SetPlayerTechState(const FPlayerTechState& NewState);

	// Reset
	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	void ResetTechTree();

private:
	// Initialize the tech database
	void InitializeTechDatabase();

	// Helper to create tech nodes
	FTechNode CreateTechNode(
		const FString& TechID,
		const FString& TechName,
		ETechCategory Category,
		ETechTier Tier,
		const FText& Description,
		const FText& DetailedEffect,
		const TArray<FTechPrerequisite>& Prerequisites = TArray<FTechPrerequisite>()
	);

	// Check if prerequisites are met
	bool CheckPrerequisites(const TArray<FTechPrerequisite>& Prerequisites) const;

	UPROPERTY()
	TArray<FTechNode> AllTechs;

	UPROPERTY()
	FPlayerTechState PlayerTechState;

	// Base tech points (before race bonuses)
	const int32 BaseTechPoints = 10;
};
