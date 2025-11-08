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
	ShipLayout.ShipName = TEXT("Endeavour");
	ShipLayout.ShipClass = TEXT("Colony Vessel - Class A");
	ShipLayout.TotalCompartments = 8;
	ShipLayout.TotalDecks = 3;
	ShipLayout.OverallIntegrity = 100.0f;

	// ===== Create Compartments =====

	// Deck 1 - Command & Control
	FShipCompartment Bridge;
	Bridge.CompartmentID = TEXT("Bridge");
	Bridge.CompartmentName = TEXT("Bridge");
	Bridge.Type = ECompartmentType::Bridge;
	Bridge.DeckNumber = 1;
	Bridge.Status = ECompartmentStatus::Operational;
	Bridge.Integrity = 100.0f;
	Bridge.Volume = 150.0f;
	Bridge.MaxCrewCapacity = 5;
	Bridge.CurrentCrewCount = 3;
	Bridge.Atmosphere.OxygenLevel = 100.0f;
	Bridge.Atmosphere.Pressure = 100.0f;
	Bridge.Atmosphere.Temperature = 22.0f;
	ShipLayout.Compartments.Add(Bridge.CompartmentID, Bridge);

	FShipCompartment LifeSupport;
	LifeSupport.CompartmentID = TEXT("LifeSupport");
	LifeSupport.CompartmentName = TEXT("Life Support Systems");
	LifeSupport.Type = ECompartmentType::LifeSupport;
	LifeSupport.DeckNumber = 1;
	LifeSupport.Status = ECompartmentStatus::Operational;
	LifeSupport.Integrity = 100.0f;
	LifeSupport.Volume = 200.0f;
	LifeSupport.MaxCrewCapacity = 2;
	LifeSupport.CurrentCrewCount = 1;
	LifeSupport.Atmosphere.OxygenLevel = 100.0f;
	LifeSupport.Atmosphere.Pressure = 100.0f;
	LifeSupport.Atmosphere.Temperature = 20.0f;
	ShipLayout.Compartments.Add(LifeSupport.CompartmentID, LifeSupport);

	// Deck 2 - Engineering & Power
	FShipCompartment Engineering;
	Engineering.CompartmentID = TEXT("Engineering");
	Engineering.CompartmentName = TEXT("Main Engineering");
	Engineering.Type = ECompartmentType::Engineering;
	Engineering.DeckNumber = 2;
	Engineering.Status = ECompartmentStatus::Operational;
	Engineering.Integrity = 100.0f;
	Engineering.Volume = 300.0f;
	Engineering.MaxCrewCapacity = 4;
	Engineering.CurrentCrewCount = 2;
	Engineering.Atmosphere.OxygenLevel = 100.0f;
	Engineering.Atmosphere.Pressure = 100.0f;
	Engineering.Atmosphere.Temperature = 25.0f;
	ShipLayout.Compartments.Add(Engineering.CompartmentID, Engineering);

	FShipCompartment ReactorRoom;
	ReactorRoom.CompartmentID = TEXT("ReactorRoom");
	ReactorRoom.CompartmentName = TEXT("Reactor Chamber");
	ReactorRoom.Type = ECompartmentType::PowerPlant;
	ReactorRoom.DeckNumber = 2;
	ReactorRoom.Status = ECompartmentStatus::Operational;
	ReactorRoom.Integrity = 100.0f;
	ReactorRoom.Volume = 250.0f;
	ReactorRoom.MaxCrewCapacity = 2;
	ReactorRoom.CurrentCrewCount = 1;
	ReactorRoom.Atmosphere.OxygenLevel = 100.0f;
	ReactorRoom.Atmosphere.Pressure = 100.0f;
	ReactorRoom.Atmosphere.Temperature = 30.0f;
	ShipLayout.Compartments.Add(ReactorRoom.CompartmentID, ReactorRoom);

	FShipCompartment CargoBay;
	CargoBay.CompartmentID = TEXT("CargoBay");
	CargoBay.CompartmentName = TEXT("Main Cargo Bay");
	CargoBay.Type = ECompartmentType::CargoBay;
	CargoBay.DeckNumber = 2;
	CargoBay.Status = ECompartmentStatus::Operational;
	CargoBay.Integrity = 100.0f;
	CargoBay.Volume = 500.0f;
	CargoBay.MaxCrewCapacity = 3;
	CargoBay.CurrentCrewCount = 0;
	CargoBay.Atmosphere.OxygenLevel = 100.0f;
	CargoBay.Atmosphere.Pressure = 100.0f;
	CargoBay.Atmosphere.Temperature = 18.0f;
	ShipLayout.Compartments.Add(CargoBay.CompartmentID, CargoBay);

	// Deck 3 - Habitation
	FShipCompartment CrewQuarters;
	CrewQuarters.CompartmentID = TEXT("CrewQuarters");
	CrewQuarters.CompartmentName = TEXT("Crew Quarters");
	CrewQuarters.Type = ECompartmentType::CrewQuarters;
	CrewQuarters.DeckNumber = 3;
	CrewQuarters.Status = ECompartmentStatus::Operational;
	CrewQuarters.Integrity = 100.0f;
	CrewQuarters.Volume = 400.0f;
	CrewQuarters.MaxCrewCapacity = 20;
	CrewQuarters.CurrentCrewCount = 15;
	CrewQuarters.Atmosphere.OxygenLevel = 100.0f;
	CrewQuarters.Atmosphere.Pressure = 100.0f;
	CrewQuarters.Atmosphere.Temperature = 21.0f;
	ShipLayout.Compartments.Add(CrewQuarters.CompartmentID, CrewQuarters);

	FShipCompartment MedBay;
	MedBay.CompartmentID = TEXT("MedBay");
	MedBay.CompartmentName = TEXT("Medical Bay");
	MedBay.Type = ECompartmentType::MedicalBay;
	MedBay.DeckNumber = 3;
	MedBay.Status = ECompartmentStatus::Operational;
	MedBay.Integrity = 100.0f;
	MedBay.Volume = 180.0f;
	MedBay.MaxCrewCapacity = 4;
	MedBay.CurrentCrewCount = 2;
	MedBay.Atmosphere.OxygenLevel = 100.0f;
	MedBay.Atmosphere.Pressure = 100.0f;
	MedBay.Atmosphere.Temperature = 20.0f;
	ShipLayout.Compartments.Add(MedBay.CompartmentID, MedBay);

	FShipCompartment ResearchLab;
	ResearchLab.CompartmentID = TEXT("ResearchLab");
	ResearchLab.CompartmentName = TEXT("Research Laboratory");
	ResearchLab.Type = ECompartmentType::Laboratory;
	ResearchLab.DeckNumber = 3;
	ResearchLab.Status = ECompartmentStatus::Operational;
	ResearchLab.Integrity = 100.0f;
	ResearchLab.Volume = 220.0f;
	ResearchLab.MaxCrewCapacity = 6;
	ResearchLab.CurrentCrewCount = 3;
	ResearchLab.Atmosphere.OxygenLevel = 100.0f;
	ResearchLab.Atmosphere.Pressure = 100.0f;
	ResearchLab.Atmosphere.Temperature = 22.0f;
	ShipLayout.Compartments.Add(ResearchLab.CompartmentID, ResearchLab);

	// Create connecting doors
	FShipDoor Door1;
	Door1.DoorID = TEXT("Door_Bridge_LifeSupport");
	Door1.ConnectedCompartments.Add(TEXT("Bridge"));
	Door1.ConnectedCompartments.Add(TEXT("LifeSupport"));
	Door1.State = EDoorState::Closed;
	Door1.Health = 100.0f;
	ShipLayout.Doors.Add(Door1.DoorID, Door1);

	FShipDoor Door2;
	Door2.DoorID = TEXT("Door_Engineering_Reactor");
	Door2.ConnectedCompartments.Add(TEXT("Engineering"));
	Door2.ConnectedCompartments.Add(TEXT("ReactorRoom"));
	Door2.State = EDoorState::Closed;
	Door2.Health = 100.0f;
	ShipLayout.Doors.Add(Door2.DoorID, Door2);

	FShipDoor Door3;
	Door3.DoorID = TEXT("Door_CrewQuarters_MedBay");
	Door3.ConnectedCompartments.Add(TEXT("CrewQuarters"));
	Door3.ConnectedCompartments.Add(TEXT("MedBay"));
	Door3.State = EDoorState::Closed;
	Door3.Health = 100.0f;
	ShipLayout.Doors.Add(Door3.DoorID, Door3);

	TotalCrewMembers = 27;

	// ===== Create Power Grid =====

	// Main fusion reactor
	FReactor MainReactor;
	MainReactor.PowerNodeID = TEXT("Node_Reactor_Main");
	MainReactor.ReactorType = EReactorType::Fusion;
	MainReactor.FuelLevel = 85.0f;
	MainReactor.FuelConsumptionRate = 0.05f;
	MainReactor.Temperature = 1050.0f;
	MainReactor.OptimalTemperature = 1000.0f;
	MainReactor.MaxSafeTemperature = 2000.0f;
	MainReactor.PowerOutputPercent = 80.0f;
	MainReactor.bRequiresCoolant = true;
	MainReactor.CoolantLevel = 90.0f;
	ElectricalGrid.Reactors.Add(TEXT("Reactor_Main"), MainReactor);

	// Reactor power node
	FPowerNode ReactorNode;
	ReactorNode.NodeID = TEXT("Node_Reactor_Main");
	ReactorNode.NodeName = TEXT("Main Reactor");
	ReactorNode.Type = EPowerNodeType::Reactor;
	ReactorNode.Status = EPowerNodeStatus::Operational;
	ReactorNode.CompartmentID = TEXT("ReactorRoom");
	ReactorNode.MaxPowerGeneration = 500.0f;
	ReactorNode.PowerGeneration = 400.0f; // 80% output
	ReactorNode.Health = 100.0f;
	ReactorNode.Priority = 10;
	ElectricalGrid.Nodes.Add(ReactorNode.NodeID, ReactorNode);

	// Engineering distributor
	FPowerNode EngDistributor;
	EngDistributor.NodeID = TEXT("Node_Eng_Distributor");
	EngDistributor.NodeName = TEXT("Engineering Distributor");
	EngDistributor.Type = EPowerNodeType::Distributor;
	EngDistributor.Status = EPowerNodeStatus::Operational;
	EngDistributor.CompartmentID = TEXT("Engineering");
	EngDistributor.MaxThroughput = 300.0f;
	EngDistributor.Health = 100.0f;
	EngDistributor.Priority = 9;
	ElectricalGrid.Nodes.Add(EngDistributor.NodeID, EngDistributor);

	// Bridge consumer
	FPowerNode BridgeConsumer;
	BridgeConsumer.NodeID = TEXT("Node_Bridge");
	BridgeConsumer.NodeName = TEXT("Bridge Systems");
	BridgeConsumer.Type = EPowerNodeType::Consumer;
	BridgeConsumer.Status = EPowerNodeStatus::Operational;
	BridgeConsumer.CompartmentID = TEXT("Bridge");
	BridgeConsumer.RequiredPower = 50.0f;
	BridgeConsumer.PowerConsumption = 50.0f;
	BridgeConsumer.Health = 100.0f;
	BridgeConsumer.Priority = 10;
	ElectricalGrid.Nodes.Add(BridgeConsumer.NodeID, BridgeConsumer);

	// Life support consumer
	FPowerNode LifeSupportConsumer;
	LifeSupportConsumer.NodeID = TEXT("Node_LifeSupport");
	LifeSupportConsumer.NodeName = TEXT("Life Support Systems");
	LifeSupportConsumer.Type = EPowerNodeType::Consumer;
	LifeSupportConsumer.Status = EPowerNodeStatus::Operational;
	LifeSupportConsumer.CompartmentID = TEXT("LifeSupport");
	LifeSupportConsumer.RequiredPower = 80.0f;
	LifeSupportConsumer.PowerConsumption = 80.0f;
	LifeSupportConsumer.Health = 100.0f;
	LifeSupportConsumer.Priority = 10; // Critical
	ElectricalGrid.Nodes.Add(LifeSupportConsumer.NodeID, LifeSupportConsumer);

	// Crew quarters consumer
	FPowerNode CrewConsumer;
	CrewConsumer.NodeID = TEXT("Node_CrewQuarters");
	CrewConsumer.NodeName = TEXT("Crew Quarters");
	CrewConsumer.Type = EPowerNodeType::Consumer;
	CrewConsumer.Status = EPowerNodeStatus::Operational;
	CrewConsumer.CompartmentID = TEXT("CrewQuarters");
	CrewConsumer.RequiredPower = 40.0f;
	CrewConsumer.PowerConsumption = 40.0f;
	CrewConsumer.Health = 100.0f;
	CrewConsumer.Priority = 8;
	ElectricalGrid.Nodes.Add(CrewConsumer.NodeID, CrewConsumer);

	// Medical bay consumer
	FPowerNode MedBayConsumer;
	MedBayConsumer.NodeID = TEXT("Node_MedBay");
	MedBayConsumer.NodeName = TEXT("Medical Bay");
	MedBayConsumer.Type = EPowerNodeType::Consumer;
	MedBayConsumer.Status = EPowerNodeStatus::Operational;
	MedBayConsumer.CompartmentID = TEXT("MedBay");
	MedBayConsumer.RequiredPower = 35.0f;
	MedBayConsumer.PowerConsumption = 35.0f;
	MedBayConsumer.Health = 100.0f;
	MedBayConsumer.Priority = 9;
	ElectricalGrid.Nodes.Add(MedBayConsumer.NodeID, MedBayConsumer);

	// Research lab consumer
	FPowerNode LabConsumer;
	LabConsumer.NodeID = TEXT("Node_ResearchLab");
	LabConsumer.NodeName = TEXT("Research Lab");
	LabConsumer.Type = EPowerNodeType::Consumer;
	LabConsumer.Status = EPowerNodeStatus::Operational;
	LabConsumer.CompartmentID = TEXT("ResearchLab");
	LabConsumer.RequiredPower = 60.0f;
	LabConsumer.PowerConsumption = 60.0f;
	LabConsumer.Health = 100.0f;
	LabConsumer.Priority = 6;
	ElectricalGrid.Nodes.Add(LabConsumer.NodeID, LabConsumer);

	// Cargo bay consumer
	FPowerNode CargoConsumer;
	CargoConsumer.NodeID = TEXT("Node_CargoBay");
	CargoConsumer.NodeName = TEXT("Cargo Bay");
	CargoConsumer.Type = EPowerNodeType::Consumer;
	CargoConsumer.Status = EPowerNodeStatus::Operational;
	CargoConsumer.CompartmentID = TEXT("CargoBay");
	CargoConsumer.RequiredPower = 20.0f;
	CargoConsumer.PowerConsumption = 20.0f;
	CargoConsumer.Health = 100.0f;
	CargoConsumer.Priority = 5;
	ElectricalGrid.Nodes.Add(CargoConsumer.NodeID, CargoConsumer);

	// Create power conduits
	FPowerConduit Conduit1;
	Conduit1.ConduitID = TEXT("Conduit_Reactor_Eng");
	Conduit1.SourceNodeID = TEXT("Node_Reactor_Main");
	Conduit1.DestinationNodeID = TEXT("Node_Eng_Distributor");
	Conduit1.MaxCapacity = 500.0f;
	Conduit1.CurrentFlow = 385.0f;
	Conduit1.Efficiency = 0.98f;
	Conduit1.Health = 100.0f;
	ElectricalGrid.Conduits.Add(Conduit1.ConduitID, Conduit1);

	FPowerConduit Conduit2;
	Conduit2.ConduitID = TEXT("Conduit_Eng_Bridge");
	Conduit2.SourceNodeID = TEXT("Node_Eng_Distributor");
	Conduit2.DestinationNodeID = TEXT("Node_Bridge");
	Conduit2.MaxCapacity = 100.0f;
	Conduit2.CurrentFlow = 50.0f;
	Conduit2.Efficiency = 0.95f;
	Conduit2.Health = 100.0f;
	ElectricalGrid.Conduits.Add(Conduit2.ConduitID, Conduit2);

	FPowerConduit Conduit3;
	Conduit3.ConduitID = TEXT("Conduit_Eng_LifeSupport");
	Conduit3.SourceNodeID = TEXT("Node_Eng_Distributor");
	Conduit3.DestinationNodeID = TEXT("Node_LifeSupport");
	Conduit3.MaxCapacity = 120.0f;
	Conduit3.CurrentFlow = 80.0f;
	Conduit3.Efficiency = 0.96f;
	Conduit3.Health = 100.0f;
	ElectricalGrid.Conduits.Add(Conduit3.ConduitID, Conduit3);

	FPowerConduit Conduit4;
	Conduit4.ConduitID = TEXT("Conduit_Eng_CrewQuarters");
	Conduit4.SourceNodeID = TEXT("Node_Eng_Distributor");
	Conduit4.DestinationNodeID = TEXT("Node_CrewQuarters");
	Conduit4.MaxCapacity = 80.0f;
	Conduit4.CurrentFlow = 40.0f;
	Conduit4.Efficiency = 0.94f;
	Conduit4.Health = 100.0f;
	ElectricalGrid.Conduits.Add(Conduit4.ConduitID, Conduit4);

	FPowerConduit Conduit5;
	Conduit5.ConduitID = TEXT("Conduit_Eng_MedBay");
	Conduit5.SourceNodeID = TEXT("Node_Eng_Distributor");
	Conduit5.DestinationNodeID = TEXT("Node_MedBay");
	Conduit5.MaxCapacity = 70.0f;
	Conduit5.CurrentFlow = 35.0f;
	Conduit5.Efficiency = 0.95f;
	Conduit5.Health = 100.0f;
	ElectricalGrid.Conduits.Add(Conduit5.ConduitID, Conduit5);

	FPowerConduit Conduit6;
	Conduit6.ConduitID = TEXT("Conduit_Eng_Lab");
	Conduit6.SourceNodeID = TEXT("Node_Eng_Distributor");
	Conduit6.DestinationNodeID = TEXT("Node_ResearchLab");
	Conduit6.MaxCapacity = 100.0f;
	Conduit6.CurrentFlow = 60.0f;
	Conduit6.Efficiency = 0.93f;
	Conduit6.Health = 100.0f;
	ElectricalGrid.Conduits.Add(Conduit6.ConduitID, Conduit6);

	FPowerConduit Conduit7;
	Conduit7.ConduitID = TEXT("Conduit_Eng_Cargo");
	Conduit7.SourceNodeID = TEXT("Node_Eng_Distributor");
	Conduit7.DestinationNodeID = TEXT("Node_CargoBay");
	Conduit7.MaxCapacity = 50.0f;
	Conduit7.CurrentFlow = 20.0f;
	Conduit7.Efficiency = 0.92f;
	Conduit7.Health = 100.0f;
	ElectricalGrid.Conduits.Add(Conduit7.ConduitID, Conduit7);

	// Set grid summary
	ElectricalGrid.TotalGeneration = 400.0f;
	ElectricalGrid.TotalConsumption = 385.0f;
	ElectricalGrid.GridEfficiency = 94.5f;
	ElectricalGrid.OverloadedConduits = 0;
	ElectricalGrid.OfflineNodes = 0;

	UE_LOG(LogTemp, Log, TEXT("ShipInfrastructureManager: Created power grid with %d nodes, %d conduits, %d reactors"),
		ElectricalGrid.Nodes.Num(), ElectricalGrid.Conduits.Num(), ElectricalGrid.Reactors.Num());

	// ===== Create Hardpoints and Equipment =====

	// Forward weapon hardpoint - Light laser
	FHardpoint ForwardWeapon;
	ForwardWeapon.HardpointID = TEXT("HP_Forward_1");
	ForwardWeapon.HardpointName = TEXT("Forward Weapon Mount");
	ForwardWeapon.Size = EHardpointSize::Medium;
	ForwardWeapon.Status = EHardpointStatus::Occupied;
	ForwardWeapon.CompartmentID = TEXT("Bridge");
	ForwardWeapon.Position = FVector(1000.0f, 0.0f, 0.0f);
	ForwardWeapon.PowerNodeID = TEXT("Node_Bridge");
	ForwardWeapon.Integrity = 100.0f;
	ForwardWeapon.MaxLoadCapacity = 2000.0f;
	ForwardWeapon.AllowedTypes = {EEquipmentType::Weapon_Light, EEquipmentType::Weapon_Medium};

	FMountedEquipment LaserWeapon;
	LaserWeapon.EquipmentID = TEXT("EQ_Laser_1");
	LaserWeapon.EquipmentName = TEXT("Pulse Laser Mk I");
	LaserWeapon.Type = EEquipmentType::Weapon_Light;
	LaserWeapon.RequiredSize = EHardpointSize::Medium;
	LaserWeapon.State = EEquipmentState::Operational;
	LaserWeapon.Health = 100.0f;
	LaserWeapon.PowerRequired = 15.0f;
	LaserWeapon.PowerConsumption = 15.0f;
	LaserWeapon.HeatGeneration = 10.0f;
	LaserWeapon.Mass = 500.0f;
	LaserWeapon.PerformanceStats.Add(TEXT("Damage"), 25.0f);
	LaserWeapon.PerformanceStats.Add(TEXT("Range"), 1500.0f);
	LaserWeapon.PerformanceStats.Add(TEXT("FireRate"), 2.0f);
	ForwardWeapon.MountedEquipment = LaserWeapon;
	HardpointLayout.Hardpoints.Add(ForwardWeapon.HardpointID, ForwardWeapon);

	// Point defense hardpoint
	FHardpoint PointDefense;
	PointDefense.HardpointID = TEXT("HP_PointDefense_1");
	PointDefense.HardpointName = TEXT("Point Defense Turret");
	PointDefense.Size = EHardpointSize::Small;
	PointDefense.Status = EHardpointStatus::Occupied;
	PointDefense.CompartmentID = TEXT("Bridge");
	PointDefense.Position = FVector(500.0f, 200.0f, 100.0f);
	PointDefense.PowerNodeID = TEXT("Node_Bridge");
	PointDefense.Integrity = 100.0f;
	PointDefense.MaxLoadCapacity = 500.0f;
	PointDefense.AllowedTypes = {EEquipmentType::Weapon_PointDefense};

	FMountedEquipment PDTurret;
	PDTurret.EquipmentID = TEXT("EQ_PD_1");
	PDTurret.EquipmentName = TEXT("Auto-Targeting PD Cannon");
	PDTurret.Type = EEquipmentType::Weapon_PointDefense;
	PDTurret.RequiredSize = EHardpointSize::Small;
	PDTurret.State = EEquipmentState::Operational;
	PDTurret.Health = 100.0f;
	PDTurret.PowerRequired = 8.0f;
	PDTurret.PowerConsumption = 8.0f;
	PDTurret.HeatGeneration = 5.0f;
	PDTurret.Mass = 200.0f;
	PDTurret.PerformanceStats.Add(TEXT("Damage"), 10.0f);
	PDTurret.PerformanceStats.Add(TEXT("Range"), 500.0f);
	PDTurret.PerformanceStats.Add(TEXT("FireRate"), 10.0f);
	PointDefense.MountedEquipment = PDTurret;
	HardpointLayout.Hardpoints.Add(PointDefense.HardpointID, PointDefense);

	// Sensor array
	FHardpoint SensorHP;
	SensorHP.HardpointID = TEXT("HP_Sensor_1");
	SensorHP.HardpointName = TEXT("Main Sensor Array");
	SensorHP.Size = EHardpointSize::Large;
	SensorHP.Status = EHardpointStatus::Occupied;
	SensorHP.CompartmentID = TEXT("Bridge");
	SensorHP.Position = FVector(800.0f, 0.0f, 200.0f);
	SensorHP.PowerNodeID = TEXT("Node_Bridge");
	SensorHP.Integrity = 100.0f;
	SensorHP.MaxLoadCapacity = 1000.0f;
	SensorHP.AllowedTypes = {EEquipmentType::Sensor_Standard, EEquipmentType::Sensor_Advanced};

	FMountedEquipment SensorArray;
	SensorArray.EquipmentID = TEXT("EQ_Sensor_1");
	SensorArray.EquipmentName = TEXT("Long-Range Scanner");
	SensorArray.Type = EEquipmentType::Sensor_Standard;
	SensorArray.RequiredSize = EHardpointSize::Large;
	SensorArray.State = EEquipmentState::Operational;
	SensorArray.Health = 100.0f;
	SensorArray.PowerRequired = 12.0f;
	SensorArray.PowerConsumption = 12.0f;
	SensorArray.HeatGeneration = 3.0f;
	SensorArray.Mass = 800.0f;
	SensorArray.PerformanceStats.Add(TEXT("Range"), 5000.0f);
	SensorArray.PerformanceStats.Add(TEXT("Accuracy"), 85.0f);
	SensorHP.MountedEquipment = SensorArray;
	HardpointLayout.Hardpoints.Add(SensorHP.HardpointID, SensorHP);

	// Shield generator
	FHardpoint ShieldHP;
	ShieldHP.HardpointID = TEXT("HP_Shield_1");
	ShieldHP.HardpointName = TEXT("Shield Generator Bay");
	ShieldHP.Size = EHardpointSize::Large;
	ShieldHP.Status = EHardpointStatus::Occupied;
	ShieldHP.CompartmentID = TEXT("Engineering");
	ShieldHP.Position = FVector(0.0f, 0.0f, 0.0f);
	ShieldHP.PowerNodeID = TEXT("Node_Eng_Distributor");
	ShieldHP.CoolantNodeID = TEXT("CoolantNode_Eng");
	ShieldHP.Integrity = 100.0f;
	ShieldHP.MaxLoadCapacity = 3000.0f;
	ShieldHP.AllowedTypes = {EEquipmentType::Shield_Generator};

	FMountedEquipment ShieldGen;
	ShieldGen.EquipmentID = TEXT("EQ_Shield_1");
	ShieldGen.EquipmentName = TEXT("Type-II Shield Generator");
	ShieldGen.Type = EEquipmentType::Shield_Generator;
	ShieldGen.RequiredSize = EHardpointSize::Large;
	ShieldGen.State = EEquipmentState::Operational;
	ShieldGen.Health = 100.0f;
	ShieldGen.PowerRequired = 40.0f;
	ShieldGen.PowerConsumption = 40.0f;
	ShieldGen.HeatGeneration = 25.0f;
	ShieldGen.bRequiresCoolant = true;
	ShieldGen.CoolantFlow = 15.0f;
	ShieldGen.Mass = 2500.0f;
	ShieldGen.PerformanceStats.Add(TEXT("ShieldStrength"), 500.0f);
	ShieldGen.PerformanceStats.Add(TEXT("RechargeRate"), 10.0f);
	ShieldHP.MountedEquipment = ShieldGen;
	HardpointLayout.Hardpoints.Add(ShieldHP.HardpointID, ShieldHP);

	// Main engine
	FHardpoint EngineHP;
	EngineHP.HardpointID = TEXT("HP_Engine_Main");
	EngineHP.HardpointName = TEXT("Main Engine Mount");
	EngineHP.Size = EHardpointSize::ExtraLarge;
	EngineHP.Status = EHardpointStatus::Occupied;
	EngineHP.CompartmentID = TEXT("Engineering");
	EngineHP.Position = FVector(-1000.0f, 0.0f, 0.0f);
	EngineHP.PowerNodeID = TEXT("Node_Eng_Distributor");
	EngineHP.CoolantNodeID = TEXT("CoolantNode_Eng");
	EngineHP.Integrity = 100.0f;
	EngineHP.MaxLoadCapacity = 5000.0f;
	EngineHP.AllowedTypes = {EEquipmentType::Engine_Main};

	FMountedEquipment MainEngine;
	MainEngine.EquipmentID = TEXT("EQ_Engine_Main");
	MainEngine.EquipmentName = TEXT("Fusion Drive");
	MainEngine.Type = EEquipmentType::Engine_Main;
	MainEngine.RequiredSize = EHardpointSize::ExtraLarge;
	MainEngine.State = EEquipmentState::Operational;
	MainEngine.Health = 100.0f;
	MainEngine.PowerRequired = 60.0f;
	MainEngine.PowerConsumption = 60.0f;
	MainEngine.HeatGeneration = 40.0f;
	MainEngine.bRequiresCoolant = true;
	MainEngine.CoolantFlow = 25.0f;
	MainEngine.Mass = 4000.0f;
	MainEngine.CrewRequired = 2;
	MainEngine.PerformanceStats.Add(TEXT("Thrust"), 10000.0f);
	MainEngine.PerformanceStats.Add(TEXT("Efficiency"), 80.0f);
	EngineHP.MountedEquipment = MainEngine;
	HardpointLayout.Hardpoints.Add(EngineHP.HardpointID, EngineHP);

	// Empty hardpoint for expansion
	FHardpoint EmptyHP;
	EmptyHP.HardpointID = TEXT("HP_Empty_1");
	EmptyHP.HardpointName = TEXT("Auxiliary Mount");
	EmptyHP.Size = EHardpointSize::Medium;
	EmptyHP.Status = EHardpointStatus::Empty;
	EmptyHP.CompartmentID = TEXT("CargoBay");
	EmptyHP.Position = FVector(0.0f, 500.0f, 0.0f);
	EmptyHP.PowerNodeID = TEXT("Node_CargoBay");
	EmptyHP.Integrity = 100.0f;
	EmptyHP.MaxLoadCapacity = 2000.0f;
	EmptyHP.AllowedTypes = {EEquipmentType::Weapon_Light, EEquipmentType::Utility_MiningLaser,
	                        EEquipmentType::Utility_TractorBeam, EEquipmentType::Utility_RepairDrone};
	HardpointLayout.Hardpoints.Add(EmptyHP.HardpointID, EmptyHP);

	// Update hardpoint summary
	HardpointLayout.TotalHardpoints = HardpointLayout.Hardpoints.Num();
	HardpointLayout.OccupiedHardpoints = 5;
	HardpointLayout.DamagedHardpoints = 0;

	UE_LOG(LogTemp, Log, TEXT("ShipInfrastructureManager: Created %d hardpoints (%d occupied)"),
		HardpointLayout.TotalHardpoints, HardpointLayout.OccupiedHardpoints);

	// ===== Create Resource Networks =====

	// Life Support System
	ResourceNetwork.LifeSupport.bIsOperational = true;
	ResourceNetwork.LifeSupport.OxygenProductionRate = 50.0f;
	ResourceNetwork.LifeSupport.CO2RemovalRate = 45.0f;
	ResourceNetwork.LifeSupport.WaterRecyclingRate = 30.0f;
	ResourceNetwork.LifeSupport.CurrentCrew = 27;
	ResourceNetwork.LifeSupport.MaxCrewCapacity = 40;
	ResourceNetwork.LifeSupport.OxygenGenerators = 2;
	ResourceNetwork.LifeSupport.CO2Scrubbers = 2;
	ResourceNetwork.LifeSupport.WaterRecyclers = 1;

	// Coolant System
	ResourceNetwork.Coolant.bIsOperational = true;
	ResourceNetwork.Coolant.CoolantLevel = 90.0f;
	ResourceNetwork.Coolant.MaxCoolantCapacity = 100.0f;
	ResourceNetwork.Coolant.HeatDissipationRate = 100.0f;
	ResourceNetwork.Coolant.CurrentHeatLoad = 65.0f; // From shield + engine
	ResourceNetwork.Coolant.HeatExchangers = 3;
	ResourceNetwork.Coolant.Radiators = 4;
	ResourceNetwork.Coolant.RadiatorEfficiency = 95.0f;

	// Resource nodes - Life Support
	FResourceNode O2Generator;
	O2Generator.NodeID = TEXT("ResNode_O2Gen_1");
	O2Generator.NodeName = TEXT("Oxygen Generator #1");
	O2Generator.Type = EResourceType::Oxygen;
	O2Generator.CompartmentID = TEXT("LifeSupport");
	O2Generator.ProductionRate = 25.0f;
	O2Generator.ConsumptionRate = 0.0f;
	O2Generator.StorageCapacity = 500.0f;
	O2Generator.CurrentStorage = 450.0f;
	O2Generator.Health = 100.0f;
	O2Generator.bIsOperational = true;
	ResourceNetwork.Nodes.Add(O2Generator.NodeID, O2Generator);

	FResourceNode O2Generator2;
	O2Generator2.NodeID = TEXT("ResNode_O2Gen_2");
	O2Generator2.NodeName = TEXT("Oxygen Generator #2");
	O2Generator2.Type = EResourceType::Oxygen;
	O2Generator2.CompartmentID = TEXT("LifeSupport");
	O2Generator2.ProductionRate = 25.0f;
	O2Generator2.ConsumptionRate = 0.0f;
	O2Generator2.StorageCapacity = 500.0f;
	O2Generator2.CurrentStorage = 480.0f;
	O2Generator2.Health = 100.0f;
	O2Generator2.bIsOperational = true;
	ResourceNetwork.Nodes.Add(O2Generator2.NodeID, O2Generator2);

	// Resource nodes - Coolant
	FResourceNode CoolantNode1;
	CoolantNode1.NodeID = TEXT("CoolantNode_Eng");
	CoolantNode1.NodeName = TEXT("Engineering Coolant Hub");
	CoolantNode1.Type = EResourceType::Coolant;
	CoolantNode1.CompartmentID = TEXT("Engineering");
	CoolantNode1.ProductionRate = 0.0f;
	CoolantNode1.ConsumptionRate = 40.0f; // Shield + Engine
	CoolantNode1.StorageCapacity = 1000.0f;
	CoolantNode1.CurrentStorage = 900.0f;
	CoolantNode1.Health = 100.0f;
	CoolantNode1.bIsOperational = true;
	ResourceNetwork.Nodes.Add(CoolantNode1.NodeID, CoolantNode1);

	FResourceNode CoolantNode2;
	CoolantNode2.NodeID = TEXT("CoolantNode_Reactor");
	CoolantNode2.NodeName = TEXT("Reactor Coolant System");
	CoolantNode2.Type = EResourceType::Coolant;
	CoolantNode2.CompartmentID = TEXT("ReactorRoom");
	CoolantNode2.ProductionRate = 0.0f;
	CoolantNode2.ConsumptionRate = 25.0f; // Reactor cooling
	CoolantNode2.StorageCapacity = 1500.0f;
	CoolantNode2.CurrentStorage = 1350.0f;
	CoolantNode2.Health = 100.0f;
	CoolantNode2.bIsOperational = true;
	ResourceNetwork.Nodes.Add(CoolantNode2.NodeID, CoolantNode2);

	// Resource pipes - Oxygen distribution
	FResourcePipe O2Pipe1;
	O2Pipe1.PipeID = TEXT("Pipe_O2_Bridge");
	O2Pipe1.ResourceType = EResourceType::Oxygen;
	O2Pipe1.SourceNodeID = TEXT("ResNode_O2Gen_1");
	O2Pipe1.DestinationNodeID = TEXT("Bridge");
	O2Pipe1.MaxFlowRate = 10.0f;
	O2Pipe1.CurrentFlowRate = 5.0f;
	O2Pipe1.Pressure = 100.0f;
	O2Pipe1.Health = 100.0f;
	O2Pipe1.bIsLeaking = false;
	ResourceNetwork.Pipes.Add(O2Pipe1.PipeID, O2Pipe1);

	FResourcePipe O2Pipe2;
	O2Pipe2.PipeID = TEXT("Pipe_O2_CrewQuarters");
	O2Pipe2.ResourceType = EResourceType::Oxygen;
	O2Pipe2.SourceNodeID = TEXT("ResNode_O2Gen_2");
	O2Pipe2.DestinationNodeID = TEXT("CrewQuarters");
	O2Pipe2.MaxFlowRate = 20.0f;
	O2Pipe2.CurrentFlowRate = 15.0f;
	O2Pipe2.Pressure = 100.0f;
	O2Pipe2.Health = 100.0f;
	O2Pipe2.bIsLeaking = false;
	ResourceNetwork.Pipes.Add(O2Pipe2.PipeID, O2Pipe2);

	// Resource pipes - Coolant distribution
	FResourcePipe CoolantPipe1;
	CoolantPipe1.PipeID = TEXT("Pipe_Coolant_Shield");
	CoolantPipe1.ResourceType = EResourceType::Coolant;
	CoolantPipe1.SourceNodeID = TEXT("CoolantNode_Eng");
	CoolantPipe1.DestinationNodeID = TEXT("HP_Shield_1");
	CoolantPipe1.MaxFlowRate = 20.0f;
	CoolantPipe1.CurrentFlowRate = 15.0f;
	CoolantPipe1.Pressure = 95.0f;
	CoolantPipe1.Health = 100.0f;
	CoolantPipe1.bIsLeaking = false;
	ResourceNetwork.Pipes.Add(CoolantPipe1.PipeID, CoolantPipe1);

	FResourcePipe CoolantPipe2;
	CoolantPipe2.PipeID = TEXT("Pipe_Coolant_Engine");
	CoolantPipe2.ResourceType = EResourceType::Coolant;
	CoolantPipe2.SourceNodeID = TEXT("CoolantNode_Eng");
	CoolantPipe2.DestinationNodeID = TEXT("HP_Engine_Main");
	CoolantPipe2.MaxFlowRate = 30.0f;
	CoolantPipe2.CurrentFlowRate = 25.0f;
	CoolantPipe2.Pressure = 95.0f;
	CoolantPipe2.Health = 100.0f;
	CoolantPipe2.bIsLeaking = false;
	ResourceNetwork.Pipes.Add(CoolantPipe2.PipeID, CoolantPipe2);

	FResourcePipe CoolantPipe3;
	CoolantPipe3.PipeID = TEXT("Pipe_Coolant_Reactor");
	CoolantPipe3.ResourceType = EResourceType::Coolant;
	CoolantPipe3.SourceNodeID = TEXT("CoolantNode_Reactor");
	CoolantPipe3.DestinationNodeID = TEXT("Reactor_Main");
	CoolantPipe3.MaxFlowRate = 50.0f;
	CoolantPipe3.CurrentFlowRate = 40.0f;
	CoolantPipe3.Pressure = 98.0f;
	CoolantPipe3.Health = 100.0f;
	CoolantPipe3.bIsLeaking = false;
	ResourceNetwork.Pipes.Add(CoolantPipe3.PipeID, CoolantPipe3);

	// Resource network summary
	ResourceNetwork.LeakingPipes = 0;
	ResourceNetwork.TotalNodes = ResourceNetwork.Nodes.Num();
	ResourceNetwork.TotalPipes = ResourceNetwork.Pipes.Num();

	UE_LOG(LogTemp, Log, TEXT("ShipInfrastructureManager: Created resource networks with %d nodes, %d pipes"),
		ResourceNetwork.TotalNodes, ResourceNetwork.TotalPipes);

	// ===== Final Summary =====

	UE_LOG(LogTemp, Log, TEXT("ShipInfrastructureManager: Starting ship 'Endeavour' initialized successfully"));
	UE_LOG(LogTemp, Log, TEXT("  - Compartments: %d"), ShipLayout.Compartments.Num());
	UE_LOG(LogTemp, Log, TEXT("  - Power Grid: %d nodes, %d conduits, %d reactors"),
		ElectricalGrid.Nodes.Num(), ElectricalGrid.Conduits.Num(), ElectricalGrid.Reactors.Num());
	UE_LOG(LogTemp, Log, TEXT("  - Hardpoints: %d total (%d occupied)"),
		HardpointLayout.TotalHardpoints, HardpointLayout.OccupiedHardpoints);
	UE_LOG(LogTemp, Log, TEXT("  - Resource Networks: %d nodes, %d pipes"),
		ResourceNetwork.TotalNodes, ResourceNetwork.TotalPipes);
	UE_LOG(LogTemp, Log, TEXT("  - Crew: %d / %d"), TotalCrewMembers, ResourceNetwork.LifeSupport.MaxCrewCapacity);
	UE_LOG(LogTemp, Log, TEXT("  - Power: %.1f kW generation, %.1f kW consumption"),
		ElectricalGrid.TotalGeneration, ElectricalGrid.TotalConsumption);
}

