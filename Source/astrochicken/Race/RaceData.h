// Source/astrochicken/Race/RaceData.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "RaceData.generated.h"

/**
 * Enum defining all available phylums in the game
 */
UENUM(BlueprintType)
enum class EPhylum : uint8
{
	// Playable races (unlocked by default)
	Chordata UMETA(DisplayName = "Chordata (Vertebranes)"),
	Arthropoda UMETA(DisplayName = "Arthropoda (Chitinoids)"),
	Cnidaria UMETA(DisplayName = "Cnidaria (Medusians)"),
	Mollusca UMETA(DisplayName = "Mollusca (Cephalopods)"),

	// Unlockable races (future content)
	Echinodermata UMETA(DisplayName = "Echinodermata (Starforms)"),
	Annelida UMETA(DisplayName = "Annelida (Segmentines)"),
	Porifera UMETA(DisplayName = "Porifera (Spongians)"),
	Platyhelminthes UMETA(DisplayName = "Platyhelminthes (Flatworlds)"),
	Nematoda UMETA(DisplayName = "Nematoda (Threadlings)"),
	Fungi UMETA(DisplayName = "Fungi (Myconids)"),
	Plantae UMETA(DisplayName = "Plantae (Chlorophytes)")
};

/**
 * Race attributes that define a species' strengths and weaknesses
 */
USTRUCT(BlueprintType)
struct FRaceAttributes
{
	GENERATED_BODY()

	// Core survival attributes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float Adaptability = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float Resilience = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float Intelligence = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float Social = 50.0f;

	// Specialized attributes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float Efficiency = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float SpaceAdaptation = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float ResourceExtraction = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float TechAffinity = 50.0f;

	FRaceAttributes() = default;
};

/**
 * Tech point bonuses provided by each race
 * Maps to the 6 tech tree categories
 */
USTRUCT(BlueprintType)
struct FTechBonuses
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Bonuses")
	int32 Physics = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Bonuses")
	int32 Engineering = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Bonuses")
	int32 Energy = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Bonuses")
	int32 Computing = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Bonuses")
	int32 Biology = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Bonuses")
	int32 Sensors = 0;

	FTechBonuses() = default;
};

/**
 * Complete race definition
 */
USTRUCT(BlueprintType)
struct FRaceData
{
	GENERATED_BODY()

	// Identification
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	EPhylum Phylum = EPhylum::Chordata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FString RaceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FString PhylumName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FText Lore;

	// Visual representation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	TSoftObjectPtr<UTexture2D> RaceIcon;

	// Game mechanics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FRaceAttributes Attributes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FTechBonuses TechBonuses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	bool bIsUnlocked = false;

	// Unlock conditions (future feature)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FString UnlockCondition;

	FRaceData() = default;
};

/**
 * Player's selected race configuration
 */
USTRUCT(BlueprintType)
struct FPlayerRaceSelection
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	EPhylum SelectedPhylum = EPhylum::Chordata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	FString ColonyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Selection")
	FRaceData RaceData;

	FPlayerRaceSelection() = default;
};
