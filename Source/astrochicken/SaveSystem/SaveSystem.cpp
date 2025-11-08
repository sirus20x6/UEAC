// Source/astrochicken/SaveSystem/SaveSystem.cpp

#include "SaveSystem.h"
#include "GitRepository.h"
#include "../Race/RaceManager.h"
#include "../TechTree/TechTreeManager.h"
#include "../Infrastructure/ShipInfrastructureManager.h"
#include "Misc/Paths.h"
#include "Misc/Guid.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void USaveSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("SaveSystem: Initializing"));

	// Get references to other subsystems
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		RaceManager = GameInstance->GetSubsystem<URaceManager>();
		TechTreeManager = GameInstance->GetSubsystem<UTechTreeManager>();
		ShipInfrastructureManager = GameInstance->GetSubsystem<UShipInfrastructureManager>();
	}

	// Create git repository instance
	GitRepo = MakeShared<FGitRepository>();
}

void USaveSystem::Deinitialize()
{
	Super::Deinitialize();

	// Cleanup
	GitRepo.Reset();

	UE_LOG(LogTemp, Log, TEXT("SaveSystem: Deinitialized"));
}

bool USaveSystem::CreateNewGame(const FString& ColonyName, const FString& RaceName)
{
	// Generate new UUID for this game run
	CurrentGameUUID = GenerateGameUUID();

	UE_LOG(LogTemp, Log, TEXT("SaveSystem: Creating new game with UUID: %s"), *CurrentGameUUID);

	// Initialize git repository for this game
	if (!InitializeRepository())
	{
		return false;
	}

	// Setup initial metadata
	CurrentSaveMetadata.GameUUID = CurrentGameUUID;
	CurrentSaveMetadata.SaveType = ESaveType::Checkpoint;
	CurrentSaveMetadata.SaveName = TEXT("New Game");
	CurrentSaveMetadata.SaveDescription = TEXT("Game started");
	CurrentSaveMetadata.Timestamp = GetCurrentTimestamp();
	CurrentSaveMetadata.bIsCheckpoint = true;
	CurrentSaveMetadata.CheckpointType = ECheckpointType::NewGame;
	CurrentSaveMetadata.ColonyName = ColonyName;
	CurrentSaveMetadata.RaceName = RaceName;
	CurrentSaveMetadata.PlaytimeSeconds = 0;
	CurrentSaveMetadata.BranchName = TEXT("main");

	// Create initial checkpoint save
	FString CommitSHA;
	if (!CommitGameState(TEXT("NewGame: ") + ColonyName, ESaveType::Checkpoint, CommitSHA))
	{
		return false;
	}

	// Tag as NewGame checkpoint
	FString TagName = FString::Printf(TEXT("checkpoint/NewGame"));
	GitRepo->CreateTag(TagName, CommitSHA, TEXT("New Game checkpoint"));

	CurrentSaveMetadata.CommitSHA = CommitSHA;

	UE_LOG(LogTemp, Log, TEXT("SaveSystem: New game created successfully"));
	return true;
}

bool USaveSystem::InitializeRepository()
{
	if (!GitRepo.IsValid())
	{
		SetError(TEXT("Git repository not initialized"));
		return false;
	}

	FString RepoPath = GetGameRepositoryPath();

	// Create the repository
	if (!GitRepo->CreateRepository(RepoPath))
	{
		SetError(FString::Printf(TEXT("Failed to create repository: %s"), *GitRepo->GetLastError()));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("SaveSystem: Repository initialized at %s"), *RepoPath);
	return true;
}

bool USaveSystem::ManualSave(const FString& SaveName, const FString& Description)
{
	if (!HasActiveGame())
	{
		SetError(TEXT("No active game to save"));
		return false;
	}

	CurrentSaveMetadata.SaveType = ESaveType::Manual;
	CurrentSaveMetadata.SaveName = SaveName;
	CurrentSaveMetadata.SaveDescription = Description;
	CurrentSaveMetadata.Timestamp = GetCurrentTimestamp();
	CurrentSaveMetadata.bIsCheckpoint = false;

	FString Message = FString::Printf(TEXT("Manual Save: %s"), *SaveName);
	FString CommitSHA;

	if (!CommitGameState(Message, ESaveType::Manual, CommitSHA))
	{
		return false;
	}

	CurrentSaveMetadata.CommitSHA = CommitSHA;

	UE_LOG(LogTemp, Log, TEXT("SaveSystem: Manual save created: %s"), *SaveName);
	return true;
}