// ===== Serialization =====

FString UShipInfrastructureManager::SerializeToJSON()
{
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject());

	// ===== Serialize Ship Layout =====
	TSharedPtr<FJsonObject> ShipLayoutObj = MakeShareable(new FJsonObject());
	ShipLayoutObj->SetStringField(TEXT("ShipName"), ShipLayout.ShipName);
	ShipLayoutObj->SetStringField(TEXT("ShipClass"), ShipLayout.ShipClass);
	ShipLayoutObj->SetNumberField(TEXT("TotalCompartments"), ShipLayout.TotalCompartments);
	ShipLayoutObj->SetNumberField(TEXT("TotalDecks"), ShipLayout.TotalDecks);
	ShipLayoutObj->SetNumberField(TEXT("OverallIntegrity"), ShipLayout.OverallIntegrity);

	// Serialize compartments
	TArray<TSharedPtr<FJsonValue>> CompartmentsArray;
	for (const auto& Pair : ShipLayout.Compartments)
	{
		const FShipCompartment& Comp = Pair.Value;
		TSharedPtr<FJsonObject> CompObj = MakeShareable(new FJsonObject());
		CompObj->SetStringField(TEXT("CompartmentID"), Comp.CompartmentID);
		CompObj->SetStringField(TEXT("CompartmentName"), Comp.CompartmentName);
		CompObj->SetNumberField(TEXT("Type"), static_cast<int32>(Comp.Type));
		CompObj->SetNumberField(TEXT("DeckNumber"), Comp.DeckNumber);
		CompObj->SetNumberField(TEXT("Status"), static_cast<int32>(Comp.Status));
		CompObj->SetNumberField(TEXT("Integrity"), Comp.Integrity);
		CompObj->SetNumberField(TEXT("Volume"), Comp.Volume);
		CompObj->SetNumberField(TEXT("MaxCrewCapacity"), Comp.MaxCrewCapacity);
		CompObj->SetNumberField(TEXT("CurrentCrewCount"), Comp.CurrentCrewCount);

		// Atmosphere
		TSharedPtr<FJsonObject> AtmosObj = MakeShareable(new FJsonObject());
		AtmosObj->SetNumberField(TEXT("OxygenLevel"), Comp.Atmosphere.OxygenLevel);
		AtmosObj->SetNumberField(TEXT("Pressure"), Comp.Atmosphere.Pressure);
		AtmosObj->SetNumberField(TEXT("Temperature"), Comp.Atmosphere.Temperature);
		CompObj->SetObjectField(TEXT("Atmosphere"), AtmosObj);

		CompObj->SetBoolField(TEXT("bIsOnFire"), Comp.bIsOnFire);
		CompObj->SetNumberField(TEXT("FireIntensity"), Comp.FireIntensity);

		CompartmentsArray.Add(MakeShareable(new FJsonValueObject(CompObj)));
	}
	ShipLayoutObj->SetArrayField(TEXT("Compartments"), CompartmentsArray);

	// Serialize doors
	TArray<TSharedPtr<FJsonValue>> DoorsArray;
	for (const auto& Pair : ShipLayout.Doors)
	{
		const FShipDoor& Door = Pair.Value;
		TSharedPtr<FJsonObject> DoorObj = MakeShareable(new FJsonObject());
		DoorObj->SetStringField(TEXT("DoorID"), Door.DoorID);
		DoorObj->SetNumberField(TEXT("State"), static_cast<int32>(Door.State));
		DoorObj->SetNumberField(TEXT("Health"), Door.Health);
		DoorObj->SetNumberField(TEXT("PowerRequired"), Door.PowerRequired);

		TArray<TSharedPtr<FJsonValue>> ConnectedComps;
		for (const FString& CompID : Door.ConnectedCompartments)
		{
			ConnectedComps.Add(MakeShareable(new FJsonValueString(CompID)));
		}
		DoorObj->SetArrayField(TEXT("ConnectedCompartments"), ConnectedComps);

		DoorsArray.Add(MakeShareable(new FJsonValueObject(DoorObj)));
	}
	ShipLayoutObj->SetArrayField(TEXT("Doors"), DoorsArray);

	RootObject->SetObjectField(TEXT("ShipLayout"), ShipLayoutObj);

	// ===== Serialize Electrical Grid =====
	TSharedPtr<FJsonObject> GridObj = MakeShareable(new FJsonObject());
	GridObj->SetNumberField(TEXT("TotalGeneration"), ElectricalGrid.TotalGeneration);
	GridObj->SetNumberField(TEXT("TotalConsumption"), ElectricalGrid.TotalConsumption);
	GridObj->SetNumberField(TEXT("GridEfficiency"), ElectricalGrid.GridEfficiency);
	GridObj->SetNumberField(TEXT("OverloadedConduits"), ElectricalGrid.OverloadedConduits);
	GridObj->SetNumberField(TEXT("OfflineNodes"), ElectricalGrid.OfflineNodes);

	// Serialize reactors
	TArray<TSharedPtr<FJsonValue>> ReactorsArray;
	for (const auto& Pair : ElectricalGrid.Reactors)
	{
		const FReactor& Reactor = Pair.Value;
		TSharedPtr<FJsonObject> ReactorObj = MakeShareable(new FJsonObject());
		ReactorObj->SetStringField(TEXT("ReactorID"), Pair.Key);
		ReactorObj->SetStringField(TEXT("PowerNodeID"), Reactor.PowerNodeID);
		ReactorObj->SetNumberField(TEXT("ReactorType"), static_cast<int32>(Reactor.ReactorType));
		ReactorObj->SetNumberField(TEXT("FuelLevel"), Reactor.FuelLevel);
		ReactorObj->SetNumberField(TEXT("FuelConsumptionRate"), Reactor.FuelConsumptionRate);
		ReactorObj->SetNumberField(TEXT("Temperature"), Reactor.Temperature);
		ReactorObj->SetNumberField(TEXT("OptimalTemperature"), Reactor.OptimalTemperature);
		ReactorObj->SetNumberField(TEXT("MaxSafeTemperature"), Reactor.MaxSafeTemperature);
		ReactorObj->SetNumberField(TEXT("PowerOutputPercent"), Reactor.PowerOutputPercent);
		ReactorObj->SetBoolField(TEXT("bRequiresCoolant"), Reactor.bRequiresCoolant);
		ReactorObj->SetNumberField(TEXT("CoolantLevel"), Reactor.CoolantLevel);
		ReactorObj->SetBoolField(TEXT("bEmergencyShutdown"), Reactor.bEmergencyShutdown);

		ReactorsArray.Add(MakeShareable(new FJsonValueObject(ReactorObj)));
	}
	GridObj->SetArrayField(TEXT("Reactors"), ReactorsArray);

	// Serialize power nodes
	TArray<TSharedPtr<FJsonValue>> NodesArray;
	for (const auto& Pair : ElectricalGrid.Nodes)
	{
		const FPowerNode& Node = Pair.Value;
		TSharedPtr<FJsonObject> NodeObj = MakeShareable(new FJsonObject());
		NodeObj->SetStringField(TEXT("NodeID"), Node.NodeID);
		NodeObj->SetStringField(TEXT("NodeName"), Node.NodeName);
		NodeObj->SetNumberField(TEXT("Type"), static_cast<int32>(Node.Type));
		NodeObj->SetNumberField(TEXT("Status"), static_cast<int32>(Node.Status));
		NodeObj->SetStringField(TEXT("CompartmentID"), Node.CompartmentID);
		NodeObj->SetNumberField(TEXT("MaxPowerGeneration"), Node.MaxPowerGeneration);
		NodeObj->SetNumberField(TEXT("PowerGeneration"), Node.PowerGeneration);
		NodeObj->SetNumberField(TEXT("RequiredPower"), Node.RequiredPower);
		NodeObj->SetNumberField(TEXT("PowerConsumption"), Node.PowerConsumption);
		NodeObj->SetNumberField(TEXT("MaxThroughput"), Node.MaxThroughput);
		NodeObj->SetNumberField(TEXT("Health"), Node.Health);
		NodeObj->SetNumberField(TEXT("Priority"), Node.Priority);

		NodesArray.Add(MakeShareable(new FJsonValueObject(NodeObj)));
	}
	GridObj->SetArrayField(TEXT("Nodes"), NodesArray);

	// Serialize power conduits
	TArray<TSharedPtr<FJsonValue>> ConduitsArray;
	for (const auto& Pair : ElectricalGrid.Conduits)
	{
		const FPowerConduit& Conduit = Pair.Value;
		TSharedPtr<FJsonObject> ConduitObj = MakeShareable(new FJsonObject());
		ConduitObj->SetStringField(TEXT("ConduitID"), Conduit.ConduitID);
		ConduitObj->SetStringField(TEXT("SourceNodeID"), Conduit.SourceNodeID);
		ConduitObj->SetStringField(TEXT("DestinationNodeID"), Conduit.DestinationNodeID);
		ConduitObj->SetNumberField(TEXT("MaxCapacity"), Conduit.MaxCapacity);
		ConduitObj->SetNumberField(TEXT("CurrentFlow"), Conduit.CurrentFlow);
		ConduitObj->SetNumberField(TEXT("Efficiency"), Conduit.Efficiency);
		ConduitObj->SetNumberField(TEXT("Health"), Conduit.Health);
		ConduitObj->SetBoolField(TEXT("bIsDamaged"), Conduit.bIsDamaged);

		ConduitsArray.Add(MakeShareable(new FJsonValueObject(ConduitObj)));
	}
	GridObj->SetArrayField(TEXT("Conduits"), ConduitsArray);

	RootObject->SetObjectField(TEXT("ElectricalGrid"), GridObj);

	// ===== Serialize Hardpoint Layout =====
	TSharedPtr<FJsonObject> HardpointsObj = MakeShareable(new FJsonObject());
	HardpointsObj->SetNumberField(TEXT("TotalHardpoints"), HardpointLayout.TotalHardpoints);
	HardpointsObj->SetNumberField(TEXT("OccupiedHardpoints"), HardpointLayout.OccupiedHardpoints);
	HardpointsObj->SetNumberField(TEXT("DamagedHardpoints"), HardpointLayout.DamagedHardpoints);

	TArray<TSharedPtr<FJsonValue>> HardpointsArray;
	for (const auto& Pair : HardpointLayout.Hardpoints)
	{
		const FHardpoint& HP = Pair.Value;
		TSharedPtr<FJsonObject> HPObj = MakeShareable(new FJsonObject());
		HPObj->SetStringField(TEXT("HardpointID"), HP.HardpointID);
		HPObj->SetStringField(TEXT("HardpointName"), HP.HardpointName);
		HPObj->SetNumberField(TEXT("Size"), static_cast<int32>(HP.Size));
		HPObj->SetNumberField(TEXT("Status"), static_cast<int32>(HP.Status));
		HPObj->SetStringField(TEXT("CompartmentID"), HP.CompartmentID);
		HPObj->SetStringField(TEXT("PowerNodeID"), HP.PowerNodeID);
		HPObj->SetStringField(TEXT("CoolantNodeID"), HP.CoolantNodeID);
		HPObj->SetNumberField(TEXT("Integrity"), HP.Integrity);
		HPObj->SetNumberField(TEXT("MaxLoadCapacity"), HP.MaxLoadCapacity);

		// Position
		TSharedPtr<FJsonObject> PosObj = MakeShareable(new FJsonObject());
		PosObj->SetNumberField(TEXT("X"), HP.Position.X);
		PosObj->SetNumberField(TEXT("Y"), HP.Position.Y);
		PosObj->SetNumberField(TEXT("Z"), HP.Position.Z);
		HPObj->SetObjectField(TEXT("Position"), PosObj);

		// Mounted Equipment
		if (HP.HasMountedEquipment())
		{
			const FMountedEquipment& Eq = HP.MountedEquipment;
			TSharedPtr<FJsonObject> EqObj = MakeShareable(new FJsonObject());
			EqObj->SetStringField(TEXT("EquipmentID"), Eq.EquipmentID);
			EqObj->SetStringField(TEXT("EquipmentName"), Eq.EquipmentName);
			EqObj->SetNumberField(TEXT("Type"), static_cast<int32>(Eq.Type));
			EqObj->SetNumberField(TEXT("RequiredSize"), static_cast<int32>(Eq.RequiredSize));
			EqObj->SetNumberField(TEXT("State"), static_cast<int32>(Eq.State));
			EqObj->SetNumberField(TEXT("Health"), Eq.Health);
			EqObj->SetNumberField(TEXT("PowerRequired"), Eq.PowerRequired);
			EqObj->SetNumberField(TEXT("PowerConsumption"), Eq.PowerConsumption);
			EqObj->SetNumberField(TEXT("HeatGeneration"), Eq.HeatGeneration);
			EqObj->SetNumberField(TEXT("CurrentHeat"), Eq.CurrentHeat);
			EqObj->SetBoolField(TEXT("bRequiresCoolant"), Eq.bRequiresCoolant);
			EqObj->SetNumberField(TEXT("CoolantFlow"), Eq.CoolantFlow);
			EqObj->SetNumberField(TEXT("Mass"), Eq.Mass);
			EqObj->SetNumberField(TEXT("CrewRequired"), Eq.CrewRequired);

			// Performance stats
			TSharedPtr<FJsonObject> StatsObj = MakeShareable(new FJsonObject());
			for (const auto& StatPair : Eq.PerformanceStats)
			{
				StatsObj->SetNumberField(StatPair.Key, StatPair.Value);
			}
			EqObj->SetObjectField(TEXT("PerformanceStats"), StatsObj);

			HPObj->SetObjectField(TEXT("MountedEquipment"), EqObj);
		}

		HardpointsArray.Add(MakeShareable(new FJsonValueObject(HPObj)));
	}
	HardpointsObj->SetArrayField(TEXT("Hardpoints"), HardpointsArray);

	RootObject->SetObjectField(TEXT("HardpointLayout"), HardpointsObj);

	// ===== Serialize Resource Network =====
	TSharedPtr<FJsonObject> ResourceObj = MakeShareable(new FJsonObject());

	// Life Support
	TSharedPtr<FJsonObject> LifeSupportObj = MakeShareable(new FJsonObject());
	LifeSupportObj->SetBoolField(TEXT("bIsOperational"), ResourceNetwork.LifeSupport.bIsOperational);
	LifeSupportObj->SetNumberField(TEXT("OxygenProductionRate"), ResourceNetwork.LifeSupport.OxygenProductionRate);
	LifeSupportObj->SetNumberField(TEXT("CO2RemovalRate"), ResourceNetwork.LifeSupport.CO2RemovalRate);
	LifeSupportObj->SetNumberField(TEXT("WaterRecyclingRate"), ResourceNetwork.LifeSupport.WaterRecyclingRate);
	LifeSupportObj->SetNumberField(TEXT("CurrentCrew"), ResourceNetwork.LifeSupport.CurrentCrew);
	LifeSupportObj->SetNumberField(TEXT("MaxCrewCapacity"), ResourceNetwork.LifeSupport.MaxCrewCapacity);
	LifeSupportObj->SetNumberField(TEXT("OxygenGenerators"), ResourceNetwork.LifeSupport.OxygenGenerators);
	LifeSupportObj->SetNumberField(TEXT("CO2Scrubbers"), ResourceNetwork.LifeSupport.CO2Scrubbers);
	LifeSupportObj->SetNumberField(TEXT("WaterRecyclers"), ResourceNetwork.LifeSupport.WaterRecyclers);
	ResourceObj->SetObjectField(TEXT("LifeSupport"), LifeSupportObj);

	// Coolant System
	TSharedPtr<FJsonObject> CoolantObj = MakeShareable(new FJsonObject());
	CoolantObj->SetBoolField(TEXT("bIsOperational"), ResourceNetwork.Coolant.bIsOperational);
	CoolantObj->SetNumberField(TEXT("CoolantLevel"), ResourceNetwork.Coolant.CoolantLevel);
	CoolantObj->SetNumberField(TEXT("MaxCoolantCapacity"), ResourceNetwork.Coolant.MaxCoolantCapacity);
	CoolantObj->SetNumberField(TEXT("HeatDissipationRate"), ResourceNetwork.Coolant.HeatDissipationRate);
	CoolantObj->SetNumberField(TEXT("CurrentHeatLoad"), ResourceNetwork.Coolant.CurrentHeatLoad);
	CoolantObj->SetNumberField(TEXT("HeatExchangers"), ResourceNetwork.Coolant.HeatExchangers);
	CoolantObj->SetNumberField(TEXT("Radiators"), ResourceNetwork.Coolant.Radiators);
	CoolantObj->SetNumberField(TEXT("RadiatorEfficiency"), ResourceNetwork.Coolant.RadiatorEfficiency);
	ResourceObj->SetObjectField(TEXT("Coolant"), CoolantObj);

	// Resource nodes
	TArray<TSharedPtr<FJsonValue>> ResNodesArray;
	for (const auto& Pair : ResourceNetwork.Nodes)
	{
		const FResourceNode& ResNode = Pair.Value;
		TSharedPtr<FJsonObject> ResNodeObj = MakeShareable(new FJsonObject());
		ResNodeObj->SetStringField(TEXT("NodeID"), ResNode.NodeID);
		ResNodeObj->SetStringField(TEXT("NodeName"), ResNode.NodeName);
		ResNodeObj->SetNumberField(TEXT("Type"), static_cast<int32>(ResNode.Type));
		ResNodeObj->SetStringField(TEXT("CompartmentID"), ResNode.CompartmentID);
		ResNodeObj->SetNumberField(TEXT("ProductionRate"), ResNode.ProductionRate);
		ResNodeObj->SetNumberField(TEXT("ConsumptionRate"), ResNode.ConsumptionRate);
		ResNodeObj->SetNumberField(TEXT("StorageCapacity"), ResNode.StorageCapacity);
		ResNodeObj->SetNumberField(TEXT("CurrentStorage"), ResNode.CurrentStorage);
		ResNodeObj->SetNumberField(TEXT("Health"), ResNode.Health);
		ResNodeObj->SetBoolField(TEXT("bIsOperational"), ResNode.bIsOperational);

		ResNodesArray.Add(MakeShareable(new FJsonValueObject(ResNodeObj)));
	}
	ResourceObj->SetArrayField(TEXT("Nodes"), ResNodesArray);

	// Resource pipes
	TArray<TSharedPtr<FJsonValue>> ResPipesArray;
	for (const auto& Pair : ResourceNetwork.Pipes)
	{
		const FResourcePipe& Pipe = Pair.Value;
		TSharedPtr<FJsonObject> PipeObj = MakeShareable(new FJsonObject());
		PipeObj->SetStringField(TEXT("PipeID"), Pipe.PipeID);
		PipeObj->SetNumberField(TEXT("ResourceType"), static_cast<int32>(Pipe.ResourceType));
		PipeObj->SetStringField(TEXT("SourceNodeID"), Pipe.SourceNodeID);
		PipeObj->SetStringField(TEXT("DestinationNodeID"), Pipe.DestinationNodeID);
		PipeObj->SetNumberField(TEXT("MaxFlowRate"), Pipe.MaxFlowRate);
		PipeObj->SetNumberField(TEXT("CurrentFlowRate"), Pipe.CurrentFlowRate);
		PipeObj->SetNumberField(TEXT("Pressure"), Pipe.Pressure);
		PipeObj->SetNumberField(TEXT("Health"), Pipe.Health);
		PipeObj->SetBoolField(TEXT("bIsLeaking"), Pipe.bIsLeaking);
		PipeObj->SetNumberField(TEXT("LeakRate"), Pipe.LeakRate);

		ResPipesArray.Add(MakeShareable(new FJsonValueObject(PipeObj)));
	}
	ResourceObj->SetArrayField(TEXT("Pipes"), ResPipesArray);

	ResourceObj->SetNumberField(TEXT("LeakingPipes"), ResourceNetwork.LeakingPipes);
	ResourceObj->SetNumberField(TEXT("TotalNodes"), ResourceNetwork.TotalNodes);
	ResourceObj->SetNumberField(TEXT("TotalPipes"), ResourceNetwork.TotalPipes);

	RootObject->SetObjectField(TEXT("ResourceNetwork"), ResourceObj);

	// ===== Serialize Crew =====
	RootObject->SetNumberField(TEXT("TotalCrewMembers"), TotalCrewMembers);

	// Convert to JSON string
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	UE_LOG(LogTemp, Log, TEXT("ShipInfrastructureManager: Serialized ship data (%d characters)"), OutputString.Len());

	return OutputString;
}

