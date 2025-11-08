// Source/astrochicken/Journey/JourneyManager.cpp

#include "JourneyManager.h"
#include "../Infrastructure/ShipInfrastructureManager.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void UJourneyManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("JourneyManager: Initializing"));

	// Get ship infrastructure manager
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		ShipInfrastructureManager = GameInstance->GetSubsystem<UShipInfrastructureManager>();
	}

	// Initialize star map
	InitializeStarMap();

	// Set starting location
	CurrentTravel.Status = ETravelStatus::Docked;
	CurrentTravel.CurrentSystemID = TEXT("Sol");

	// Initialize navigation capabilities (will be updated based on ship/tech)
	NavigationCaps.MaxJumpDistance = 20.0f;
	NavigationCaps.CruiseSpeed = 10.0f;
	NavigationCaps.BaseFuelConsumption = 1.0f;
	NavigationCaps.FuelCapacity = 1000.0f;
	NavigationCaps.CurrentFuel = 1000.0f;

	UE_LOG(LogTemp, Log, TEXT("JourneyManager: Started at system %s"), *CurrentTravel.CurrentSystemID);
}

void UJourneyManager::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("JourneyManager: Deinitialized"));
}

void UJourneyManager::InitializeStarMap()
{
	// Clear existing systems
	StarSystems.Empty();

	// Create Sol system (starting location)
	CreateStarSystem(TEXT("Sol"), TEXT("Sol System"), EStarType::MainSequence,
		FVector(0, 0, 0), {ESystemAttribute::HasStarport, ESystemAttribute::HasColony});

	// Create nearby systems
	CreateStarSystem(TEXT("AlphaCentauri"), TEXT("Alpha Centauri"), EStarType::BinarySystem,
		FVector(4.37f, 0, 0), {ESystemAttribute::HasStarport});

	CreateStarSystem(TEXT("Sirius"), TEXT("Sirius"), EStarType::MainSequence,
		FVector(0, 8.6f, 0), {ESystemAttribute::TradeHub});

	CreateStarSystem(TEXT("Procyon"), TEXT("Procyon"), EStarType::MainSequence,
		FVector(11.4f, 0, 0), {ESystemAttribute::ResourceRich});

	CreateStarSystem(TEXT("Tau Ceti"), TEXT("Tau Ceti"), EStarType::MainSequence,
		FVector(11.9f, 5.0f, 0), {ESystemAttribute::HasColony, ESystemAttribute::ResourceRich});

	// Mid-range systems
	CreateStarSystem(TEXT("Epsilon Eridani"), TEXT("Epsilon Eridani"), EStarType::MainSequence,
		FVector(10.5f, -8.0f, 0), {ESystemAttribute::SalvageOpportunity});

	CreateStarSystem(TEXT("Vega"), TEXT("Vega"), EStarType::MainSequence,
		FVector(25.0f, 0, 0), {ESystemAttribute::HasStarport, ESystemAttribute::TradeHub});

	CreateStarSystem(TEXT("Altair"), TEXT("Altair"), EStarType::MainSequence,
		FVector(16.7f, 0, 0), {ESystemAttribute::ResourceRich});

	CreateStarSystem(TEXT("Fomalhaut"), TEXT("Fomalhaut"), EStarType::MainSequence,
		FVector(25.1f, 15.0f, 0), {ESystemAttribute::Hazardous});

	// Distant systems
	CreateStarSystem(TEXT("Betelgeuse"), TEXT("Betelgeuse"), EStarType::RedGiant,
		FVector(548.0f, 200.0f, 100.0f), {ESystemAttribute::Hazardous, ESystemAttribute::ResourceRich});

	CreateStarSystem(TEXT("Rigel"), TEXT("Rigel"), EStarType::MainSequence,
		FVector(860.0f, -100.0f, 50.0f), {ESystemAttribute::HasStarport, ESystemAttribute::TradeHub});

	// Dangerous systems
	CreateStarSystem(TEXT("PirateNest"), TEXT("Serpent's Den"), EStarType::Nebula,
		FVector(30.0f, -20.0f, 0), {ESystemAttribute::PirateActivity, ESystemAttribute::SalvageOpportunity});

	CreateStarSystem(TEXT("Neutron7"), TEXT("Pulsar Station"), EStarType::Neutron,
		FVector(45.0f, 10.0f, 0), {ESystemAttribute::Hazardous, ESystemAttribute::ResourceRich});

	// Deep space waypoints
	CreateStarSystem(TEXT("DeepSpace1"), TEXT("Waypoint Alpha"), EStarType::DeepSpace,
		FVector(15.0f, 15.0f, 0), {});

	CreateStarSystem(TEXT("DeepSpace2"), TEXT("Waypoint Beta"), EStarType::DeepSpace,
		FVector(35.0f, 5.0f, 0), {});

	// Mark Sol as discovered and visited
	if (FStarSystem* SolSystem = StarSystems.Find(TEXT("Sol")))
	{
		SolSystem->bIsDiscovered = true;
		SolSystem->bIsVisited = true;
	}

	UE_LOG(LogTemp, Log, TEXT("JourneyManager: Initialized star map with %d systems"), StarSystems.Num());
}

