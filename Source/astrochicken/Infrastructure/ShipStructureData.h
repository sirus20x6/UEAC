// Source/astrochicken/Infrastructure/ShipStructureData.h

#pragma once

#include "CoreMinimal.h"
#include "ShipStructureData.generated.h"

/**
 * Types of ship compartments
 */
UENUM(BlueprintType)
enum class ECompartmentType : uint8
{
	Bridge UMETA(DisplayName = "Bridge"),
	Engineering UMETA(DisplayName = "Engineering"),
	PowerPlant UMETA(DisplayName = "Power Plant"),
	LifeSupport UMETA(DisplayName = "Life Support"),
	Medbay UMETA(DisplayName = "Medbay"),
	CrewQuarters UMETA(DisplayName = "Crew Quarters"),
	Storage UMETA(DisplayName = "Storage"),
	WeaponsBay UMETA(DisplayName = "Weapons Bay"),
	ShieldGenerator UMETA(DisplayName = "Shield Generator"),
	Sensors UMETA(DisplayName = "Sensors"),
	Hangar UMETA(DisplayName = "Hangar"),
	Laboratory UMETA(DisplayName = "Laboratory"),
	Hydroponics UMETA(DisplayName = "Hydroponics"),
	CargoBay UMETA(DisplayName = "Cargo Bay"),
	Corridor UMETA(DisplayName = "Corridor"),
	Airlock UMETA(DisplayName = "Airlock"),
	Generic UMETA(DisplayName = "Generic")
};

/**
 * Compartment status
 */
UENUM(BlueprintType)
enum class ECompartmentStatus : uint8
{
	Operational UMETA(DisplayName = "Operational"),
	Damaged UMETA(DisplayName = "Damaged"),
	Critical UMETA(DisplayName = "Critical"),
	Destroyed UMETA(DisplayName = "Destroyed"),
	Depressurized UMETA(DisplayName = "Depressurized"),
	OnFire UMETA(DisplayName = "On Fire"),
	Breached UMETA(DisplayName = "Breached")
};

/**
 * Door state
 */
UENUM(BlueprintType)
enum class EDoorState : uint8
{
	Open UMETA(DisplayName = "Open"),
	Closed UMETA(DisplayName = "Closed"),
	Locked UMETA(DisplayName = "Locked"),
	Damaged UMETA(DisplayName = "Damaged"),
	Sealed UMETA(DisplayName = "Sealed")
};

/**
 * Atmosphere data for a compartment
 */
USTRUCT(BlueprintType)
struct FAtmosphereData
{
	GENERATED_BODY()

	// Oxygen level (0-100%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	float OxygenLevel = 100.0f;

	// Pressure (0-100% of normal)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	float Pressure = 100.0f;

	// Temperature (Celsius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	float Temperature = 20.0f;

	// Contaminants/toxicity level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	float Contamination = 0.0f;

	// Is atmosphere breathable?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
	bool bIsBreathable = true;

	FAtmosphereData() = default;

	// Check if atmosphere is safe for crew
	bool IsSafe() const
	{
		return bIsBreathable &&
		       OxygenLevel > 18.0f &&
		       Pressure > 50.0f &&
		       Contamination < 20.0f &&
		       Temperature > -10.0f &&
		       Temperature < 40.0f;
	}
};

/**
 * Door connecting compartments
 */
USTRUCT(BlueprintType)
struct FShipDoor
{
	GENERATED_BODY()

	// Door identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	FString DoorID;

	// Connected compartments
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	FString CompartmentA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	FString CompartmentB;

	// Door state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	EDoorState State = EDoorState::Closed;

	// Is this an airlock?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	bool bIsAirlock = false;

	// Power requirement to operate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	float PowerRequired = 0.5f;

	// Health (0-100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	float Health = 100.0f;

	FShipDoor() = default;

	bool CanOperate() const
	{
		return Health > 0.0f && State != EDoorState::Damaged;
	}
};

/**
 * Ship compartment/room definition
 */
USTRUCT(BlueprintType)
struct FShipCompartment
{
	GENERATED_BODY()

	// Unique identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compartment")
	FString CompartmentID;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compartment")
	FString CompartmentName;

	// Compartment type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compartment")
	ECompartmentType Type = ECompartmentType::Generic;

	// Deck level (0 = main deck, 1 = deck above, -1 = deck below, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compartment")
	int32 DeckLevel = 0;

	// Position on ship
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compartment")
	FVector Position = FVector::ZeroVector;

	// Volume of compartment (cubic meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compartment")
	float Volume = 100.0f;

	// Current status
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compartment")
	ECompartmentStatus Status = ECompartmentStatus::Operational;

	// Structural integrity (0-100%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compartment")
	float Integrity = 100.0f;

	// Hull armor rating
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compartment")
	float ArmorRating = 1.0f;

	// Atmosphere data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compartment")
	FAtmosphereData Atmosphere;

	// Fire intensity (0-100%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compartment")
	float FireIntensity = 0.0f;

	// Connected compartments (via doors)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compartment")
	TArray<FString> ConnectedCompartments;

	// Crew currently in compartment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compartment")
	TArray<FString> CrewIDs;

	// Maximum crew capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compartment")
	int32 MaxCrewCapacity = 4;

	// Power nodes in this compartment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compartment")
	TArray<FString> PowerNodeIDs;

	// Systems installed in this compartment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compartment")
	TArray<FString> SystemIDs;

	FShipCompartment() = default;

	bool IsHabitable() const
	{
		return Status != ECompartmentStatus::Destroyed &&
		       Status != ECompartmentStatus::Breached &&
		       Atmosphere.IsSafe() &&
		       FireIntensity < 50.0f;
	}

	bool IsOperational() const
	{
		return Status == ECompartmentStatus::Operational &&
		       Integrity > 20.0f;
	}
};

/**
 * Ship deck (collection of compartments)
 */
USTRUCT(BlueprintType)
struct FShipDeck
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	int32 DeckLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	FString DeckName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	TArray<FString> CompartmentIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deck")
	float OverallIntegrity = 100.0f;

	FShipDeck() = default;
};

/**
 * Overall ship layout
 */
USTRUCT(BlueprintType)
struct FShipLayout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Layout")
	FString ShipName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Layout")
	FString ShipClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Layout")
	TArray<FShipDeck> Decks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Layout")
	TMap<FString, FShipCompartment> Compartments;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Layout")
	TMap<FString, FShipDoor> Doors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Layout")
	float OverallIntegrity = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Layout")
	int32 TotalCrewCapacity = 0;

	FShipLayout() = default;
};
