// Source/astrochicken/Infrastructure/ShipInfrastructureManager.cpp

#include "ShipInfrastructureManager.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void UShipInfrastructureManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("ShipInfrastructureManager: Initializing"));

	// Initialize with default ship layout
	InitializeStartingShip();
}

void UShipInfrastructureManager::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("ShipInfrastructureManager: Deinitialized"));
}

void UShipInfrastructureManager::UpdateSystems(float DeltaTime)
{
	// Update electrical grid
	PowerUpdateAccumulator += DeltaTime;
	if (PowerUpdateAccumulator >= PowerUpdateInterval)
	{
		UpdateElectricalGrid(PowerUpdateAccumulator);
		PowerUpdateAccumulator = 0.0f;
	}

	// Update resource networks
	ResourceUpdateAccumulator += DeltaTime;
	if (ResourceUpdateAccumulator >= ResourceUpdateInterval)
	{
		UpdateResourceNetworks(ResourceUpdateAccumulator);
		ResourceUpdateAccumulator = 0.0f;
	}

	// Update other systems every frame
	UpdateCompartments(DeltaTime);
	UpdateHardpoints(DeltaTime);
	UpdateReactors(DeltaTime);
}

// ===== Ship Structure =====

bool UShipInfrastructureManager::GetCompartment(const FString& CompartmentID, FShipCompartment& OutCompartment)
{
	if (ShipLayout.Compartments.Contains(CompartmentID))
	{
		OutCompartment = ShipLayout.Compartments[CompartmentID];
		return true;
	}

	return false;
}

bool UShipInfrastructureManager::DamageCompartment(const FString& CompartmentID, float Damage)
{
	if (!ShipLayout.Compartments.Contains(CompartmentID))
	{
		return false;
	}

	FShipCompartment& Compartment = ShipLayout.Compartments[CompartmentID];
	Compartment.Integrity = FMath::Clamp(Compartment.Integrity - Damage, 0.0f, 100.0f);

	// Update status based on integrity
	if (Compartment.Integrity <= 0.0f)
	{
		Compartment.Status = ECompartmentStatus::Destroyed;
	}
	else if (Compartment.Integrity < 25.0f)
	{
		Compartment.Status = ECompartmentStatus::Critical;
	}
	else if (Compartment.Integrity < 75.0f)
	{
		Compartment.Status = ECompartmentStatus::Damaged;
	}

	UE_LOG(LogTemp, Warning, TEXT("Compartment %s damaged! Integrity: %.1f%%"), *CompartmentID, Compartment.Integrity);

	return true;
}

bool UShipInfrastructureManager::RepairCompartment(const FString& CompartmentID, float RepairAmount)
{
	if (!ShipLayout.Compartments.Contains(CompartmentID))
	{
		return false;
	}

	FShipCompartment& Compartment = ShipLayout.Compartments[CompartmentID];
	Compartment.Integrity = FMath::Clamp(Compartment.Integrity + RepairAmount, 0.0f, 100.0f);

	// Update status
	if (Compartment.Integrity > 75.0f)
	{
		Compartment.Status = ECompartmentStatus::Operational;
	}
	else if (Compartment.Integrity > 25.0f)
	{
		Compartment.Status = ECompartmentStatus::Damaged;
	}

	return true;
}

bool UShipInfrastructureManager::IsCompartmentHabitable(const FString& CompartmentID)
{
	FShipCompartment Compartment;
	if (GetCompartment(CompartmentID, Compartment))
	{
		return Compartment.IsHabitable();
	}

	return false;
}

bool UShipInfrastructureManager::SetDoorState(const FString& DoorID, EDoorState NewState)
{
	if (!ShipLayout.Doors.Contains(DoorID))
	{
		return false;
	}

	FShipDoor& Door = ShipLayout.Doors[DoorID];

	if (!Door.CanOperate())
	{
		UE_LOG(LogTemp, Warning, TEXT("Door %s cannot operate (damaged or offline)"), *DoorID);
		return false;
	}

	Door.State = NewState;
	UE_LOG(LogTemp, Log, TEXT("Door %s state changed to %d"), *DoorID, static_cast<int32>(NewState));

	return true;
}

// ===== Electrical Grid =====

bool UShipInfrastructureManager::SetReactorOutput(const FString& ReactorID, float OutputPercent)
{
	if (!ElectricalGrid.Reactors.Contains(ReactorID))
	{
		return false;
	}

	FReactor& Reactor = ElectricalGrid.Reactors[ReactorID];
	Reactor.PowerOutputPercent = FMath::Clamp(OutputPercent, 0.0f, 100.0f);

	UE_LOG(LogTemp, Log, TEXT("Reactor %s output set to %.1f%%"), *ReactorID, OutputPercent);

	return true;
}