void UJourneyManager::CreateStarSystem(const FString& ID, const FString& Name, EStarType Type,
                                       const FVector& Position, const TArray<ESystemAttribute>& Attributes)
{
	FStarSystem System;
	System.SystemID = ID;
	System.SystemName = Name;
	System.StarType = Type;
	System.Position = Position;
	System.Attributes = Attributes;

	// Set resource availability based on attributes
	if (System.HasAttribute(ESystemAttribute::HasStarport))
	{
		System.FuelAvailable = 1000.0f;
		System.SuppliesAvailable = 500.0f;
	}
	else if (System.HasAttribute(ESystemAttribute::ResourceRich))
	{
		System.FuelAvailable = 500.0f;
		System.SuppliesAvailable = 300.0f;
	}

	// Set planet count based on star type
	switch (Type)
	{
	case EStarType::MainSequence:
		System.PlanetCount = FMath::RandRange(3, 8);
		break;
	case EStarType::RedGiant:
		System.PlanetCount = FMath::RandRange(0, 2);
		break;
	case EStarType::BinarySystem:
		System.PlanetCount = FMath::RandRange(2, 5);
		break;
	default:
		System.PlanetCount = FMath::RandRange(0, 3);
		break;
	}

	// Generate description
	System.Description = FString::Printf(TEXT("%s - %d planets"), *Name, System.PlanetCount);

	StarSystems.Add(ID, System);
}

void UJourneyManager::UpdateTravel(float DeltaTime)
{
	if (!IsInTransit())
	{
		return;
	}

	// Apply time acceleration
	float AcceleratedDelta = DeltaTime * TimeAccelerationFactor;
	GameTime += AcceleratedDelta;

	// Process travel
	ProcessTravel(AcceleratedDelta);

	// Check for fuel warnings
	CheckFuelWarnings();

	// Update ship systems if needed
	if (ShipInfrastructureManager)
	{
		UpdateShipSystems();
	}
}

void UJourneyManager::ProcessTravel(float DeltaTime)
{
	// Calculate distance traveled this frame
	float DaysPassed = DeltaTime / 86400.0f; // Convert seconds to days
	float DistanceTraveled = CurrentTravel.TravelSpeed * DaysPassed;

	// Update progress
	FStarSystem StartSystem;
	FStarSystem DestSystem;
	if (GetStarSystem(CurrentTravel.CurrentSystemID, StartSystem) &&
	    GetStarSystem(CurrentTravel.DestinationSystemID, DestSystem))
	{
		float TotalDistance = StartSystem.DistanceTo(DestSystem);
		CurrentTravel.DistanceRemaining -= DistanceTraveled;

		if (CurrentTravel.DistanceRemaining <= 0.0f)
		{
			CurrentTravel.DistanceRemaining = 0.0f;
			CurrentTravel.TravelProgress = 1.0f;
			CompleteJump();
		}
		else
		{
			CurrentTravel.TravelProgress = 1.0f - (CurrentTravel.DistanceRemaining / TotalDistance);
		}

		// Consume fuel
		ConsumeFuel(DistanceTraveled);

		// Broadcast progress
		OnTravelProgress.Broadcast(CurrentTravel.TravelProgress, CurrentTravel.DistanceRemaining);

		// Update stats
		Stats.TotalDistanceTraveled += DistanceTraveled;
		Stats.TotalTravelTime += DaysPassed;
	}
}

