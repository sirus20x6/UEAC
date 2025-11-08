// Source/astrochicken/Infrastructure/ElectricalGridData.h

#pragma once

#include "CoreMinimal.h"
#include "ElectricalGridData.generated.h"

/**
 * Power node types
 */
UENUM(BlueprintType)
enum class EPowerNodeType : uint8
{
	Reactor UMETA(DisplayName = "Reactor"),
	Junction UMETA(DisplayName = "Junction Box"),
	Distributor UMETA(DisplayName = "Power Distributor"),
	Capacitor UMETA(DisplayName = "Capacitor/Battery"),
	Consumer UMETA(DisplayName = "Power Consumer")
};

/**
 * Node status
 */
UENUM(BlueprintType)
enum class EPowerNodeStatus : uint8
{
	Operational UMETA(DisplayName = "Operational"),
	Overloaded UMETA(DisplayName = "Overloaded"),
	Damaged UMETA(DisplayName = "Damaged"),
	Offline UMETA(DisplayName = "Offline"),
	Emergency UMETA(DisplayName = "Emergency Power")
};

/**
 * Reactor types
 */
UENUM(BlueprintType)
enum class EReactorType : uint8
{
	Fission UMETA(DisplayName = "Fission Reactor"),
	Fusion UMETA(DisplayName = "Fusion Reactor"),
	Antimatter UMETA(DisplayName = "Antimatter Reactor"),
	Solar UMETA(DisplayName = "Solar Array"),
	Chemical UMETA(DisplayName = "Chemical Battery")
};

/**
 * Power conduit (wire/connection between nodes)
 */
USTRUCT(BlueprintType)
struct FPowerConduit
{
	GENERATED_BODY()

	// Unique identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conduit")
	FString ConduitID;

	// Source node
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conduit")
	FString SourceNodeID;

	// Destination node
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conduit")
	FString DestinationNodeID;

	// Maximum power capacity (kW)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conduit")
	float MaxCapacity = 100.0f;

	// Current power flow (kW)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conduit")
	float CurrentFlow = 0.0f;

	// Efficiency (0-1, power loss during transmission)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conduit")
	float Efficiency = 0.95f;

	// Health (0-100%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conduit")
	float Health = 100.0f;

	// Is conduit damaged/severed?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conduit")
	bool bIsDamaged = false;

	FPowerConduit() = default;

	float GetEffectivePower() const
	{
		if (bIsDamaged || Health <= 0.0f)
		{
			return 0.0f;
		}

		float healthFactor = Health / 100.0f;
		return CurrentFlow * Efficiency * healthFactor;
	}

	bool IsOverloaded() const
	{
		return CurrentFlow > MaxCapacity;
	}
};

/**
 * Power node in electrical grid
 */
USTRUCT(BlueprintType)
struct FPowerNode
{
	GENERATED_BODY()

	// Unique identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Node")
	FString NodeID;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Node")
	FString NodeName;

	// Node type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Node")
	EPowerNodeType Type = EPowerNodeType::Junction;

	// Status
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Node")
	EPowerNodeStatus Status = EPowerNodeStatus::Operational;

	// Compartment this node is in
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Node")
	FString CompartmentID;

	// Position within compartment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Node")
	FVector LocalPosition = FVector::ZeroVector;

	// Power generation (if reactor/generator)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Node")
	float PowerGeneration = 0.0f;

	// Maximum power generation capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Node")
	float MaxPowerGeneration = 0.0f;

	// Power consumption (if consumer)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Node")
	float PowerConsumption = 0.0f;

	// Required power for operation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Node")
	float RequiredPower = 0.0f;

	// Power storage capacity (if capacitor/battery)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Node")
	float StorageCapacity = 0.0f;

	// Current stored power
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Node")
	float StoredPower = 0.0f;

	// Maximum throughput (for junctions/distributors)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Node")
	float MaxThroughput = 100.0f;

	// Current power available at this node
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Node")
	float AvailablePower = 0.0f;