bool UShipInfrastructureManager::DeserializeFromJSON(const FString& JSON)
{
	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSON);

	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ShipInfrastructureManager: Failed to parse JSON"));
		return false;
	}

	// Clear existing data
	ShipLayout.Compartments.Empty();
	ShipLayout.Doors.Empty();
	ElectricalGrid.Reactors.Empty();
	ElectricalGrid.Nodes.Empty();
	ElectricalGrid.Conduits.Empty();
	HardpointLayout.Hardpoints.Empty();
	ResourceNetwork.Nodes.Empty();
	ResourceNetwork.Pipes.Empty();

	// ===== Deserialize Ship Layout =====
	const TSharedPtr<FJsonObject>* ShipLayoutObj;
	if (RootObject->TryGetObjectField(TEXT("ShipLayout"), ShipLayoutObj))
	{
		ShipLayout.ShipName = (*ShipLayoutObj)->GetStringField(TEXT("ShipName"));
		ShipLayout.ShipClass = (*ShipLayoutObj)->GetStringField(TEXT("ShipClass"));
		ShipLayout.TotalCompartments = (*ShipLayoutObj)->GetIntegerField(TEXT("TotalCompartments"));
		ShipLayout.TotalDecks = (*ShipLayoutObj)->GetIntegerField(TEXT("TotalDecks"));
		ShipLayout.OverallIntegrity = (*ShipLayoutObj)->GetNumberField(TEXT("OverallIntegrity"));

		// Deserialize compartments
		const TArray<TSharedPtr<FJsonValue>>* CompartmentsArray;
		if ((*ShipLayoutObj)->TryGetArrayField(TEXT("Compartments"), CompartmentsArray))
		{
			for (const TSharedPtr<FJsonValue>& CompValue : *CompartmentsArray)
			{
				const TSharedPtr<FJsonObject>& CompObj = CompValue->AsObject();
				FShipCompartment Comp;

				Comp.CompartmentID = CompObj->GetStringField(TEXT("CompartmentID"));
				Comp.CompartmentName = CompObj->GetStringField(TEXT("CompartmentName"));
				Comp.Type = static_cast<ECompartmentType>(CompObj->GetIntegerField(TEXT("Type")));
				Comp.DeckNumber = CompObj->GetIntegerField(TEXT("DeckNumber"));
				Comp.Status = static_cast<ECompartmentStatus>(CompObj->GetIntegerField(TEXT("Status")));
				Comp.Integrity = CompObj->GetNumberField(TEXT("Integrity"));
				Comp.Volume = CompObj->GetNumberField(TEXT("Volume"));
				Comp.MaxCrewCapacity = CompObj->GetIntegerField(TEXT("MaxCrewCapacity"));
				Comp.CurrentCrewCount = CompObj->GetIntegerField(TEXT("CurrentCrewCount"));

				// Atmosphere
				const TSharedPtr<FJsonObject>* AtmosObj;
				if (CompObj->TryGetObjectField(TEXT("Atmosphere"), AtmosObj))
				{
					Comp.Atmosphere.OxygenLevel = (*AtmosObj)->GetNumberField(TEXT("OxygenLevel"));
					Comp.Atmosphere.Pressure = (*AtmosObj)->GetNumberField(TEXT("Pressure"));
					Comp.Atmosphere.Temperature = (*AtmosObj)->GetNumberField(TEXT("Temperature"));
				}

				Comp.bIsOnFire = CompObj->GetBoolField(TEXT("bIsOnFire"));
				Comp.FireIntensity = CompObj->GetNumberField(TEXT("FireIntensity"));

				ShipLayout.Compartments.Add(Comp.CompartmentID, Comp);
			}
		}

		// Deserialize doors
		const TArray<TSharedPtr<FJsonValue>>* DoorsArray;
		if ((*ShipLayoutObj)->TryGetArrayField(TEXT("Doors"), DoorsArray))
		{
			for (const TSharedPtr<FJsonValue>& DoorValue : *DoorsArray)
			{
				const TSharedPtr<FJsonObject>& DoorObj = DoorValue->AsObject();
				FShipDoor Door;

				Door.DoorID = DoorObj->GetStringField(TEXT("DoorID"));
				Door.State = static_cast<EDoorState>(DoorObj->GetIntegerField(TEXT("State")));
				Door.Health = DoorObj->GetNumberField(TEXT("Health"));
				Door.PowerRequired = DoorObj->GetNumberField(TEXT("PowerRequired"));

				const TArray<TSharedPtr<FJsonValue>>* ConnectedComps;
				if (DoorObj->TryGetArrayField(TEXT("ConnectedCompartments"), ConnectedComps))
				{
					for (const TSharedPtr<FJsonValue>& CompValue : *ConnectedComps)
					{
						Door.ConnectedCompartments.Add(CompValue->AsString());
					}
				}

				ShipLayout.Doors.Add(Door.DoorID, Door);
			}
		}
	}

	// ===== Deserialize Electrical Grid =====
	const TSharedPtr<FJsonObject>* GridObj;
	if (RootObject->TryGetObjectField(TEXT("ElectricalGrid"), GridObj))
	{
		ElectricalGrid.TotalGeneration = (*GridObj)->GetNumberField(TEXT("TotalGeneration"));
		ElectricalGrid.TotalConsumption = (*GridObj)->GetNumberField(TEXT("TotalConsumption"));
		ElectricalGrid.GridEfficiency = (*GridObj)->GetNumberField(TEXT("GridEfficiency"));
		ElectricalGrid.OverloadedConduits = (*GridObj)->GetIntegerField(TEXT("OverloadedConduits"));
		ElectricalGrid.OfflineNodes = (*GridObj)->GetIntegerField(TEXT("OfflineNodes"));

		// Deserialize reactors
		const TArray<TSharedPtr<FJsonValue>>* ReactorsArray;
		if ((*GridObj)->TryGetArrayField(TEXT("Reactors"), ReactorsArray))
		{
			for (const TSharedPtr<FJsonValue>& ReactorValue : *ReactorsArray)
			{
				const TSharedPtr<FJsonObject>& ReactorObj = ReactorValue->AsObject();
				FReactor Reactor;

				FString ReactorID = ReactorObj->GetStringField(TEXT("ReactorID"));
				Reactor.PowerNodeID = ReactorObj->GetStringField(TEXT("PowerNodeID"));
				Reactor.ReactorType = static_cast<EReactorType>(ReactorObj->GetIntegerField(TEXT("ReactorType")));
				Reactor.FuelLevel = ReactorObj->GetNumberField(TEXT("FuelLevel"));
				Reactor.FuelConsumptionRate = ReactorObj->GetNumberField(TEXT("FuelConsumptionRate"));
				Reactor.Temperature = ReactorObj->GetNumberField(TEXT("Temperature"));
				Reactor.OptimalTemperature = ReactorObj->GetNumberField(TEXT("OptimalTemperature"));
				Reactor.MaxSafeTemperature = ReactorObj->GetNumberField(TEXT("MaxSafeTemperature"));
				Reactor.PowerOutputPercent = ReactorObj->GetNumberField(TEXT("PowerOutputPercent"));
				Reactor.bRequiresCoolant = ReactorObj->GetBoolField(TEXT("bRequiresCoolant"));
				Reactor.CoolantLevel = ReactorObj->GetNumberField(TEXT("CoolantLevel"));
				Reactor.bEmergencyShutdown = ReactorObj->GetBoolField(TEXT("bEmergencyShutdown"));

				ElectricalGrid.Reactors.Add(ReactorID, Reactor);
			}
		}

		// Deserialize power nodes
		const TArray<TSharedPtr<FJsonValue>>* NodesArray;
		if ((*GridObj)->TryGetArrayField(TEXT("Nodes"), NodesArray))
		{
			for (const TSharedPtr<FJsonValue>& NodeValue : *NodesArray)
			{
				const TSharedPtr<FJsonObject>& NodeObj = NodeValue->AsObject();
				FPowerNode Node;

				Node.NodeID = NodeObj->GetStringField(TEXT("NodeID"));
				Node.NodeName = NodeObj->GetStringField(TEXT("NodeName"));
				Node.Type = static_cast<EPowerNodeType>(NodeObj->GetIntegerField(TEXT("Type")));
				Node.Status = static_cast<EPowerNodeStatus>(NodeObj->GetIntegerField(TEXT("Status")));
				Node.CompartmentID = NodeObj->GetStringField(TEXT("CompartmentID"));
				Node.MaxPowerGeneration = NodeObj->GetNumberField(TEXT("MaxPowerGeneration"));
				Node.PowerGeneration = NodeObj->GetNumberField(TEXT("PowerGeneration"));
				Node.RequiredPower = NodeObj->GetNumberField(TEXT("RequiredPower"));
				Node.PowerConsumption = NodeObj->GetNumberField(TEXT("PowerConsumption"));
				Node.MaxThroughput = NodeObj->GetNumberField(TEXT("MaxThroughput"));
				Node.Health = NodeObj->GetNumberField(TEXT("Health"));
				Node.Priority = NodeObj->GetIntegerField(TEXT("Priority"));

				ElectricalGrid.Nodes.Add(Node.NodeID, Node);
			}
		}

		// Deserialize power conduits
		const TArray<TSharedPtr<FJsonValue>>* ConduitsArray;
		if ((*GridObj)->TryGetArrayField(TEXT("Conduits"), ConduitsArray))
		{
			for (const TSharedPtr<FJsonValue>& ConduitValue : *ConduitsArray)
			{
				const TSharedPtr<FJsonObject>& ConduitObj = ConduitValue->AsObject();
				FPowerConduit Conduit;

				Conduit.ConduitID = ConduitObj->GetStringField(TEXT("ConduitID"));
				Conduit.SourceNodeID = ConduitObj->GetStringField(TEXT("SourceNodeID"));
				Conduit.DestinationNodeID = ConduitObj->GetStringField(TEXT("DestinationNodeID"));
				Conduit.MaxCapacity = ConduitObj->GetNumberField(TEXT("MaxCapacity"));
				Conduit.CurrentFlow = ConduitObj->GetNumberField(TEXT("CurrentFlow"));
				Conduit.Efficiency = ConduitObj->GetNumberField(TEXT("Efficiency"));
				Conduit.Health = ConduitObj->GetNumberField(TEXT("Health"));
				Conduit.bIsDamaged = ConduitObj->GetBoolField(TEXT("bIsDamaged"));

				ElectricalGrid.Conduits.Add(Conduit.ConduitID, Conduit);
			}
		}
	}

	// ===== Deserialize Hardpoint Layout =====
	const TSharedPtr<FJsonObject>* HardpointsObj;
	if (RootObject->TryGetObjectField(TEXT("HardpointLayout"), HardpointsObj))
	{
		HardpointLayout.TotalHardpoints = (*HardpointsObj)->GetIntegerField(TEXT("TotalHardpoints"));
		HardpointLayout.OccupiedHardpoints = (*HardpointsObj)->GetIntegerField(TEXT("OccupiedHardpoints"));
		HardpointLayout.DamagedHardpoints = (*HardpointsObj)->GetIntegerField(TEXT("DamagedHardpoints"));

		const TArray<TSharedPtr<FJsonValue>>* HardpointsArray;
		if ((*HardpointsObj)->TryGetArrayField(TEXT("Hardpoints"), HardpointsArray))
		{
			for (const TSharedPtr<FJsonValue>& HPValue : *HardpointsArray)
			{
				const TSharedPtr<FJsonObject>& HPObj = HPValue->AsObject();
				FHardpoint HP;

				HP.HardpointID = HPObj->GetStringField(TEXT("HardpointID"));
				HP.HardpointName = HPObj->GetStringField(TEXT("HardpointName"));
				HP.Size = static_cast<EHardpointSize>(HPObj->GetIntegerField(TEXT("Size")));
				HP.Status = static_cast<EHardpointStatus>(HPObj->GetIntegerField(TEXT("Status")));
				HP.CompartmentID = HPObj->GetStringField(TEXT("CompartmentID"));
				HP.PowerNodeID = HPObj->GetStringField(TEXT("PowerNodeID"));
				HP.CoolantNodeID = HPObj->GetStringField(TEXT("CoolantNodeID"));
				HP.Integrity = HPObj->GetNumberField(TEXT("Integrity"));
				HP.MaxLoadCapacity = HPObj->GetNumberField(TEXT("MaxLoadCapacity"));

				// Position
				const TSharedPtr<FJsonObject>* PosObj;
				if (HPObj->TryGetObjectField(TEXT("Position"), PosObj))
				{
					HP.Position.X = (*PosObj)->GetNumberField(TEXT("X"));
					HP.Position.Y = (*PosObj)->GetNumberField(TEXT("Y"));
					HP.Position.Z = (*PosObj)->GetNumberField(TEXT("Z"));
				}

				// Mounted Equipment
				const TSharedPtr<FJsonObject>* EqObj;
				if (HPObj->TryGetObjectField(TEXT("MountedEquipment"), EqObj))
				{
					FMountedEquipment Eq;
					Eq.EquipmentID = (*EqObj)->GetStringField(TEXT("EquipmentID"));
					Eq.EquipmentName = (*EqObj)->GetStringField(TEXT("EquipmentName"));
					Eq.Type = static_cast<EEquipmentType>((*EqObj)->GetIntegerField(TEXT("Type")));
					Eq.RequiredSize = static_cast<EHardpointSize>((*EqObj)->GetIntegerField(TEXT("RequiredSize")));
					Eq.State = static_cast<EEquipmentState>((*EqObj)->GetIntegerField(TEXT("State")));
					Eq.Health = (*EqObj)->GetNumberField(TEXT("Health"));
					Eq.PowerRequired = (*EqObj)->GetNumberField(TEXT("PowerRequired"));
					Eq.PowerConsumption = (*EqObj)->GetNumberField(TEXT("PowerConsumption"));
					Eq.HeatGeneration = (*EqObj)->GetNumberField(TEXT("HeatGeneration"));
					Eq.CurrentHeat = (*EqObj)->GetNumberField(TEXT("CurrentHeat"));
					Eq.bRequiresCoolant = (*EqObj)->GetBoolField(TEXT("bRequiresCoolant"));
					Eq.CoolantFlow = (*EqObj)->GetNumberField(TEXT("CoolantFlow"));
					Eq.Mass = (*EqObj)->GetNumberField(TEXT("Mass"));
					Eq.CrewRequired = (*EqObj)->GetIntegerField(TEXT("CrewRequired"));

					// Performance stats
					const TSharedPtr<FJsonObject>* StatsObj;
					if ((*EqObj)->TryGetObjectField(TEXT("PerformanceStats"), StatsObj))
					{
						for (const auto& StatPair : (*StatsObj)->Values)
						{
							Eq.PerformanceStats.Add(StatPair.Key, StatPair.Value->AsNumber());
						}
					}

					HP.MountedEquipment = Eq;
				}

				HardpointLayout.Hardpoints.Add(HP.HardpointID, HP);
			}
		}
	}

	// ===== Deserialize Resource Network =====
	const TSharedPtr<FJsonObject>* ResourceObj;
	if (RootObject->TryGetObjectField(TEXT("ResourceNetwork"), ResourceObj))
	{
		// Life Support
		const TSharedPtr<FJsonObject>* LifeSupportObj;
		if ((*ResourceObj)->TryGetObjectField(TEXT("LifeSupport"), LifeSupportObj))
		{
			ResourceNetwork.LifeSupport.bIsOperational = (*LifeSupportObj)->GetBoolField(TEXT("bIsOperational"));
			ResourceNetwork.LifeSupport.OxygenProductionRate = (*LifeSupportObj)->GetNumberField(TEXT("OxygenProductionRate"));
			ResourceNetwork.LifeSupport.CO2RemovalRate = (*LifeSupportObj)->GetNumberField(TEXT("CO2RemovalRate"));
			ResourceNetwork.LifeSupport.WaterRecyclingRate = (*LifeSupportObj)->GetNumberField(TEXT("WaterRecyclingRate"));
			ResourceNetwork.LifeSupport.CurrentCrew = (*LifeSupportObj)->GetIntegerField(TEXT("CurrentCrew"));
			ResourceNetwork.LifeSupport.MaxCrewCapacity = (*LifeSupportObj)->GetIntegerField(TEXT("MaxCrewCapacity"));
			ResourceNetwork.LifeSupport.OxygenGenerators = (*LifeSupportObj)->GetIntegerField(TEXT("OxygenGenerators"));
			ResourceNetwork.LifeSupport.CO2Scrubbers = (*LifeSupportObj)->GetIntegerField(TEXT("CO2Scrubbers"));
			ResourceNetwork.LifeSupport.WaterRecyclers = (*LifeSupportObj)->GetIntegerField(TEXT("WaterRecyclers"));
		}

		// Coolant System
		const TSharedPtr<FJsonObject>* CoolantObj;
		if ((*ResourceObj)->TryGetObjectField(TEXT("Coolant"), CoolantObj))
		{
			ResourceNetwork.Coolant.bIsOperational = (*CoolantObj)->GetBoolField(TEXT("bIsOperational"));
			ResourceNetwork.Coolant.CoolantLevel = (*CoolantObj)->GetNumberField(TEXT("CoolantLevel"));
			ResourceNetwork.Coolant.MaxCoolantCapacity = (*CoolantObj)->GetNumberField(TEXT("MaxCoolantCapacity"));
			ResourceNetwork.Coolant.HeatDissipationRate = (*CoolantObj)->GetNumberField(TEXT("HeatDissipationRate"));
			ResourceNetwork.Coolant.CurrentHeatLoad = (*CoolantObj)->GetNumberField(TEXT("CurrentHeatLoad"));
			ResourceNetwork.Coolant.HeatExchangers = (*CoolantObj)->GetIntegerField(TEXT("HeatExchangers"));
			ResourceNetwork.Coolant.Radiators = (*CoolantObj)->GetIntegerField(TEXT("Radiators"));
			ResourceNetwork.Coolant.RadiatorEfficiency = (*CoolantObj)->GetNumberField(TEXT("RadiatorEfficiency"));
		}

		// Resource nodes
		const TArray<TSharedPtr<FJsonValue>>* ResNodesArray;
		if ((*ResourceObj)->TryGetArrayField(TEXT("Nodes"), ResNodesArray))
		{
			for (const TSharedPtr<FJsonValue>& ResNodeValue : *ResNodesArray)
			{
				const TSharedPtr<FJsonObject>& ResNodeObj = ResNodeValue->AsObject();
				FResourceNode ResNode;

				ResNode.NodeID = ResNodeObj->GetStringField(TEXT("NodeID"));
				ResNode.NodeName = ResNodeObj->GetStringField(TEXT("NodeName"));
				ResNode.Type = static_cast<EResourceType>(ResNodeObj->GetIntegerField(TEXT("Type")));
				ResNode.CompartmentID = ResNodeObj->GetStringField(TEXT("CompartmentID"));
				ResNode.ProductionRate = ResNodeObj->GetNumberField(TEXT("ProductionRate"));
				ResNode.ConsumptionRate = ResNodeObj->GetNumberField(TEXT("ConsumptionRate"));
				ResNode.StorageCapacity = ResNodeObj->GetNumberField(TEXT("StorageCapacity"));
				ResNode.CurrentStorage = ResNodeObj->GetNumberField(TEXT("CurrentStorage"));
				ResNode.Health = ResNodeObj->GetNumberField(TEXT("Health"));
				ResNode.bIsOperational = ResNodeObj->GetBoolField(TEXT("bIsOperational"));

				ResourceNetwork.Nodes.Add(ResNode.NodeID, ResNode);
			}
		}

		// Resource pipes
		const TArray<TSharedPtr<FJsonValue>>* ResPipesArray;
		if ((*ResourceObj)->TryGetArrayField(TEXT("Pipes"), ResPipesArray))
		{
			for (const TSharedPtr<FJsonValue>& PipeValue : *ResPipesArray)
			{
				const TSharedPtr<FJsonObject>& PipeObj = PipeValue->AsObject();
				FResourcePipe Pipe;

				Pipe.PipeID = PipeObj->GetStringField(TEXT("PipeID"));
				Pipe.ResourceType = static_cast<EResourceType>(PipeObj->GetIntegerField(TEXT("ResourceType")));
				Pipe.SourceNodeID = PipeObj->GetStringField(TEXT("SourceNodeID"));
				Pipe.DestinationNodeID = PipeObj->GetStringField(TEXT("DestinationNodeID"));
				Pipe.MaxFlowRate = PipeObj->GetNumberField(TEXT("MaxFlowRate"));
				Pipe.CurrentFlowRate = PipeObj->GetNumberField(TEXT("CurrentFlowRate"));
				Pipe.Pressure = PipeObj->GetNumberField(TEXT("Pressure"));
				Pipe.Health = PipeObj->GetNumberField(TEXT("Health"));
				Pipe.bIsLeaking = PipeObj->GetBoolField(TEXT("bIsLeaking"));
				Pipe.LeakRate = PipeObj->GetNumberField(TEXT("LeakRate"));

				ResourceNetwork.Pipes.Add(Pipe.PipeID, Pipe);
			}
		}

		ResourceNetwork.LeakingPipes = (*ResourceObj)->GetIntegerField(TEXT("LeakingPipes"));
		ResourceNetwork.TotalNodes = (*ResourceObj)->GetIntegerField(TEXT("TotalNodes"));
		ResourceNetwork.TotalPipes = (*ResourceObj)->GetIntegerField(TEXT("TotalPipes"));
	}

	// ===== Deserialize Crew =====
	TotalCrewMembers = RootObject->GetIntegerField(TEXT("TotalCrewMembers"));

	UE_LOG(LogTemp, Log, TEXT("ShipInfrastructureManager: Deserialized ship data successfully"));
	UE_LOG(LogTemp, Log, TEXT("  - Loaded %d compartments, %d power nodes, %d hardpoints, %d resource nodes"),
		ShipLayout.Compartments.Num(), ElectricalGrid.Nodes.Num(),
		HardpointLayout.Hardpoints.Num(), ResourceNetwork.Nodes.Num());

	return true;
}