void UJourneyManager::ConsumeFuel(float Distance)
{
	float FuelCost = Distance * NavigationCaps.BaseFuelConsumption * NavigationCaps.GetFuelEfficiency();
	NavigationCaps.CurrentFuel = FMath::Max(0.0f, NavigationCaps.CurrentFuel - FuelCost);
	Stats.TotalFuelConsumed += FuelCost;

	// Emergency stop if out of fuel
	if (NavigationCaps.CurrentFuel <= 0.0f && IsInTransit())
	{
		UE_LOG(LogTemp, Error, TEXT("JourneyManager: Out of fuel! Emergency stop."));
		EmergencyStop();
	}
}

void UJourneyManager::CheckFuelWarnings()
{
	float FuelPercent = GetFuelPercentage();

	if (FuelPercent <= FuelCriticalThreshold)
	{
		OnFuelLow.Broadcast(NavigationCaps.CurrentFuel);
		bHasWarnedLowFuel = true;
	}
	else if (FuelPercent <= FuelWarningThreshold && !bHasWarnedLowFuel)
	{
		OnFuelLow.Broadcast(NavigationCaps.CurrentFuel);
		bHasWarnedLowFuel = true;
	}
	else if (FuelPercent > FuelWarningThreshold)
	{
		bHasWarnedLowFuel = false;
	}
}

void UJourneyManager::CompleteJump()
{
	CurrentTravel.Status = ETravelStatus::Arrived;

	// Update current system
	FString OldSystem = CurrentTravel.CurrentSystemID;
	CurrentTravel.CurrentSystemID = CurrentTravel.DestinationSystemID;

	// Mark system as visited
	if (FStarSystem* System = StarSystems.Find(CurrentTravel.CurrentSystemID))
	{
		System->bIsVisited = true;
		if (!System->bIsDiscovered)
		{
			System->bIsDiscovered = true;
		}
	}

	// Update stats
	Stats.JumpsCompleted++;
	Stats.SystemsVisited++;

	// Broadcast event
	OnJumpCompleted.Broadcast(CurrentTravel.CurrentSystemID);

	// Auto-dock
	DockAtSystem(CurrentTravel.CurrentSystemID);

	UE_LOG(LogTemp, Log, TEXT("JourneyManager: Arrived at %s"), *CurrentTravel.CurrentSystemID);
}

bool UJourneyManager::InitiateJump(const FString& DestinationSystemID)
{
	FString Reason;
	if (!CanJumpTo(DestinationSystemID, Reason))
	{
		UE_LOG(LogTemp, Warning, TEXT("JourneyManager: Cannot jump to %s: %s"), *DestinationSystemID, *Reason);
		return false;
	}

	FStarSystem StartSystem;
	FStarSystem DestSystem;
	if (!GetStarSystem(CurrentTravel.CurrentSystemID, StartSystem) ||
	    !GetStarSystem(DestinationSystemID, DestSystem))
	{
		return false;
	}

	// Calculate travel parameters
	float Distance = StartSystem.DistanceTo(DestSystem);
	CurrentTravel.Status = ETravelStatus::InTransit;
	CurrentTravel.DestinationSystemID = DestinationSystemID;
	CurrentTravel.TravelProgress = 0.0f;
	CurrentTravel.DistanceRemaining = Distance;
	CurrentTravel.TravelSpeed = NavigationCaps.CruiseSpeed;
	CurrentTravel.FuelConsumptionRate = NavigationCaps.BaseFuelConsumption * NavigationCaps.GetFuelEfficiency();
	CurrentTravel.EstimatedArrivalTime = Distance / NavigationCaps.CruiseSpeed;

	// Broadcast event
	OnJumpStarted.Broadcast(DestinationSystemID);

	UE_LOG(LogTemp, Log, TEXT("JourneyManager: Jump initiated to %s (%.1f LY, ETA: %.1f days)"),
		*DestinationSystemID, Distance, CurrentTravel.EstimatedArrivalTime);

	return true;
}