bool UShipInfrastructureManager::AddPowerNode(const FPowerNode& Node)
{
	if (ElectricalGrid.Nodes.Contains(Node.NodeID))
	{
		UE_LOG(LogTemp, Warning, TEXT("Power node %s already exists"), *Node.NodeID);
		return false;
	}

	ElectricalGrid.Nodes.Add(Node.NodeID, Node);
	UE_LOG(LogTemp, Log, TEXT("Added power node: %s"), *Node.NodeID);

	return true;
}

bool UShipInfrastructureManager::AddPowerConduit(const FPowerConduit& Conduit)
{
	if (ElectricalGrid.Conduits.Contains(Conduit.ConduitID))
	{
		UE_LOG(LogTemp, Warning, TEXT("Power conduit %s already exists"), *Conduit.ConduitID);
		return false;
	}

	ElectricalGrid.Conduits.Add(Conduit.ConduitID, Conduit);
	UE_LOG(LogTemp, Log, TEXT("Added power conduit: %s"), *Conduit.ConduitID);

	return true;
}

bool UShipInfrastructureManager::DamagePowerNode(const FString& NodeID, float Damage)
{
	if (!ElectricalGrid.Nodes.Contains(NodeID))
	{
		return false;
	}

	FPowerNode& Node = ElectricalGrid.Nodes[NodeID];
	Node.Health = FMath::Clamp(Node.Health - Damage, 0.0f, 100.0f);

	if (Node.Health <= 0.0f)
	{
		Node.Status = EPowerNodeStatus::Offline;
	}
	else if (Node.Health < 50.0f)
	{
		Node.Status = EPowerNodeStatus::Damaged;
	}

	return true;
}

bool UShipInfrastructureManager::DamagePowerConduit(const FString& ConduitID, float Damage)
{
	if (!ElectricalGrid.Conduits.Contains(ConduitID))
	{
		return false;
	}

	FPowerConduit& Conduit = ElectricalGrid.Conduits[ConduitID];
	Conduit.Health = FMath::Clamp(Conduit.Health - Damage, 0.0f, 100.0f);

	if (Conduit.Health <= 0.0f)
	{
		Conduit.bIsDamaged = true;
	}

	return true;
}

// ===== Hardpoints =====

bool UShipInfrastructureManager::GetHardpoint(const FString& HardpointID, FHardpoint& OutHardpoint)
{
	if (HardpointLayout.Hardpoints.Contains(HardpointID))
	{
		OutHardpoint = HardpointLayout.Hardpoints[HardpointID];
		return true;
	}

	return false;
}

bool UShipInfrastructureManager::MountEquipment(const FString& HardpointID, const FMountedEquipment& Equipment)
{
	if (!HardpointLayout.Hardpoints.Contains(HardpointID))
	{
		return false;
	}

	FHardpoint& Hardpoint = HardpointLayout.Hardpoints[HardpointID];

	if (!Hardpoint.CanMount(Equipment))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot mount equipment on hardpoint %s"), *HardpointID);
		return false;
	}

	Hardpoint.MountedEquipment = Equipment;
	Hardpoint.Status = EHardpointStatus::Occupied;
	HardpointLayout.OccupiedHardpoints++;

	UE_LOG(LogTemp, Log, TEXT("Mounted %s on hardpoint %s"), *Equipment.EquipmentName, *HardpointID);

	return true;
}

bool UShipInfrastructureManager::UnmountEquipment(const FString& HardpointID)
{
	if (!HardpointLayout.Hardpoints.Contains(HardpointID))
	{
		return false;
	}

	FHardpoint& Hardpoint = HardpointLayout.Hardpoints[HardpointID];

	if (Hardpoint.Status != EHardpointStatus::Occupied)
	{
		return false;
	}

	Hardpoint.MountedEquipment = FMountedEquipment();
	Hardpoint.Status = EHardpointStatus::Empty;
	HardpointLayout.OccupiedHardpoints--;

	UE_LOG(LogTemp, Log, TEXT("Unmounted equipment from hardpoint %s"), *HardpointID);

	return true;
}

bool UShipInfrastructureManager::DamageEquipment(const FString& HardpointID, float Damage)
{
	if (!HardpointLayout.Hardpoints.Contains(HardpointID))
	{
		return false;
	}

	FHardpoint& Hardpoint = HardpointLayout.Hardpoints[HardpointID];

	if (Hardpoint.Status != EHardpointStatus::Occupied)
	{
		return false;
	}

	Hardpoint.MountedEquipment.Health = FMath::Clamp(Hardpoint.MountedEquipment.Health - Damage, 0.0f, 100.0f);

	if (Hardpoint.MountedEquipment.Health <= 0.0f)
	{
		Hardpoint.MountedEquipment.State = EEquipmentState::Destroyed;
	}
	else if (Hardpoint.MountedEquipment.Health < 50.0f)
	{
		Hardpoint.MountedEquipment.State = EEquipmentState::Damaged;
	}

	return true;
}