bool USaveSystem::AutoSave()
{
	if (!HasActiveGame())
	{
		SetError(TEXT("No active game to save"));
		return false;
	}

	// Switch to auto-save branch if not already on it
	FString CurrentBranch;
	if (GitRepo->GetCurrentBranch(CurrentBranch))
	{
		if (CurrentBranch != TEXT("auto-save"))
		{
			// Create auto-save branch if it doesn't exist
			GitRepo->CreateBranch(TEXT("auto-save"));
			GitRepo->SwitchBranch(TEXT("auto-save"));
		}
	}

	CurrentSaveMetadata.SaveType = ESaveType::AutoSave;
	CurrentSaveMetadata.SaveName = TEXT("Auto Save");
	CurrentSaveMetadata.SaveDescription = TEXT("Automatic save");
	CurrentSaveMetadata.Timestamp = GetCurrentTimestamp();
	CurrentSaveMetadata.bIsCheckpoint = false;
	CurrentSaveMetadata.BranchName = TEXT("auto-save");

	FString Message = FString::Printf(TEXT("Auto Save - %s"), *GetCurrentTimestamp());
	FString CommitSHA;

	if (!CommitGameState(Message, ESaveType::AutoSave, CommitSHA))
	{
		return false;
	}

	CurrentSaveMetadata.CommitSHA = CommitSHA;

	// Cleanup old auto-saves
	CleanupOldAutoSaves();

	UE_LOG(LogTemp, Log, TEXT("SaveSystem: Auto save created"));
	return true;
}

bool USaveSystem::CheckpointSave(ECheckpointType CheckpointType)
{
	if (!HasActiveGame())
	{
		SetError(TEXT("No active game to save"));
		return false;
	}

	// Get checkpoint name
	FString CheckpointName;
	switch (CheckpointType)
	{
	case ECheckpointType::NewGame: CheckpointName = TEXT("New Game"); break;
	case ECheckpointType::Prologue: CheckpointName = TEXT("Prologue"); break;
	case ECheckpointType::Act1: CheckpointName = TEXT("Act 1"); break;
	case ECheckpointType::Act2: CheckpointName = TEXT("Act 2"); break;
	case ECheckpointType::Act3: CheckpointName = TEXT("Act 3"); break;
	case ECheckpointType::Act4: CheckpointName = TEXT("Act 4"); break;
	case ECheckpointType::Act5: CheckpointName = TEXT("Act 5"); break;
	case ECheckpointType::Ending: CheckpointName = TEXT("Ending"); break;
	}

	CurrentSaveMetadata.SaveType = ESaveType::Checkpoint;
	CurrentSaveMetadata.SaveName = CheckpointName;
	CurrentSaveMetadata.SaveDescription = FString::Printf(TEXT("Story checkpoint: %s"), *CheckpointName);
	CurrentSaveMetadata.Timestamp = GetCurrentTimestamp();
	CurrentSaveMetadata.bIsCheckpoint = true;
	CurrentSaveMetadata.CheckpointType = CheckpointType;

	FString Message = FString::Printf(TEXT("Checkpoint: %s"), *CheckpointName);
	FString CommitSHA;

	if (!CommitGameState(Message, ESaveType::Checkpoint, CommitSHA))
	{
		return false;
	}

	// Tag the checkpoint
	FString TagName = FString::Printf(TEXT("checkpoint/%s"), *CheckpointName.Replace(TEXT(" "), TEXT("")));
	GitRepo->CreateTag(TagName, CommitSHA, Message);

	CurrentSaveMetadata.CommitSHA = CommitSHA;

	UE_LOG(LogTemp, Log, TEXT("SaveSystem: Checkpoint save created: %s"), *CheckpointName);
	return true;
}

bool USaveSystem::CommitGameState(const FString& Message, ESaveType SaveType, FString& OutCommitSHA)
{
	if (!GitRepo.IsValid())
	{
		SetError(TEXT("Git repository not valid"));
		return false;
	}

	// Serialize game state
	FGameSaveData SaveData;
	if (!SerializeGameState(SaveData))
	{
		return false;
	}

	// Write game state files
	if (!WriteGameStateFiles(SaveData))
	{
		return false;
	}

	// Stage all files
	if (!GitRepo->StageAllFiles())
	{
		SetError(FString::Printf(TEXT("Failed to stage files: %s"), *GitRepo->GetLastError()));
		return false;
	}

	// Create commit
	if (!GitRepo->CreateCommit(Message, TEXT("AstroChicken"), TEXT("save@game.local"), OutCommitSHA))
	{
		SetError(FString::Printf(TEXT("Failed to create commit: %s"), *GitRepo->GetLastError()));
		return false;
	}

	return true;
}

