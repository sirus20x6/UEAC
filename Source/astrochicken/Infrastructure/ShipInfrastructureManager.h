// Source/astrochicken/Infrastructure/ShipInfrastructureManager.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ShipStructureData.h"
#include "ElectricalGridData.h"
#include "HardpointData.h"
#include "ResourceFlowData.h"
#include "ShipInfrastructureManager.generated.h"

/**
 * Manages all ship infrastructure systems
 * Similar to FTL but more in-depth with power grids, resource flows, and modular design
 */
UCLASS()
class ASTROCHICKEN_API UShipInfrastructureManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tick/Update
	void UpdateSystems(float DeltaTime);

	// ===== Ship Structure =====

	UFUNCTION(BlueprintCallable, Category = "Ship Structure")
	FShipLayout GetShipLayout() const { return ShipLayout; }

	UFUNCTION(BlueprintCallable, Category = "Ship Structure")
	bool GetCompartment(const FString& CompartmentID, FShipCompartment& OutCompartment);

	UFUNCTION(BlueprintCallable, Category = "Ship Structure")
	bool DamageCompartment(const FString& CompartmentID, float Damage);

	UFUNCTION(BlueprintCallable, Category = "Ship Structure")
	bool RepairCompartment(const FString& CompartmentID, float RepairAmount);

	UFUNCTION(BlueprintCallable, Category = "Ship Structure")
	bool IsCompartmentHabitable(const FString& CompartmentID);

	UFUNCTION(BlueprintCallable, Category = "Ship Structure")
	bool SetDoorState(const FString& DoorID, EDoorState NewState);

	// ===== Electrical Grid =====

	UFUNCTION(BlueprintCallable, Category = "Electrical Grid")
	FElectricalGrid GetElectricalGrid() const { return ElectricalGrid; }

	UFUNCTION(BlueprintCallable, Category = "Electrical Grid")
	float GetTotalPowerGeneration() const { return ElectricalGrid.TotalGeneration; }

	UFUNCTION(BlueprintCallable, Category = "Electrical Grid")
	float GetTotalPowerConsumption() const { return ElectricalGrid.TotalConsumption; }

	UFUNCTION(BlueprintCallable, Category = "Electrical Grid")
	bool HasPowerDeficit() const { return ElectricalGrid.HasPowerDeficit(); }

	UFUNCTION(BlueprintCallable, Category = "Electrical Grid")
	bool SetReactorOutput(const FString& ReactorID, float OutputPercent);

	UFUNCTION(BlueprintCallable, Category = "Electrical Grid")
	bool AddPowerNode(const FPowerNode& Node);

	UFUNCTION(BlueprintCallable, Category = "Electrical Grid")
	bool AddPowerConduit(const FPowerConduit& Conduit);

	UFUNCTION(BlueprintCallable, Category = "Electrical Grid")
	bool DamagePowerNode(const FString& NodeID, float Damage);

	UFUNCTION(BlueprintCallable, Category = "Electrical Grid")
	bool DamagePowerConduit(const FString& ConduitID, float Damage);

	// ===== Hardpoints =====

	UFUNCTION(BlueprintCallable, Category = "Hardpoints")
	FHardpointLayout GetHardpointLayout() const { return HardpointLayout; }

	UFUNCTION(BlueprintCallable, Category = "Hardpoints")
	bool GetHardpoint(const FString& HardpointID, FHardpoint& OutHardpoint);

	UFUNCTION(BlueprintCallable, Category = "Hardpoints")
	bool MountEquipment(const FString& HardpointID, const FMountedEquipment& Equipment);

	UFUNCTION(BlueprintCallable, Category = "Hardpoints")
	bool UnmountEquipment(const FString& HardpointID);

	UFUNCTION(BlueprintCallable, Category = "Hardpoints")
	bool DamageEquipment(const FString& HardpointID, float Damage);

	UFUNCTION(BlueprintCallable, Category = "Hardpoints")
	TArray<FHardpoint> GetAvailableHardpoints(EHardpointSize MinSize);

	// ===== Resource Networks =====

	UFUNCTION(BlueprintCallable, Category = "Resource Network")
	FResourceNetwork GetResourceNetwork() const { return ResourceNetwork; }

	UFUNCTION(BlueprintCallable, Category = "Resource Network")
	bool AddResourceNode(const FResourceNode& Node);

	UFUNCTION(BlueprintCallable, Category = "Resource Network")
	bool AddResourcePipe(const FResourcePipe& Pipe);

	UFUNCTION(BlueprintCallable, Category = "Resource Network")
	bool DamageResourceNode(const FString& NodeID, float Damage);

	UFUNCTION(BlueprintCallable, Category = "Resource Network")
	bool DamageResourcePipe(const FString& PipeID, float Damage);

	UFUNCTION(BlueprintCallable, Category = "Resource Network")
	float GetOxygenLevel(const FString& CompartmentID);

	UFUNCTION(BlueprintCallable, Category = "Resource Network")
	bool IsLifeSupportOperational() const;

	// ===== System Initialization =====

	UFUNCTION(BlueprintCallable, Category = "Ship Infrastructure")
	void InitializeDefaultShipLayout();

	UFUNCTION(BlueprintCallable, Category = "Ship Infrastructure")
	void InitializeStartingShip();

	// ===== Save/Load Integration =====

	UFUNCTION(BlueprintCallable, Category = "Ship Infrastructure")
	FString SerializeToJSON();

	UFUNCTION(BlueprintCallable, Category = "Ship Infrastructure")
	bool DeserializeFromJSON(const FString& JSON);

private:
	// Update functions called each frame
	void UpdateElectricalGrid(float DeltaTime);
	void UpdateResourceNetworks(float DeltaTime);
	void UpdateCompartments(float DeltaTime);
	void UpdateHardpoints(float DeltaTime);
	void UpdateReactors(float DeltaTime);
	void UpdateLifeSupport(float DeltaTime);
	void UpdateCoolantSystem(float DeltaTime);

	// Power distribution algorithm
	void DistributePower();
	void CalculatePowerFlow();
	void HandlePowerPriorities();

	// Resource flow simulation
	void SimulateResourceFlow(EResourceType ResourceType);
	void CalculateAtmosphereChanges(float DeltaTime);

	// Damage propagation
	void PropagateFireDamage(float DeltaTime);
	void PropagateAtmosphereLoss(float DeltaTime);

	// Ship state
	UPROPERTY()
	FShipLayout ShipLayout;

	UPROPERTY()
	FElectricalGrid ElectricalGrid;

	UPROPERTY()
	FHardpointLayout HardpointLayout;

	UPROPERTY()
	FResourceNetwork ResourceNetwork;

	// Overall ship status
	UPROPERTY()
	float OverallShipIntegrity = 100.0f;

	UPROPERTY()
	int32 TotalCrewMembers = 0;

	UPROPERTY()
	bool bEmergencyPowerMode = false;

	// Update timing
	float PowerUpdateAccumulator = 0.0f;
	float ResourceUpdateAccumulator = 0.0f;
	const float PowerUpdateInterval = 0.1f; // Update power 10x per second
	const float ResourceUpdateInterval = 0.5f; // Update resources 2x per second
};
