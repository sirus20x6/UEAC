// Source/astrochicken/SaveSystem/SaveData.h

#pragma once

#include "CoreMinimal.h"
#include "SaveData.generated.h"

/**
 * Save type classification
 */
UENUM(BlueprintType)
enum class ESaveType : uint8
{
	Manual UMETA(DisplayName = "Manual Save"),
	AutoSave UMETA(DisplayName = "Auto Save"),
	Checkpoint UMETA(DisplayName = "Checkpoint")
};

/**
 * Checkpoint types for story progression
 */
UENUM(BlueprintType)
enum class ECheckpointType : uint8
{
	NewGame UMETA(DisplayName = "New Game"),
	Prologue UMETA(DisplayName = "Prologue"),
	Act1 UMETA(DisplayName = "Act 1"),
	Act2 UMETA(DisplayName = "Act 2"),
	Act3 UMETA(DisplayName = "Act 3"),
	Act4 UMETA(DisplayName = "Act 4"),
	Act5 UMETA(DisplayName = "Act 5"),
	Ending UMETA(DisplayName = "Ending")
};

/**
 * Save metadata stored in git commits
 */
USTRUCT(BlueprintType)
struct FSaveMetadata
{
	GENERATED_BODY()

	// Unique identifier for this game run
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Metadata")
	FString GameUUID;

	// Save information
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Metadata")
	ESaveType SaveType = ESaveType::Manual;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Metadata")
	FString SaveName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Metadata")
	FString SaveDescription;

	// Timestamp (ISO 8601 format)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Metadata")
	FString Timestamp;

	// Git information
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Metadata")
	FString CommitSHA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Metadata")
	FString BranchName;

	// Checkpoint info (if applicable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Metadata")
	bool bIsCheckpoint = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Metadata")
	ECheckpointType CheckpointType = ECheckpointType::NewGame;

	// Game state summary
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Metadata")
	FString ColonyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Metadata")
	FString RaceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Metadata")
	int32 PlaytimeSeconds = 0;

	FSaveMetadata() = default;
};

/**
 * Complete game state for serialization
 */
USTRUCT(BlueprintType)
struct FGameSaveData
{
	GENERATED_BODY()

	// Metadata
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data")
	FSaveMetadata Metadata;

	// Game state (will be serialized to separate JSON files)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data")
	FString RaceSelectionJSON;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data")
	FString TechTreeJSON;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data")
	FString ShipInfrastructureJSON;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data")
	FString PlayerProgressJSON;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Data")
	FString GameStateJSON;

	FGameSaveData() = default;
};

/**
 * Save tree node for visualization
 */
USTRUCT(BlueprintType)
struct FSaveTreeNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Tree")
	FSaveMetadata Metadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Tree")
	FString ParentCommitSHA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Tree")
	TArray<FString> ChildCommitSHAs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Tree")
	int32 SequenceNumber = 0;

	FSaveTreeNode() = default;
};

/**
 * Git repository configuration
 */
USTRUCT()
struct FGitRepositoryConfig
{
	GENERATED_BODY()

	// Repository path (e.g., "SaveGames/{GameUUID}")
	FString RepositoryPath;

	// Main branch name
	FString MainBranchName = TEXT("main");

	// Auto-save branch name
	FString AutoSaveBranchName = TEXT("auto-save");

	// Current branch
	FString CurrentBranch;

	// Number of auto-saves to keep
	int32 MaxAutoSaves = 3;

	FGitRepositoryConfig() = default;
};
