// Source/astrochicken/Journey/JourneyManager.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "JourneyData.h"
#include "JourneyManager.generated.h"

class UShipInfrastructureManager;

// Delegates for journey events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJumpStarted, const FString&, DestinationSystemID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJumpCompleted, const FString&, ArrivedSystemID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTravelProgress, float, Progress, float, DistanceRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFuelLow, float, RemainingFuel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEmergencyStop);

/**
 * Journey Manager - Handles space travel, navigation, and fuel management
 * Manages the Oregon Trail-style journey through space
 */
UCLASS()
class ASTROCHICKEN_API UJourneyManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Update travel progress (should be called from game tick or timer)
	UFUNCTION(BlueprintCallable, Category = "Journey")
	void UpdateTravel(float DeltaTime);

	// Navigation operations
	UFUNCTION(BlueprintCallable, Category = "Journey")
	bool InitiateJump(const FString& DestinationSystemID);

	UFUNCTION(BlueprintCallable, Category = "Journey")
	bool CanJumpTo(const FString& SystemID, FString& OutReason) const;

	UFUNCTION(BlueprintCallable, Category = "Journey")
	bool DockAtSystem(const FString& SystemID);

	UFUNCTION(BlueprintCallable, Category = "Journey")
	void EmergencyStop();

	// Route planning
	UFUNCTION(BlueprintCallable, Category = "Journey")
	FRoute PlanRoute(const FString& FromSystemID, const FString& ToSystemID, int32 MaxWaypoints = 10);

	UFUNCTION(BlueprintCallable, Category = "Journey")
	bool SetCurrentRoute(const FRoute& Route);

	UFUNCTION(BlueprintCallable, Category = "Journey")
	bool FollowRoute();

	// Star system queries
	UFUNCTION(BlueprintCallable, Category = "Journey")
	bool GetStarSystem(const FString& SystemID, FStarSystem& OutSystem) const;

	UFUNCTION(BlueprintCallable, Category = "Journey")
	TArray<FStarSystem> GetSystemsInRange(float MaxDistance) const;

	UFUNCTION(BlueprintCallable, Category = "Journey")
	TArray<FStarSystem> GetDiscoveredSystems() const;

	UFUNCTION(BlueprintCallable, Category = "Journey")
	float CalculateDistance(const FString& SystemA, const FString& SystemB) const;

	// Fuel management
	UFUNCTION(BlueprintCallable, Category = "Journey")
	bool RefuelShip(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Journey")
	float GetCurrentFuel() const { return NavigationCaps.CurrentFuel; }

	UFUNCTION(BlueprintCallable, Category = "Journey")
	float GetFuelCapacity() const { return NavigationCaps.FuelCapacity; }

	UFUNCTION(BlueprintCallable, Category = "Journey")
	float GetFuelPercentage() const;

	// State queries
	UFUNCTION(BlueprintCallable, Category = "Journey")
	FTravelState GetCurrentTravelState() const { return CurrentTravel; }

	UFUNCTION(BlueprintCallable, Category = "Journey")
	FNavigationCapabilities GetNavigationCapabilities() const { return NavigationCaps; }

	UFUNCTION(BlueprintCallable, Category = "Journey")
	FJourneyStats GetJourneyStats() const { return Stats; }

	UFUNCTION(BlueprintCallable, Category = "Journey")
	FString GetCurrentSystemID() const { return CurrentTravel.CurrentSystemID; }

	UFUNCTION(BlueprintCallable, Category = "Journey")
	bool IsInTransit() const { return CurrentTravel.IsInTransit(); }

	// Discovery
	UFUNCTION(BlueprintCallable, Category = "Journey")
	void DiscoverSystem(const FString& SystemID);

	// Serialization
	FString SerializeToJSON() const;
	bool DeserializeFromJSON(const FString& JSON);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Journey Events")
	FOnJumpStarted OnJumpStarted;

	UPROPERTY(BlueprintAssignable, Category = "Journey Events")
	FOnJumpCompleted OnJumpCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Journey Events")
	FOnTravelProgress OnTravelProgress;

	UPROPERTY(BlueprintAssignable, Category = "Journey Events")
	FOnFuelLow OnFuelLow;

	UPROPERTY(BlueprintAssignable, Category = "Journey Events")
	FOnEmergencyStop OnEmergencyStop;

private:
	// Initialization
	void InitializeStarMap();
	void CreateStarSystem(const FString& ID, const FString& Name, EStarType Type,
	                      const FVector& Position, const TArray<ESystemAttribute>& Attributes);

	// Travel simulation
	void ProcessTravel(float DeltaTime);
	void ConsumeFuel(float Distance);
	void CheckFuelWarnings();
	void CompleteJump();

	// Route finding (simple pathfinding)
	TArray<FString> FindPath(const FString& Start, const FString& End, int32 MaxWaypoints);

	// Integration with other systems
	void UpdateShipSystems();

	// Star map database
	UPROPERTY()
	TMap<FString, FStarSystem> StarSystems;

	// Current state
	UPROPERTY()
	FTravelState CurrentTravel;

	UPROPERTY()
	FNavigationCapabilities NavigationCaps;

	UPROPERTY()
	FRoute CurrentRoute;

	UPROPERTY()
	FJourneyStats Stats;

	// References
	UPROPERTY()
	UShipInfrastructureManager* ShipInfrastructureManager;

	// Configuration
	float FuelWarningThreshold = 20.0f; // Percentage
	float FuelCriticalThreshold = 10.0f; // Percentage
	bool bHasWarnedLowFuel = false;

	// Time tracking
	float GameTime = 0.0f; // In-game days elapsed
	float TimeAccelerationFactor = 1.0f;
};