TArray<FHardpoint> UShipInfrastructureManager::GetAvailableHardpoints(EHardpointSize MinSize)
{
	TArray<FHardpoint> Available;

	for (const auto& Pair : HardpointLayout.Hardpoints)
	{
		const FHardpoint& Hardpoint = Pair.Value;

		if (Hardpoint.IsEmpty() && Hardpoint.Size >= MinSize && Hardpoint.Integrity > 50.0f)
		{
			Available.Add(Hardpoint);
		}
	}

	return Available;
}

// ===== Resource Networks =====

bool UShipInfrastructureManager::AddResourceNode(const FResourceNode& Node)
{
	if (ResourceNetwork.Nodes.Contains(Node.NodeID))
	{
		return false;
	}

	ResourceNetwork.Nodes.Add(Node.NodeID, Node);
	return true;
}

bool UShipInfrastructureManager::AddResourcePipe(const FResourcePipe& Pipe)
{
	if (ResourceNetwork.Pipes.Contains(Pipe.PipeID))
	{
		return false;
	}

	ResourceNetwork.Pipes.Add(Pipe.PipeID, Pipe);
	return true;
}

bool UShipInfrastructureManager::DamageResourceNode(const FString& NodeID, float Damage)
{
	// TODO: Implement
	return false;
}

bool UShipInfrastructureManager::DamageResourcePipe(const FString& PipeID, float Damage)
{
	if (!ResourceNetwork.Pipes.Contains(PipeID))
	{
		return false;
	}

	FResourcePipe& Pipe = ResourceNetwork.Pipes[PipeID];
	Pipe.Health = FMath::Clamp(Pipe.Health - Damage, 0.0f, 100.0f);

	if (Pipe.Health < 50.0f)
	{
		Pipe.bIsLeaking = true;
		Pipe.LeakRate = (100.0f - Pipe.Health) / 100.0f;
		ResourceNetwork.LeakingPipes++;
	}

	return true;
}

float UShipInfrastructureManager::GetOxygenLevel(const FString& CompartmentID)
{
	FShipCompartment Compartment;
	if (GetCompartment(CompartmentID, Compartment))
	{
		return Compartment.Atmosphere.OxygenLevel;
	}

	return 0.0f;
}

bool UShipInfrastructureManager::IsLifeSupportOperational() const
{
	return ResourceNetwork.LifeSupport.bIsOperational;
}

// ===== Update Functions =====

void UShipInfrastructureManager::UpdateElectricalGrid(float DeltaTime)
{
	// Calculate power generation
	ElectricalGrid.TotalGeneration = 0.0f;
	for (auto& Pair : ElectricalGrid.Reactors)
	{
		FReactor& Reactor = Pair.Value;

		if (ElectricalGrid.Nodes.Contains(Reactor.PowerNodeID))
		{
			FPowerNode& Node = ElectricalGrid.Nodes[Reactor.PowerNodeID];
			float Efficiency = Reactor.GetEfficiency();
			Node.PowerGeneration = Node.MaxPowerGeneration * Efficiency;
			ElectricalGrid.TotalGeneration += Node.PowerGeneration;
		}
	}

	// Calculate power consumption
	ElectricalGrid.TotalConsumption = 0.0f;
	for (auto& Pair : ElectricalGrid.Nodes)
	{
		if (Pair.Value.IsConsumer())
		{
			ElectricalGrid.TotalConsumption += Pair.Value.PowerConsumption;
		}
	}

	// Distribute power
	DistributePower();
}

void UShipInfrastructureManager::UpdateResourceNetworks(float DeltaTime)
{
	// Simulate oxygen flow
	SimulateResourceFlow(EResourceType::Oxygen);

	// Simulate coolant flow
	SimulateResourceFlow(EResourceType::Coolant);

	// Update life support
	UpdateLifeSupport(DeltaTime);

	// Update coolant system
	UpdateCoolantSystem(DeltaTime);
}

void UShipInfrastructureManager::UpdateCompartments(float DeltaTime)
{
	// Update atmosphere in each compartment
	CalculateAtmosphereChanges(DeltaTime);

	// Propagate fire damage
	PropagateFireDamage(DeltaTime);

	// Handle atmosphere loss from breaches
	PropagateAtmosphereLoss(DeltaTime);
}

