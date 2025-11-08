// Source/astrochicken/Infrastructure/HardpointData.h

#pragma once

#include "CoreMinimal.h"
#include "HardpointData.generated.h"

/**
 * Hardpoint size categories
 */
UENUM(BlueprintType)
enum class EHardpointSize : uint8
{
	Small UMETA(DisplayName = "Small"),
	Medium UMETA(DisplayName = "Medium"),
	Large UMETA(DisplayName = "Large"),
	ExtraLarge UMETA(DisplayName = "Extra Large"),
	Capital UMETA(DisplayName = "Capital")
};

/**
 * Equipment types that can be mounted
 */
UENUM(BlueprintType)
enum class EEquipmentType : uint8
{
	Weapon_Light UMETA(DisplayName = "Light Weapon"),
	Weapon_Medium UMETA(DisplayName = "Medium Weapon"),
	Weapon_Heavy UMETA(DisplayName = "Heavy Weapon"),
	Weapon_PointDefense UMETA(DisplayName = "Point Defense"),
	Sensor_Standard UMETA(DisplayName = "Standard Sensor"),
	Sensor_Advanced UMETA(DisplayName = "Advanced Sensor"),
	Shield_Generator UMETA(DisplayName = "Shield Generator"),
	Engine_Thruster UMETA(DisplayName = "Thruster"),
	Engine_Main UMETA(DisplayName = "Main Engine"),
	Utility_TractorBeam UMETA(DisplayName = "Tractor Beam"),
	Utility_MiningLaser UMETA(DisplayName = "Mining Laser"),
	Utility_RepairDrone UMETA(DisplayName = "Repair Drone"),
	Communication UMETA(DisplayName = "Communication Array"),
	Docking_Port UMETA(DisplayName = "Docking Port")
};

/**
 * Hardpoint status
 */
UENUM(BlueprintType)
enum class EHardpointStatus : uint8
{
	Empty UMETA(DisplayName = "Empty"),
	Occupied UMETA(DisplayName = "Occupied"),
	Damaged UMETA(DisplayName = "Damaged"),
	Destroyed UMETA(DisplayName = "Destroyed"),
	Installing UMETA(DisplayName = "Installing"),
	Removing UMETA(DisplayName = "Removing")
};

/**
 * Equipment operational state
 */
UENUM(BlueprintType)
enum class EEquipmentState : uint8
{
	Operational UMETA(DisplayName = "Operational"),
	Standby UMETA(DisplayName = "Standby"),
	Damaged UMETA(DisplayName = "Damaged"),
	Overheating UMETA(DisplayName = "Overheating"),
	NoPower UMETA(DisplayName = "No Power"),
	Disabled UMETA(DisplayName = "Disabled"),
	Destroyed UMETA(DisplayName = "Destroyed")
};

/**
 * Equipment mounted on a hardpoint
 */
USTRUCT(BlueprintType)
struct FMountedEquipment
{
	GENERATED_BODY()

	// Unique identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FString EquipmentID;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FString EquipmentName;

	// Equipment type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	EEquipmentType Type = EEquipmentType::Weapon_Light;

	// Required hardpoint size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	EHardpointSize RequiredSize = EHardpointSize::Small;

	// Current state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	EEquipmentState State = EEquipmentState::Operational;

	// Health (0-100%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	float Health = 100.0f;

	// Power requirements
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	float PowerRequired = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	float PowerConsumption = 0.0f;

	// Heat generation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	float HeatGeneration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	float CurrentHeat = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	float MaxHeat = 100.0f;

	// Coolant required
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	bool bRequiresCoolant = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	float CoolantFlow = 0.0f;

	// Mass (kg)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	float Mass = 100.0f;

	// Crew required to operate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	int32 CrewRequired = 0;

	// Current crew assigned
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TArray<FString> AssignedCrewIDs;

	// Performance stats (type-specific)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TMap<FString, float> PerformanceStats;

	FMountedEquipment() = default;

	bool IsOperational() const
	{
		return State == EEquipmentState::Operational && Health > 0.0f;
	}

	bool HasSufficientPower() const
	{
		return PowerConsumption >= PowerRequired;
	}

	bool IsOverheating() const
	{
		return CurrentHeat > MaxHeat * 0.9f;
	}

	float GetEfficiency() const
	{
		if (!IsOperational())
		{
			return 0.0f;
		}

		float healthFactor = Health / 100.0f;
		float powerFactor = FMath::Clamp(PowerConsumption / PowerRequired, 0.0f, 1.0f);
		float heatFactor = FMath::Clamp(1.0f - (CurrentHeat / MaxHeat), 0.5f, 1.0f);

		return healthFactor * powerFactor * heatFactor;
	}
};

/**
 * Hardpoint mount point on ship
 */
USTRUCT(BlueprintType)
struct FHardpoint
{
	GENERATED_BODY()

	// Unique identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	FString HardpointID;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	FString HardpointName;

	// Size category
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	EHardpointSize Size = EHardpointSize::Medium;

	// Status
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	EHardpointStatus Status = EHardpointStatus::Empty;

	// Compartment this hardpoint is in
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	FString CompartmentID;

	// Position (world space or relative to ship)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	FVector Position = FVector::ZeroVector;

	// Rotation (for directional equipment like weapons)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	FRotator Rotation = FRotator::ZeroRotator;

	// Allowed equipment types
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	TArray<EEquipmentType> AllowedTypes;

	// Currently mounted equipment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	FMountedEquipment MountedEquipment;

	// Power node providing power to this hardpoint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	FString PowerNodeID;

	// Coolant connection ID (if applicable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	FString CoolantNodeID;

	// Structural integrity (0-100%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	float Integrity = 100.0f;

	// Maximum load capacity (kg)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	float MaxLoadCapacity = 1000.0f;

	// Can equipment be changed in-flight?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	bool bHotSwappable = false;

	// Installation time (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	float InstallationTime = 60.0f;

	// Current installation/removal progress
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint")
	float InstallProgress = 0.0f;

	FHardpoint() = default;

	bool IsEmpty() const
	{
		return Status == EHardpointStatus::Empty;
	}

	bool CanMount(const FMountedEquipment& Equipment) const
	{
		if (Status != EHardpointStatus::Empty)
		{
			return false;
		}

		if (Integrity < 50.0f)
		{
			return false;
		}

		// Check size compatibility
		if (Equipment.RequiredSize > Size)
		{
			return false;
		}

		// Check weight capacity
		if (Equipment.Mass > MaxLoadCapacity)
		{
			return false;
		}

		// Check type restrictions
		if (AllowedTypes.Num() > 0 && !AllowedTypes.Contains(Equipment.Type))
		{
			return false;
		}

		return true;
	}

	bool HasMountedEquipment() const
	{
		return Status == EHardpointStatus::Occupied;
	}
};

/**
 * Collection of hardpoints on ship
 */
USTRUCT(BlueprintType)
struct FHardpointLayout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint Layout")
	TMap<FString, FHardpoint> Hardpoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint Layout")
	int32 TotalHardpoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint Layout")
	int32 OccupiedHardpoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardpoint Layout")
	int32 DamagedHardpoints = 0;

	FHardpointLayout() = default;
};
