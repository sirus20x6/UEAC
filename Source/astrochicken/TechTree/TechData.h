// Source/astrochicken/TechTree/TechData.h

#pragma once

#include "CoreMinimal.h"
#include "TechData.generated.h"

/**
 * Tech categories available in the game
 */
UENUM(BlueprintType)
enum class ETechCategory : uint8
{
	Physics UMETA(DisplayName = "Physics"),
	Engineering UMETA(DisplayName = "Engineering"),
	Energy UMETA(DisplayName = "Energy"),
	Computing UMETA(DisplayName = "Computing"),
	Biology UMETA(DisplayName = "Biology"),
	Sensors UMETA(DisplayName = "Sensors")
};

/**
 * Tech tier/level system
 */
UENUM(BlueprintType)
enum class ETechTier : uint8
{
	Tier1 UMETA(DisplayName = "Tier 1 - Basic"),
	Tier2 UMETA(DisplayName = "Tier 2 - Advanced"),
	Tier3 UMETA(DisplayName = "Tier 3 - Superior"),
	Tier4 UMETA(DisplayName = "Tier 4 - Cutting Edge"),
	Tier5 UMETA(DisplayName = "Tier 5 - Revolutionary")
};

/**
 * Prerequisites for unlocking a tech
 */
USTRUCT(BlueprintType)
struct FTechPrerequisite
{
	GENERATED_BODY()

	// Category required
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisite")
	ETechCategory Category = ETechCategory::Physics;

	// Minimum points required in this category
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisite")
	int32 MinimumPoints = 0;

	// Specific tech ID that must be unlocked (optional)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisite")
	FString RequiredTechID;

	FTechPrerequisite() = default;

	FTechPrerequisite(ETechCategory InCategory, int32 InMinPoints)
		: Category(InCategory), MinimumPoints(InMinPoints) {}
};

/**
 * A single technology node in the tech tree
 */
USTRUCT(BlueprintType)
struct FTechNode
{
	GENERATED_BODY()

	// Unique identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech")
	FString TechID;

	// Display information
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech")
	FString TechName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech")
	FText DetailedEffect;

	// Classification
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech")
	ETechCategory Category = ETechCategory::Physics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech")
	ETechTier Tier = ETechTier::Tier1;

	// Prerequisites to unlock this tech
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech")
	TArray<FTechPrerequisite> Prerequisites;

	// Cost to research (if using research points separately from allocation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech")
	int32 ResearchCost = 0;

	// Game effects (to be expanded based on gameplay needs)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech")
	TMap<FString, float> GameplayModifiers;

	// Unlock state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech")
	bool bIsUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech")
	bool bIsResearching = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech")
	float ResearchProgress = 0.0f;

	FTechNode() = default;
};

/**
 * Tech points allocated to each category
 */
USTRUCT(BlueprintType)
struct FTechPoints
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Points")
	int32 Physics = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Points")
	int32 Engineering = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Points")
	int32 Energy = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Points")
	int32 Computing = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Points")
	int32 Biology = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Points")
	int32 Sensors = 10;

	FTechPoints() = default;

	// Get points for a specific category
	int32 GetPoints(ETechCategory Category) const
	{
		switch (Category)
		{
		case ETechCategory::Physics: return Physics;
		case ETechCategory::Engineering: return Engineering;
		case ETechCategory::Energy: return Energy;
		case ETechCategory::Computing: return Computing;
		case ETechCategory::Biology: return Biology;
		case ETechCategory::Sensors: return Sensors;
		default: return 0;
		}
	}

	// Set points for a specific category
	void SetPoints(ETechCategory Category, int32 Points)
	{
		switch (Category)
		{
		case ETechCategory::Physics: Physics = Points; break;
		case ETechCategory::Engineering: Engineering = Points; break;
		case ETechCategory::Energy: Energy = Points; break;
		case ETechCategory::Computing: Computing = Points; break;
		case ETechCategory::Biology: Biology = Points; break;
		case ETechCategory::Sensors: Sensors = Points; break;
		}
	}

	// Add points to a category
	void AddPoints(ETechCategory Category, int32 Points)
	{
		SetPoints(Category, GetPoints(Category) + Points);
	}

	// Get total points across all categories
	int32 GetTotalPoints() const
	{
		return Physics + Engineering + Energy + Computing + Biology + Sensors;
	}
};

/**
 * Player's tech tree state
 */
USTRUCT(BlueprintType)
struct FPlayerTechState
{
	GENERATED_BODY()

	// Current tech points allocated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech State")
	FTechPoints AllocatedPoints;

	// Unlocked technologies
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech State")
	TArray<FString> UnlockedTechs;

	// Currently researching tech (for active research system)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech State")
	FString CurrentlyResearchingTech;

	FPlayerTechState() = default;
};
