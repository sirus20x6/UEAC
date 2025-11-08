// Source/astrochicken/Infrastructure/ResourceFlowData.h

#pragma once

#include "CoreMinimal.h"
#include "ResourceFlowData.generated.h"

/**
 * Resource types
 */
UENUM(BlueprintType)
enum class EResourceType : uint8
{
	Power UMETA(DisplayName = "Electrical Power"),
	Oxygen UMETA(DisplayName = "Oxygen"),
	Coolant UMETA(DisplayName = "Coolant"),
	Water UMETA(DisplayName = "Water"),
	Fuel UMETA(DisplayName = "Fuel"),
	Waste UMETA(DisplayName = "Waste Heat")
};

/**
 * Resource node status
 */
UENUM(BlueprintType)
enum class EResourceNodeStatus : uint8
{
	Operational UMETA(DisplayName = "Operational"),
	LowPressure UMETA(DisplayName = "Low Pressure"),
	HighPressure UMETA(DisplayName = "High Pressure"),
	Leaking UMETA(DisplayName = "Leaking"),
	Blocked UMETA(DisplayName = "Blocked"),
	Offline UMETA(DisplayName = "Offline")
};

/**
 * Resource pipe/conduit
 */
USTRUCT(BlueprintType)
struct FResourcePipe
{
	GENERATED_BODY()

	// Unique identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe")
	FString PipeID;

	// Resource type this pipe carries
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe")
	EResourceType ResourceType = EResourceType::Oxygen;

	// Source node
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe")
	FString SourceNodeID;

	// Destination node
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe")
	FString DestinationNodeID;

	// Maximum flow rate (units/second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe")
	float MaxFlowRate = 10.0f;

	// Current flow rate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe")
	float CurrentFlowRate = 0.0f;

	// Pipe diameter (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe")
	float Diameter = 0.1f;

	// Pipe length (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe")
	float Length = 5.0f;

	// Pressure at source (kPa)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe")
	float SourcePressure = 100.0f;

	// Pressure at destination (kPa)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe")
	float DestinationPressure = 100.0f;

	// Health (0-100%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe")
	float Health = 100.0f;

	// Is pipe ruptured/leaking?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe")
	bool bIsLeaking = false;

	// Leak rate (if leaking)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe")
	float LeakRate = 0.0f;

	FResourcePipe() = default;

	float GetEffectiveFlowRate() const
	{
		if (bIsLeaking)
		{
			return CurrentFlowRate * (1.0f - LeakRate);
		}

		float healthFactor = Health / 100.0f;
		return CurrentFlowRate * healthFactor;
	}

	bool IsBlocked() const
	{
		return Health < 10.0f || CurrentFlowRate == 0.0f;
	}
};

/**
 * Resource node (tank, generator, consumer)
 */
USTRUCT(BlueprintType)
struct FResourceNode
{
	GENERATED_BODY()

	// Unique identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
	FString NodeID;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
	FString NodeName;

	// Resource type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
	EResourceType ResourceType = EResourceType::Oxygen;

	// Status
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
	EResourceNodeStatus Status = EResourceNodeStatus::Operational;

	// Compartment this node is in
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
	FString CompartmentID;

	// Is this a generator/producer?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
	bool bIsProducer = false;

	// Production rate (units/second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
	float ProductionRate = 0.0f;

	// Maximum production capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
	float MaxProduction = 0.0f;

	// Is this a consumer?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
	bool bIsConsumer = false;

	// Consumption rate (units/second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
	float ConsumptionRate = 0.0f;

	// Required consumption rate for operation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
	float RequiredConsumption = 0.0f;

	// Storage capacity (if tank/reservoir)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
	float StorageCapacity = 0.0f;

	// Current stored amount
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
	float StoredAmount = 0.0f;

	// Pressure (kPa)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
	float Pressure = 100.0f;

	// Temperature (Celsius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
	float Temperature = 20.0f;

	// Connected pipes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
	TArray<FString> ConnectedPipeIDs;

	// Priority for resource distribution
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
	int32 Priority = 5;

	// Power required to operate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Node")
	float PowerRequired = 0.0f;

	FResourceNode() = default;

