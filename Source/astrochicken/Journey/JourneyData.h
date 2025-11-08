// Source/astrochicken/Journey/JourneyData.h

#pragma once

#include "CoreMinimal.h"
#include "JourneyData.generated.h"

/**
 * Types of star systems
 */
UENUM(BlueprintType)
enum class EStarType : uint8
{
	MainSequence UMETA(DisplayName = "Main Sequence Star"),
	RedGiant UMETA(DisplayName = "Red Giant"),
	WhiteDwarf UMETA(DisplayName = "White Dwarf"),
	Neutron UMETA(DisplayName = "Neutron Star"),
	BinarySystem UMETA(DisplayName = "Binary System"),
	Nebula UMETA(DisplayName = "Nebula"),
	DeepSpace UMETA(DisplayName = "Deep Space")
};

/**
 * Star system attributes
 */
UENUM(BlueprintType)
enum class ESystemAttribute : uint8
{
	None UMETA(DisplayName = "None"),
	HasStarport UMETA(DisplayName = "Has Starport"),
	HasColony UMETA(DisplayName = "Has Colony"),
	ResourceRich UMETA(DisplayName = "Resource Rich"),
	Hazardous UMETA(DisplayName = "Hazardous"),
	PirateActivity UMETA(DisplayName = "Pirate Activity"),
	SalvageOpportunity UMETA(DisplayName = "Salvage Opportunity"),
	TradeHub UMETA(DisplayName = "Trade Hub")
};

/**
 * Travel status
 */
UENUM(BlueprintType)
enum class ETravelStatus : uint8
{
	Docked UMETA(DisplayName = "Docked"),
	PreparingJump UMETA(DisplayName = "Preparing Jump"),
	InTransit UMETA(DisplayName = "In Transit"),
	Arrived UMETA(DisplayName = "Arrived"),
	Emergency UMETA(DisplayName = "Emergency Stop")
};

/**
 * Star system definition
 */
USTRUCT(BlueprintType)
struct FStarSystem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star System")
	FString SystemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star System")
	FString SystemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star System")
	EStarType StarType = EStarType::MainSequence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star System")
	FVector Position = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star System")
	TArray<ESystemAttribute> Attributes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star System")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star System")
	int32 PlanetCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star System")
	bool bIsDiscovered = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star System")
	bool bIsVisited = false;

	// Resources available for refueling/resupply
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star System")
	float FuelAvailable = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Star System")
	float SuppliesAvailable = 0.0f;

	FStarSystem() = default;

	bool HasAttribute(ESystemAttribute Attribute) const
	{
		return Attributes.Contains(Attribute);
	}

	float DistanceTo(const FStarSystem& Other) const
	{
		return FVector::Dist(Position, Other.Position);
	}
};

/**
 * Waypoint in a route
 */
USTRUCT(BlueprintType)
struct FWaypoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	FString SystemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	int32 SequenceNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	bool bIsCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	FDateTime ArrivalTime;

	FWaypoint() = default;
};

/**
 * Planned route through multiple star systems
 */
USTRUCT(BlueprintType)
struct FRoute
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	FString RouteName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	TArray<FWaypoint> Waypoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float TotalDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float EstimatedFuelCost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
	float EstimatedTravelTime = 0.0f;

	FRoute() = default;

	int32 GetCurrentWaypointIndex() const
	{
		for (int32 i = 0; i < Waypoints.Num(); ++i)
		{
			if (!Waypoints[i].bIsCompleted)
			{
				return i;
			}
		}
		return Waypoints.Num() - 1;
	}

	bool IsComplete() const
	{
		for (const FWaypoint& Waypoint : Waypoints)
		{
			if (!Waypoint.bIsCompleted)
			{
				return false;
			}
		}
		return true;
	}
};

/**
 * Current travel state
 */
USTRUCT(BlueprintType)
struct FTravelState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel State")
	ETravelStatus Status = ETravelStatus::Docked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel State")
	FString CurrentSystemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel State")
	FString DestinationSystemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel State")
	float TravelProgress = 0.0f; // 0.0 to 1.0

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel State")
	float DistanceRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel State")
	float TravelSpeed = 0.0f; // Light years per day

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel State")
	float FuelConsumptionRate = 0.0f; // Fuel per light year

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel State")
	float EstimatedArrivalTime = 0.0f; // Days

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel State")
	FDateTime DepartureTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel State")
	FDateTime EstimatedArrivalDateTime;

	FTravelState() = default;

	bool IsInTransit() const
	{
		return Status == ETravelStatus::InTransit;
	}

	bool IsDocked() const
	{
		return Status == ETravelStatus::Docked;
	}
};

/**
 * Navigation settings and capabilities
 */
USTRUCT(BlueprintType)
struct FNavigationCapabilities
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	float MaxJumpDistance = 20.0f; // Light years

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	float CruiseSpeed = 10.0f; // Light years per day

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	float BaseFuelConsumption = 1.0f; // Fuel per light year

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	float FuelCapacity = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	float CurrentFuel = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	bool bHasAdvancedSensors = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	bool bHasLongRangeScanner = false;

	FNavigationCapabilities() = default;

	float GetFuelEfficiency() const
	{
		// Better with more advanced tech (would be influenced by tech tree)
		float efficiency = 1.0f;
		if (bHasAdvancedSensors) efficiency *= 0.9f;
		if (bHasLongRangeScanner) efficiency *= 0.95f;
		return efficiency;
	}

	bool CanMakeJump(float Distance) const
	{
		if (Distance > MaxJumpDistance) return false;
		float requiredFuel = Distance * BaseFuelConsumption * GetFuelEfficiency();
		return CurrentFuel >= requiredFuel;
	}

	float GetMaxReachableDistance() const
	{
		float efficiency = GetFuelEfficiency();
		return (CurrentFuel / (BaseFuelConsumption * efficiency));
	}
};

/**
 * Journey statistics
 */
USTRUCT(BlueprintType)
struct FJourneyStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Journey Stats")
	float TotalDistanceTraveled = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Journey Stats")
	int32 SystemsVisited = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Journey Stats")
	int32 JumpsCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Journey Stats")
	float TotalFuelConsumed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Journey Stats")
	float TotalTravelTime = 0.0f; // Days

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Journey Stats")
	int32 EncountersExperienced = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Journey Stats")
	int32 EmergencyStops = 0;

	FJourneyStats() = default;
};