bool UJourneyManager::CanJumpTo(const FString& SystemID, FString& OutReason) const
{
	// Must be docked
	if (!CurrentTravel.IsDocked())
	{
		OutReason = TEXT("Ship must be docked to initiate jump");
		return false;
	}

	// Check if system exists
	FStarSystem DestSystem;
	if (!GetStarSystem(SystemID, DestSystem))
	{
		OutReason = TEXT("System not found");
		return false;
	}

	// Check distance
	FStarSystem CurrentSystem;
	if (GetStarSystem(CurrentTravel.CurrentSystemID, CurrentSystem))
	{
		float Distance = CurrentSystem.DistanceTo(DestSystem);

		if (Distance > NavigationCaps.MaxJumpDistance)
		{
			OutReason = FString::Printf(TEXT("Distance %.1f LY exceeds max jump range %.1f LY"),
				Distance, NavigationCaps.MaxJumpDistance);
			return false;
		}

		// Check fuel
		if (!NavigationCaps.CanMakeJump(Distance))
		{
			OutReason = TEXT("Insufficient fuel for jump");
			return false;
		}
	}

	OutReason = TEXT("");
	return true;
}

bool UJourneyManager::DockAtSystem(const FString& SystemID)
{
	if (CurrentTravel.CurrentSystemID != SystemID)
	{
		return false;
	}

	CurrentTravel.Status = ETravelStatus::Docked;
	CurrentTravel.DestinationSystemID = TEXT("");
	CurrentTravel.TravelProgress = 0.0f;
	CurrentTravel.DistanceRemaining = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("JourneyManager: Docked at %s"), *SystemID);
	return true;
}

void UJourneyManager::EmergencyStop()
{
	if (!IsInTransit())
	{
		return;
	}

	CurrentTravel.Status = ETravelStatus::Emergency;
	Stats.EmergencyStops++;

	OnEmergencyStop.Broadcast();

	UE_LOG(LogTemp, Warning, TEXT("JourneyManager: Emergency stop initiated"));
}

FRoute UJourneyManager::PlanRoute(const FString& FromSystemID, const FString& ToSystemID, int32 MaxWaypoints)
{
	FRoute Route;
	Route.RouteName = FString::Printf(TEXT("Route to %s"), *ToSystemID);

	// Simple direct route for now (could implement A* pathfinding later)
	TArray<FString> Path = FindPath(FromSystemID, ToSystemID, MaxWaypoints);

	// Build waypoints
	float TotalDistance = 0.0f;
	for (int32 i = 0; i < Path.Num(); ++i)
	{
		FWaypoint Waypoint;
		Waypoint.SystemID = Path[i];
		Waypoint.SequenceNumber = i;
		Waypoint.bIsCompleted = (i == 0); // First waypoint (start) is already completed
		Route.Waypoints.Add(Waypoint);

		// Calculate distance for this leg
		if (i > 0)
		{
			FStarSystem System1, System2;
			if (GetStarSystem(Path[i - 1], System1) && GetStarSystem(Path[i], System2))
			{
				float LegDistance = System1.DistanceTo(System2);
				TotalDistance += LegDistance;
			}
		}
	}

	Route.TotalDistance = TotalDistance;
	Route.EstimatedFuelCost = TotalDistance * NavigationCaps.BaseFuelConsumption * NavigationCaps.GetFuelEfficiency();
	Route.EstimatedTravelTime = TotalDistance / NavigationCaps.CruiseSpeed;

	return Route;
}

TArray<FString> UJourneyManager::FindPath(const FString& Start, const FString& End, int32 MaxWaypoints)
{
	// Simple direct path for now
	// TODO: Implement proper pathfinding with waypoints to stay within jump range
	TArray<FString> Path;
	Path.Add(Start);
	Path.Add(End);
	return Path;
}