void UShipInfrastructureManager::UpdateHardpoints(float DeltaTime)
{
	// Update equipment heat
	for (auto& Pair : HardpointLayout.Hardpoints)
	{
		FHardpoint& Hardpoint = Pair.Value;

		if (Hardpoint.HasMountedEquipment())
		{
			FMountedEquipment& Equipment = Hardpoint.MountedEquipment;

			// Cool down equipment
			if (Equipment.CurrentHeat > 0.0f)
			{
				float CoolingRate = Equipment.bRequiresCoolant ? Equipment.CoolantFlow * 2.0f : 1.0f;
				Equipment.CurrentHeat = FMath::Max(0.0f, Equipment.CurrentHeat - CoolingRate * DeltaTime);
			}

			// Check overheating
			if (Equipment.IsOverheating())
			{
				Equipment.State = EEquipmentState::Overheating;
			}
		}
	}
}

void UShipInfrastructureManager::UpdateReactors(float DeltaTime)
{
	for (auto& Pair : ElectricalGrid.Reactors)
	{
		FReactor& Reactor = Pair.Value;

		if (Reactor.bEmergencyShutdown)
		{
			continue;
		}

		// Consume fuel
		if (Reactor.FuelLevel > 0.0f)
		{
			Reactor.FuelLevel = FMath::Max(0.0f, Reactor.FuelLevel - Reactor.FuelConsumptionRate * DeltaTime);
		}

		// Update temperature based on power output
		float TargetTemp = Reactor.OptimalTemperature * (Reactor.PowerOutputPercent / 100.0f);
		float TempDelta = TargetTemp - Reactor.Temperature;
		Reactor.Temperature += TempDelta * 0.1f * DeltaTime; // Gradual temperature change

		// Cool with coolant if available
		if (Reactor.bRequiresCoolant && Reactor.CoolantLevel > 0.0f)
		{
			float CoolingEffect = 100.0f * DeltaTime;
			Reactor.Temperature = FMath::Max(20.0f, Reactor.Temperature - CoolingEffect);
		}

		// Emergency shutdown if critical
		if (Reactor.IsCritical())
		{
			Reactor.bEmergencyShutdown = true;
			UE_LOG(LogTemp, Error, TEXT("Reactor %s emergency shutdown due to critical temperature!"), *Pair.Key);
		}
	}
}

void UShipInfrastructureManager::UpdateLifeSupport(float DeltaTime)
{
	// TODO: Implement life support simulation
}

void UShipInfrastructureManager::UpdateCoolantSystem(float DeltaTime)
{
	// TODO: Implement coolant system simulation
}

void UShipInfrastructureManager::DistributePower()
{
	// Simple power distribution - prioritize critical systems
	// TODO: Implement full power distribution algorithm with priorities
}

void UShipInfrastructureManager::CalculatePowerFlow()
{
	// TODO: Implement power flow calculations through conduits
}

void UShipInfrastructureManager::HandlePowerPriorities()
{
	// TODO: Implement priority-based power allocation
}

void UShipInfrastructureManager::SimulateResourceFlow(EResourceType ResourceType)
{
	// TODO: Implement resource flow simulation
}

void UShipInfrastructureManager::CalculateAtmosphereChanges(float DeltaTime)
{
	// TODO: Implement atmosphere simulation
}

void UShipInfrastructureManager::PropagateFireDamage(float DeltaTime)
{
	// TODO: Implement fire propagation
}

void UShipInfrastructureManager::PropagateAtmosphereLoss(float DeltaTime)
{
	// TODO: Implement breach/decompression simulation
}

// ===== Initialization =====

void UShipInfrastructureManager::InitializeDefaultShipLayout()
{
	// TODO: Implement default ship layout generation
}

void UShipInfrastructureManager::InitializeStartingShip()
{
	UE_LOG(LogTemp, Log, TEXT("ShipInfrastructureManager: Initializing starting ship"));

	// Initialize ship layout
	ShipLayout.ShipName = TEXT("Colony Ship");
	ShipLayout.ShipClass = TEXT("Generation Ship");

	// TODO: Create default compartments, power grid, hardpoints, etc.

	UE_LOG(LogTemp, Log, TEXT("ShipInfrastructureManager: Starting ship initialized"));
}

// ===== Serialization =====

FString UShipInfrastructureManager::SerializeToJSON()
{
	// TODO: Implement full JSON serialization
	return TEXT("{}");
}

bool UShipInfrastructureManager::DeserializeFromJSON(const FString& JSON)
{
	// TODO: Implement JSON deserialization
	return false;
}
