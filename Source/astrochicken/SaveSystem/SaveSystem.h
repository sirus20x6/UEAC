// Source/astrochicken/SaveSystem/SaveSystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveData.h"
#include "SaveSystem.generated.h"

class FGitRepository;
class URaceManager;
class UTechTreeManager;

/**
 * Save system using git for version control and branching
 * Manages all save operations, load operations, and save tree visualization
 */
UCLASS()
class ASTROCHICKEN_API USaveSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// New game creation
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool CreateNewGame(const FString& ColonyName, const FString& RaceName);

	UFUNCTION(BlueprintCallable, Category = "Save System")
	FString GetCurrentGameUUID() const { return CurrentGameUUID; }

	// Save operations
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool ManualSave(const FString& SaveName, const FString& Description = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool AutoSave();

	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool CheckpointSave(ECheckpointType CheckpointType);

	// Load operations
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool LoadSave(const FString& CommitSHA);

	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool LoadLatestSave();

	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool LoadCheckpoint(ECheckpointType CheckpointType);

	// Branch operations (for save tree)
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool CreateSaveBranch(const FString& BranchName, const FString& FromCommitSHA);

	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool SwitchToSaveBranch(const FString& BranchName);

	// Query operations
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool GetSaveList(TArray<FSaveMetadata>& OutSaves);

	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool GetSaveTree(TArray<FSaveTreeNode>& OutTree);

	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool GetSaveMetadata(const FString& CommitSHA, FSaveMetadata& OutMetadata);

	// Utility
	UFUNCTION(BlueprintCallable, Category = "Save System")
	FString GetSaveDirectory() const;

	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool HasActiveGame() const { return !CurrentGameUUID.IsEmpty(); }

	// Error handling
	UFUNCTION(BlueprintCallable, Category = "Save System")
	FString GetLastError() const { return LastError; }

private:
	// Internal operations
	bool InitializeRepository();
	bool SerializeGameState(FGameSaveData& OutSaveData);
	bool DeserializeGameState(const FGameSaveData& SaveData);
	bool WriteGameStateFiles(const FGameSaveData& SaveData);
	bool ReadGameStateFiles(FGameSaveData& OutSaveData);
	FString GenerateGameUUID();
	FString GetCurrentTimestamp();
	FString GetGameRepositoryPath() const;

	// Git operations wrapper
	bool CommitGameState(const FString& Message, ESaveType SaveType, FString& OutCommitSHA);

	// Metadata management
	bool SaveMetadata(const FSaveMetadata& Metadata);
	bool LoadMetadata(FSaveMetadata& OutMetadata);

	// Auto-save management
	bool CleanupOldAutoSaves();

	void SetError(const FString& Error);

	// Git repository instance
	TSharedPtr<FGitRepository> GitRepo;

	// Current game state
	UPROPERTY()
	FString CurrentGameUUID;

	UPROPERTY()
	FSaveMetadata CurrentSaveMetadata;

	// References to other subsystems
	UPROPERTY()
	URaceManager* RaceManager;

	UPROPERTY()
	UTechTreeManager* TechTreeManager;

	// Error tracking
	FString LastError;

	// Configuration
	FString SaveGamesBasePath = TEXT("SaveGames");
	int32 MaxAutoSaves = 3;
	int32 PlaytimeSeconds = 0;
};