	bool IsProducing() const
	{
		return bIsProducer && ProductionRate > 0.0f;
	}

	bool IsConsuming() const
	{
		return bIsConsumer && ConsumptionRate > 0.0f;
	}

	bool HasStorage() const
	{
		return StorageCapacity > 0.0f;
	}

	float GetStorageFillPercent() const
	{
		if (StorageCapacity <= 0.0f)
		{
			return 0.0f;
		}

		return (StoredAmount / StorageCapacity) * 100.0f;
	}

	bool IsFull() const
	{
		return StoredAmount >= StorageCapacity;
	}

	bool IsEmpty() const
	{
		return StoredAmount <= 0.0f;
	}
};

/**
 * Life support system
 */
USTRUCT(BlueprintType)
struct FLifeSupportSystem
{
	GENERATED_BODY()

	// Oxygen generator node ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Life Support")
	FString OxygenGeneratorID;

	// CO2 scrubber node ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Life Support")
	FString CO2ScrubberID;

	// Water recycler node ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Life Support")
	FString WaterRecyclerID;

	// Temperature control node ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Life Support")
	FString TemperatureControlID;

	// Is system operational?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Life Support")
	bool bIsOperational = true;

	// Power consumption
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Life Support")
	float PowerConsumption = 20.0f;

	// Oxygen production rate (L/s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Life Support")
	float OxygenProduction = 10.0f;

	// Current crew supported
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Life Support")
	int32 CurrentCrew = 0;

	// Maximum crew capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Life Support")
	int32 MaxCrewCapacity = 20;

	FLifeSupportSystem() = default;

	bool CanSupportCrew(int32 CrewCount) const
	{
		return bIsOperational && CrewCount <= MaxCrewCapacity;
	}

	float GetEfficiency() const
	{
		if (!bIsOperational || MaxCrewCapacity == 0)
		{
			return 0.0f;
		}

		return 1.0f - (static_cast<float>(CurrentCrew) / MaxCrewCapacity);
	}
};

/**
 * Coolant system
 */
USTRUCT(BlueprintType)
struct FCoolantSystem
{
	GENERATED_BODY()

	// Primary coolant loop node ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant")
	FString PrimaryCoolantLoopID;

	// Heat exchanger node IDs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant")
	TArray<FString> HeatExchangerIDs;

	// Radiator node IDs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant")
	TArray<FString> RadiatorIDs;

	// Total coolant capacity (liters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant")
	float TotalCoolantCapacity = 1000.0f;

	// Current coolant level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant")
	float CurrentCoolantLevel = 1000.0f;

	// Coolant flow rate (L/s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant")
	float FlowRate = 10.0f;

	// Coolant temperature (Celsius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant")
	float CoolantTemperature = 20.0f;

	// Heat dissipation rate (kW)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant")
	float HeatDissipationRate = 100.0f;

	// Current heat load (kW)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant")
	float CurrentHeatLoad = 0.0f;

	// Is system operational?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coolant")
	bool bIsOperational = true;

	FCoolantSystem() = default;

	bool IsOverloaded() const
	{
		return CurrentHeatLoad > HeatDissipationRate;
	}

	float GetCoolantLevelPercent() const
	{
		return (CurrentCoolantLevel / TotalCoolantCapacity) * 100.0f;
	}

	bool NeedsRefill() const
	{
		return CurrentCoolantLevel < TotalCoolantCapacity * 0.25f;
	}
};

/**
 * Resource network state
 */
USTRUCT(BlueprintType)
struct FResourceNetwork
{
	GENERATED_BODY()

	// All resource nodes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Network")
	TMap<FString, FResourceNode> Nodes;

	// All resource pipes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Network")
	TMap<FString, FResourcePipe> Pipes;

	// Life support system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Network")
	FLifeSupportSystem LifeSupport;

	// Coolant system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Network")
	FCoolantSystem Coolant;

	// Total oxygen available
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Network")
	float TotalOxygen = 0.0f;

	// Total coolant available
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Network")
	float TotalCoolantAvailable = 0.0f;

	// Number of leaking pipes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Network")
	int32 LeakingPipes = 0;

	FResourceNetwork() = default;
};