bool USaveSystem::SerializeGameState(FGameSaveData& OutSaveData)
{
	OutSaveData.Metadata = CurrentSaveMetadata;

	// Serialize race selection
	if (RaceManager)
	{
		FPlayerRaceSelection RaceSelection = RaceManager->GetPlayerRaceSelection();

		TSharedPtr<FJsonObject> RaceJson = MakeShareable(new FJsonObject);
		RaceJson->SetStringField(TEXT("ColonyName"), RaceSelection.ColonyName);
		RaceJson->SetNumberField(TEXT("SelectedPhylum"), static_cast<int32>(RaceSelection.SelectedPhylum));

		FString RaceSelectionString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RaceSelectionString);
		FJsonSerializer::Serialize(RaceJson.ToSharedRef(), Writer);

		OutSaveData.RaceSelectionJSON = RaceSelectionString;
	}

	// Serialize tech tree
	if (TechTreeManager)
	{
		FPlayerTechState TechState = TechTreeManager->GetPlayerTechState();

		TSharedPtr<FJsonObject> TechJson = MakeShareable(new FJsonObject);
		TechJson->SetNumberField(TEXT("Physics"), TechState.AllocatedPoints.Physics);
		TechJson->SetNumberField(TEXT("Engineering"), TechState.AllocatedPoints.Engineering);
		TechJson->SetNumberField(TEXT("Energy"), TechState.AllocatedPoints.Energy);
		TechJson->SetNumberField(TEXT("Computing"), TechState.AllocatedPoints.Computing);
		TechJson->SetNumberField(TEXT("Biology"), TechState.AllocatedPoints.Biology);
		TechJson->SetNumberField(TEXT("Sensors"), TechState.AllocatedPoints.Sensors);

		// Serialize unlocked techs array
		TArray<TSharedPtr<FJsonValue>> UnlockedArray;
		for (const FString& TechID : TechState.UnlockedTechs)
		{
			UnlockedArray.Add(MakeShareable(new FJsonValueString(TechID)));
		}
		TechJson->SetArrayField(TEXT("UnlockedTechs"), UnlockedArray);

		FString TechTreeString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&TechTreeString);
		FJsonSerializer::Serialize(TechJson.ToSharedRef(), Writer);

		OutSaveData.TechTreeJSON = TechTreeString;
	}

	// Serialize ship infrastructure
	if (ShipInfrastructureManager)
	{
		OutSaveData.ShipInfrastructureJSON = ShipInfrastructureManager->SerializeToJSON();
	}
	else
	{
		OutSaveData.ShipInfrastructureJSON = TEXT("{}"); // Empty object if no ship data
	}

	// TODO: Serialize other game systems (journey, encounters, colonies, etc.)

	OutSaveData.PlayerProgressJSON = TEXT("{}"); // Placeholder
	OutSaveData.GameStateJSON = TEXT("{}"); // Placeholder

	return true;
}