bool UJourneyManager::SetCurrentRoute(const FRoute& Route)
{
	CurrentRoute = Route;
	return true;
}

bool UJourneyManager::FollowRoute()
{
	if (CurrentRoute.IsComplete())
	{
		return false;
	}

	int32 CurrentWaypointIndex = CurrentRoute.GetCurrentWaypointIndex();
	if (CurrentWaypointIndex < 0 || CurrentWaypointIndex >= CurrentRoute.Waypoints.Num())
	{
		return false;
	}

	FWaypoint& NextWaypoint = CurrentRoute.Waypoints[CurrentWaypointIndex];
	return InitiateJump(NextWaypoint.SystemID);
}

bool UJourneyManager::GetStarSystem(const FString& SystemID, FStarSystem& OutSystem) const
{
	const FStarSystem* System = StarSystems.Find(SystemID);
	if (System)
	{
		OutSystem = *System;
		return true;
	}
	return false;
}

TArray<FStarSystem> UJourneyManager::GetSystemsInRange(float MaxDistance) const
{
	TArray<FStarSystem> SystemsInRange;

	FStarSystem CurrentSystem;
	if (!GetStarSystem(CurrentTravel.CurrentSystemID, CurrentSystem))
	{
		return SystemsInRange;
	}

	for (const auto& Pair : StarSystems)
	{
		if (Pair.Key == CurrentTravel.CurrentSystemID)
		{
			continue;
		}

		float Distance = CurrentSystem.DistanceTo(Pair.Value);
		if (Distance <= MaxDistance)
		{
			SystemsInRange.Add(Pair.Value);
		}
	}

	return SystemsInRange;
}

TArray<FStarSystem> UJourneyManager::GetDiscoveredSystems() const
{
	TArray<FStarSystem> Discovered;

	for (const auto& Pair : StarSystems)
	{
		if (Pair.Value.bIsDiscovered)
		{
			Discovered.Add(Pair.Value);
		}
	}

	return Discovered;
}

float UJourneyManager::CalculateDistance(const FString& SystemA, const FString& SystemB) const
{
	FStarSystem System1, System2;
	if (GetStarSystem(SystemA, System1) && GetStarSystem(SystemB, System2))
	{
		return System1.DistanceTo(System2);
	}
	return -1.0f;
}

bool UJourneyManager::RefuelShip(float Amount)
{
	float OldFuel = NavigationCaps.CurrentFuel;
	NavigationCaps.CurrentFuel = FMath::Min(NavigationCaps.FuelCapacity, NavigationCaps.CurrentFuel + Amount);
	float Refueled = NavigationCaps.CurrentFuel - OldFuel;

	UE_LOG(LogTemp, Log, TEXT("JourneyManager: Refueled %.1f units (%.1f%%)"),
		Refueled, GetFuelPercentage());

	return Refueled > 0.0f;
}

float UJourneyManager::GetFuelPercentage() const
{
	return (NavigationCaps.CurrentFuel / NavigationCaps.FuelCapacity) * 100.0f;
}

void UJourneyManager::DiscoverSystem(const FString& SystemID)
{
	if (FStarSystem* System = StarSystems.Find(SystemID))
	{
		if (!System->bIsDiscovered)
		{
			System->bIsDiscovered = true;
			UE_LOG(LogTemp, Log, TEXT("JourneyManager: Discovered new system: %s"), *System->SystemName);
		}
	}
}

void UJourneyManager::UpdateShipSystems()
{
	// TODO: Integrate with ship infrastructure
	// - Check if FTL drive is operational
	// - Verify power requirements
	// - Monitor fuel storage
}