	// Connected conduits
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Node")
	TArray<FString> ConnectedConduitIDs;

	// Priority for power distribution (higher = more important)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Node")
	int32 Priority = 5;

	// Can node operate on partial power?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Node")
	bool bCanOperatePartial = true;

	// Health (0-100%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Node")
	float Health = 100.0f;

	FPowerNode() = default;

	bool IsGenerator() const
	{
		return Type == EPowerNodeType::Reactor && MaxPowerGeneration > 0.0f;
	}

	bool IsConsumer() const
	{
		return Type == EPowerNodeType::Consumer || RequiredPower > 0.0f;
	}

	bool HasSufficientPower() const
	{
		return AvailablePower >= RequiredPower;
	}

	float GetPowerDeficit() const
	{
		return FMath::Max(0.0f, RequiredPower - AvailablePower);
	}
};

/**
 * Reactor definition
 */
USTRUCT(BlueprintType)
struct FReactor
{
	GENERATED_BODY()

	// Associated power node ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactor")
	FString PowerNodeID;

	// Reactor type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactor")
	EReactorType ReactorType = EReactorType::Fusion;

	// Fuel level (0-100%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactor")
	float FuelLevel = 100.0f;

	// Fuel consumption rate (per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactor")
	float FuelConsumptionRate = 0.1f;

	// Temperature (Celsius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactor")
	float Temperature = 20.0f;

	// Optimal operating temperature
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactor")
	float OptimalTemperature = 1000.0f;

	// Maximum safe temperature
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactor")
	float MaxSafeTemperature = 2000.0f;

	// Current power output percentage (0-100%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactor")
	float PowerOutputPercent = 100.0f;

	// Is reactor in emergency shutdown?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactor")
	bool bEmergencyShutdown = false;

	// Requires coolant?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactor")
	bool bRequiresCoolant = true;

	// Coolant level (0-100%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reactor")
	float CoolantLevel = 100.0f;

	FReactor() = default;

	bool IsOverheating() const
	{
		return Temperature > MaxSafeTemperature * 0.9f;
	}

	bool IsCritical() const
	{
		return Temperature > MaxSafeTemperature;
	}

	float GetEfficiency() const
	{
		if (bEmergencyShutdown)
		{
			return 0.0f;
		}

		// Efficiency drops if too hot or too cold
		float tempDelta = FMath::Abs(Temperature - OptimalTemperature);
		float tempEfficiency = FMath::Clamp(1.0f - (tempDelta / OptimalTemperature), 0.1f, 1.0f);

		// Efficiency drops with low fuel
		float fuelEfficiency = FMath::Clamp(FuelLevel / 100.0f, 0.0f, 1.0f);

		return tempEfficiency * fuelEfficiency * (PowerOutputPercent / 100.0f);
	}
};

/**
 * Electrical grid state
 */
USTRUCT(BlueprintType)
struct FElectricalGrid
{
	GENERATED_BODY()

	// All power nodes in the grid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TMap<FString, FPowerNode> Nodes;

	// All power conduits
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TMap<FString, FPowerConduit> Conduits;

	// All reactors
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TMap<FString, FReactor> Reactors;

	// Total power generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float TotalGeneration = 0.0f;

	// Total power consumption
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float TotalConsumption = 0.0f;

	// Total stored power
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float TotalStored = 0.0f;

	// Grid efficiency (0-100%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float GridEfficiency = 100.0f;

	// Number of overloaded conduits
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 OverloadedConduits = 0;

	// Number of offline nodes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 OfflineNodes = 0;

	FElectricalGrid() = default;

	bool HasPowerDeficit() const
	{
		return TotalConsumption > TotalGeneration;
	}

	float GetPowerDeficit() const
	{
		return FMath::Max(0.0f, TotalConsumption - TotalGeneration);
	}

	float GetPowerSurplus() const
	{
		return FMath::Max(0.0f, TotalGeneration - TotalConsumption);
	}
};