bool USaveSystem::WriteGameStateFiles(const FGameSaveData& SaveData)
{
	FString WorkDir;
	if (!GitRepo->GetWorkingDirectoryPath(WorkDir))
	{
		SetError(TEXT("Failed to get working directory"));
		return false;
	}

	// Write metadata
	FString MetadataPath = FPaths::Combine(WorkDir, TEXT("Metadata.json"));
	if (!FFileHelper::SaveStringToFile(SaveData.Metadata.SaveName, *MetadataPath))
	{
		SetError(TEXT("Failed to write metadata"));
		return false;
	}

	// Write race selection
	FString RacePath = FPaths::Combine(WorkDir, TEXT("RaceSelection.json"));
	if (!FFileHelper::SaveStringToFile(SaveData.RaceSelectionJSON, *RacePath))
	{
		SetError(TEXT("Failed to write race selection"));
		return false;
	}

	// Write tech tree
	FString TechPath = FPaths::Combine(WorkDir, TEXT("TechTree.json"));
	if (!FFileHelper::SaveStringToFile(SaveData.TechTreeJSON, *TechPath))
	{
		SetError(TEXT("Failed to write tech tree"));
		return false;
	}

	// Write ship infrastructure
	FString ShipPath = FPaths::Combine(WorkDir, TEXT("ShipInfrastructure.json"));
	if (!FFileHelper::SaveStringToFile(SaveData.ShipInfrastructureJSON, *ShipPath))
	{
		SetError(TEXT("Failed to write ship infrastructure"));
		return false;
	}

	// Write player progress
	FString ProgressPath = FPaths::Combine(WorkDir, TEXT("PlayerProgress.json"));
	if (!FFileHelper::SaveStringToFile(SaveData.PlayerProgressJSON, *ProgressPath))
	{
		SetError(TEXT("Failed to write player progress"));
		return false;
	}

	// Write game state
	FString StatePath = FPaths::Combine(WorkDir, TEXT("GameState.json"));
	if (!FFileHelper::SaveStringToFile(SaveData.GameStateJSON, *StatePath))
	{
		SetError(TEXT("Failed to write game state"));
		return false;
	}

	return true;
}

FString USaveSystem::GenerateGameUUID()
{
	FGuid GUID = FGuid::NewGuid();
	return GUID.ToString(EGuidFormats::DigitsWithHyphensLower);
}

FString USaveSystem::GetCurrentTimestamp()
{
	return FDateTime::UtcNow().ToIso8601();
}

FString USaveSystem::GetGameRepositoryPath() const
{
	FString ProjectDir = FPaths::ProjectDir();
	return FPaths::Combine(ProjectDir, SaveGamesBasePath, CurrentGameUUID);
}

FString USaveSystem::GetSaveDirectory() const
{
	return FPaths::Combine(FPaths::ProjectDir(), SaveGamesBasePath);
}

bool USaveSystem::CleanupOldAutoSaves()
{
	// TODO: Implement pruning of old auto-save commits
	// Keep only the last MaxAutoSaves commits on the auto-save branch
	return true;
}

void USaveSystem::SetError(const FString& Error)
{
	LastError = Error;
	UE_LOG(LogTemp, Error, TEXT("SaveSystem Error: %s"), *Error);
}

// Stub implementations for load and query operations
bool USaveSystem::LoadSave(const FString& CommitSHA)
{
	SetError(TEXT("LoadSave not yet implemented"));
	return false;
}

bool USaveSystem::LoadLatestSave()
{
	SetError(TEXT("LoadLatestSave not yet implemented"));
	return false;
}

bool USaveSystem::LoadCheckpoint(ECheckpointType CheckpointType)
{
	SetError(TEXT("LoadCheckpoint not yet implemented"));
	return false;
}

bool USaveSystem::DeserializeGameState(const FGameSaveData& SaveData)
{
	SetError(TEXT("DeserializeGameState not yet implemented"));
	return false;
}

bool USaveSystem::ReadGameStateFiles(FGameSaveData& OutSaveData)
{
	SetError(TEXT("ReadGameStateFiles not yet implemented"));
	return false;
}

bool USaveSystem::CreateSaveBranch(const FString& BranchName, const FString& FromCommitSHA)
{
	SetError(TEXT("CreateSaveBranch not yet implemented"));
	return false;
}

bool USaveSystem::SwitchToSaveBranch(const FString& BranchName)
{
	SetError(TEXT("SwitchToSaveBranch not yet implemented"));
	return false;
}

bool USaveSystem::GetSaveList(TArray<FSaveMetadata>& OutSaves)
{
	SetError(TEXT("GetSaveList not yet implemented"));
	return false;
}

bool USaveSystem::GetSaveTree(TArray<FSaveTreeNode>& OutTree)
{
	SetError(TEXT("GetSaveTree not yet implemented"));
	return false;
}

bool USaveSystem::GetSaveMetadata(const FString& CommitSHA, FSaveMetadata& OutMetadata)
{
	SetError(TEXT("GetSaveMetadata not yet implemented"));
	return false;
}

bool USaveSystem::SaveMetadata(const FSaveMetadata& Metadata)
{
	SetError(TEXT("SaveMetadata not yet implemented"));
	return false;
}

bool USaveSystem::LoadMetadata(FSaveMetadata& OutMetadata)
{
	SetError(TEXT("LoadMetadata not yet implemented"));
	return false;
}