FString UJourneyManager::SerializeToJSON() const
{
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject());

	// Serialize travel state
	TSharedPtr<FJsonObject> TravelObj = MakeShareable(new FJsonObject());
	TravelObj->SetNumberField(TEXT("Status"), static_cast<int32>(CurrentTravel.Status));
	TravelObj->SetStringField(TEXT("CurrentSystemID"), CurrentTravel.CurrentSystemID);
	TravelObj->SetStringField(TEXT("DestinationSystemID"), CurrentTravel.DestinationSystemID);
	TravelObj->SetNumberField(TEXT("TravelProgress"), CurrentTravel.TravelProgress);
	TravelObj->SetNumberField(TEXT("DistanceRemaining"), CurrentTravel.DistanceRemaining);
	TravelObj->SetNumberField(TEXT("TravelSpeed"), CurrentTravel.TravelSpeed);
	TravelObj->SetNumberField(TEXT("FuelConsumptionRate"), CurrentTravel.FuelConsumptionRate);
	TravelObj->SetNumberField(TEXT("EstimatedArrivalTime"), CurrentTravel.EstimatedArrivalTime);
	RootObject->SetObjectField(TEXT("TravelState"), TravelObj);

	// Serialize navigation capabilities
	TSharedPtr<FJsonObject> NavObj = MakeShareable(new FJsonObject());
	NavObj->SetNumberField(TEXT("MaxJumpDistance"), NavigationCaps.MaxJumpDistance);
	NavObj->SetNumberField(TEXT("CruiseSpeed"), NavigationCaps.CruiseSpeed);
	NavObj->SetNumberField(TEXT("BaseFuelConsumption"), NavigationCaps.BaseFuelConsumption);
	NavObj->SetNumberField(TEXT("FuelCapacity"), NavigationCaps.FuelCapacity);
	NavObj->SetNumberField(TEXT("CurrentFuel"), NavigationCaps.CurrentFuel);
	NavObj->SetBoolField(TEXT("bHasAdvancedSensors"), NavigationCaps.bHasAdvancedSensors);
	NavObj->SetBoolField(TEXT("bHasLongRangeScanner"), NavigationCaps.bHasLongRangeScanner);
	RootObject->SetObjectField(TEXT("NavigationCapabilities"), NavObj);

	// Serialize stats
	TSharedPtr<FJsonObject> StatsObj = MakeShareable(new FJsonObject());
	StatsObj->SetNumberField(TEXT("TotalDistanceTraveled"), Stats.TotalDistanceTraveled);
	StatsObj->SetNumberField(TEXT("SystemsVisited"), Stats.SystemsVisited);
	StatsObj->SetNumberField(TEXT("JumpsCompleted"), Stats.JumpsCompleted);
	StatsObj->SetNumberField(TEXT("TotalFuelConsumed"), Stats.TotalFuelConsumed);
	StatsObj->SetNumberField(TEXT("TotalTravelTime"), Stats.TotalTravelTime);
	StatsObj->SetNumberField(TEXT("EncountersExperienced"), Stats.EncountersExperienced);
	StatsObj->SetNumberField(TEXT("EmergencyStops"), Stats.EmergencyStops);
	RootObject->SetObjectField(TEXT("JourneyStats"), StatsObj);

	// Serialize discovered/visited systems
	TArray<TSharedPtr<FJsonValue>> DiscoveredArray;
	TArray<TSharedPtr<FJsonValue>> VisitedArray;
	for (const auto& Pair : StarSystems)
	{
		if (Pair.Value.bIsDiscovered)
		{
			DiscoveredArray.Add(MakeShareable(new FJsonValueString(Pair.Key)));
		}
		if (Pair.Value.bIsVisited)
		{
			VisitedArray.Add(MakeShareable(new FJsonValueString(Pair.Key)));
		}
	}
	RootObject->SetArrayField(TEXT("DiscoveredSystems"), DiscoveredArray);
	RootObject->SetArrayField(TEXT("VisitedSystems"), VisitedArray);

	RootObject->SetNumberField(TEXT("GameTime"), GameTime);

	// Convert to JSON string
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	return OutputString;
}

bool UJourneyManager::DeserializeFromJSON(const FString& JSON)
{
	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JSON);

	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("JourneyManager: Failed to parse JSON"));
		return false;
	}

	// Deserialize travel state
	const TSharedPtr<FJsonObject>* TravelObj;
	if (RootObject->TryGetObjectField(TEXT("TravelState"), TravelObj))
	{
		CurrentTravel.Status = static_cast<ETravelStatus>((*TravelObj)->GetIntegerField(TEXT("Status")));
		CurrentTravel.CurrentSystemID = (*TravelObj)->GetStringField(TEXT("CurrentSystemID"));
		CurrentTravel.DestinationSystemID = (*TravelObj)->GetStringField(TEXT("DestinationSystemID"));
		CurrentTravel.TravelProgress = (*TravelObj)->GetNumberField(TEXT("TravelProgress"));
		CurrentTravel.DistanceRemaining = (*TravelObj)->GetNumberField(TEXT("DistanceRemaining"));
		CurrentTravel.TravelSpeed = (*TravelObj)->GetNumberField(TEXT("TravelSpeed"));
		CurrentTravel.FuelConsumptionRate = (*TravelObj)->GetNumberField(TEXT("FuelConsumptionRate"));
		CurrentTravel.EstimatedArrivalTime = (*TravelObj)->GetNumberField(TEXT("EstimatedArrivalTime"));
	}

	// Deserialize navigation capabilities
	const TSharedPtr<FJsonObject>* NavObj;
	if (RootObject->TryGetObjectField(TEXT("NavigationCapabilities"), NavObj))
	{
		NavigationCaps.MaxJumpDistance = (*NavObj)->GetNumberField(TEXT("MaxJumpDistance"));
		NavigationCaps.CruiseSpeed = (*NavObj)->GetNumberField(TEXT("CruiseSpeed"));
		NavigationCaps.BaseFuelConsumption = (*NavObj)->GetNumberField(TEXT("BaseFuelConsumption"));
		NavigationCaps.FuelCapacity = (*NavObj)->GetNumberField(TEXT("FuelCapacity"));
		NavigationCaps.CurrentFuel = (*NavObj)->GetNumberField(TEXT("CurrentFuel"));
		NavigationCaps.bHasAdvancedSensors = (*NavObj)->GetBoolField(TEXT("bHasAdvancedSensors"));
		NavigationCaps.bHasLongRangeScanner = (*NavObj)->GetBoolField(TEXT("bHasLongRangeScanner"));
	}

	// Deserialize stats
	const TSharedPtr<FJsonObject>* StatsObj;
	if (RootObject->TryGetObjectField(TEXT("JourneyStats"), StatsObj))
	{
		Stats.TotalDistanceTraveled = (*StatsObj)->GetNumberField(TEXT("TotalDistanceTraveled"));
		Stats.SystemsVisited = (*StatsObj)->GetIntegerField(TEXT("SystemsVisited"));
		Stats.JumpsCompleted = (*StatsObj)->GetIntegerField(TEXT("JumpsCompleted"));
		Stats.TotalFuelConsumed = (*StatsObj)->GetNumberField(TEXT("TotalFuelConsumed"));
		Stats.TotalTravelTime = (*StatsObj)->GetNumberField(TEXT("TotalTravelTime"));
		Stats.EncountersExperienced = (*StatsObj)->GetIntegerField(TEXT("EncountersExperienced"));
		Stats.EmergencyStops = (*StatsObj)->GetIntegerField(TEXT("EmergencyStops"));
	}

	// Deserialize discovered/visited systems
	const TArray<TSharedPtr<FJsonValue>>* DiscoveredArray;
	if (RootObject->TryGetArrayField(TEXT("DiscoveredSystems"), DiscoveredArray))
	{
		for (const TSharedPtr<FJsonValue>& Value : *DiscoveredArray)
		{
			FString SystemID = Value->AsString();
			if (FStarSystem* System = StarSystems.Find(SystemID))
			{
				System->bIsDiscovered = true;
			}
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* VisitedArray;
	if (RootObject->TryGetArrayField(TEXT("VisitedSystems"), VisitedArray))
	{
		for (const TSharedPtr<FJsonValue>& Value : *VisitedArray)
		{
			FString SystemID = Value->AsString();
			if (FStarSystem* System = StarSystems.Find(SystemID))
			{
				System->bIsVisited = true;
			}
		}
	}

	GameTime = RootObject->GetNumberField(TEXT("GameTime"));

	UE_LOG(LogTemp, Log, TEXT("JourneyManager: Deserialized journey state successfully"));
	return true;
}
