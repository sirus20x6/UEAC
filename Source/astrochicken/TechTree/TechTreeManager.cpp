// Source/astrochicken/TechTree/TechTreeManager.cpp

#include "TechTreeManager.h"
#include "../Race/RaceData.h"

void UTechTreeManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("TechTreeManager: Initializing"));

	InitializeTechDatabase();
	LoadTechState();
}

void UTechTreeManager::Deinitialize()
{
	SaveTechState();
	Super::Deinitialize();
}

void UTechTreeManager::InitializeTechDatabase()
{
	AllTechs.Empty();

	// PHYSICS TECHS
	{
		// Tier 1
		AllTechs.Add(CreateTechNode(
			TEXT("PHY_BasicPropulsion"),
			TEXT("Basic Propulsion"),
			ETechCategory::Physics,
			ETechTier::Tier1,
			FText::FromString(TEXT("Fundamental rocket propulsion systems")),
			FText::FromString(TEXT("Unlocks basic ship movement and maneuverability. +10% thrust efficiency."))
		));

		AllTechs.Add(CreateTechNode(
			TEXT("PHY_NewtonianMechanics"),
			TEXT("Newtonian Mechanics"),
			ETechCategory::Physics,
			ETechTier::Tier1,
			FText::FromString(TEXT("Understanding of classical mechanics in space")),
			FText::FromString(TEXT("Improves trajectory calculations. +15% fuel efficiency during navigation."))
		));

		// Tier 2
		AllTechs.Add(CreateTechNode(
			TEXT("PHY_IonDrive"),
			TEXT("Ion Drive Technology"),
			ETechCategory::Physics,
			ETechTier::Tier2,
			FText::FromString(TEXT("Advanced ion propulsion systems")),
			FText::FromString(TEXT("Enables efficient long-distance travel. +25% fuel efficiency, +10% max speed.")),
			{FTechPrerequisite(ETechCategory::Physics, 15)}
		));

		AllTechs.Add(CreateTechNode(
			TEXT("PHY_QuantumMechanics"),
			TEXT("Quantum Mechanics"),
			ETechCategory::Physics,
			ETechTier::Tier2,
			FText::FromString(TEXT("Understanding quantum phenomena")),
			FText::FromString(TEXT("Foundation for advanced technologies. Unlocks quantum-based systems.")),
			{FTechPrerequisite(ETechCategory::Physics, 20), FTechPrerequisite(ETechCategory::Computing, 15)}
		));

		// Tier 3
		AllTechs.Add(CreateTechNode(
			TEXT("PHY_FusionPropulsion"),
			TEXT("Fusion Propulsion"),
			ETechCategory::Physics,
			ETechTier::Tier3,
			FText::FromString(TEXT("Nuclear fusion-based propulsion")),
			FText::FromString(TEXT("Massive increase in thrust and efficiency. +50% max speed, +40% fuel efficiency.")),
			{FTechPrerequisite(ETechCategory::Physics, 30), FTechPrerequisite(ETechCategory::Energy, 25)}
		));

		AllTechs.Add(CreateTechNode(
			TEXT("PHY_GravityManipulation"),
			TEXT("Gravity Manipulation"),
			ETechCategory::Physics,
			ETechTier::Tier3,
			FText::FromString(TEXT("Basic gravitational field control")),
			FText::FromString(TEXT("Enables artificial gravity and gravity-assisted maneuvers. +20% crew comfort.")),
			{FTechPrerequisite(ETechCategory::Physics, 35)}
		));
	}

	// ENGINEERING TECHS
	{
		// Tier 1
		AllTechs.Add(CreateTechNode(
			TEXT("ENG_BasicConstruction"),
			TEXT("Basic Construction"),
			ETechCategory::Engineering,
			ETechTier::Tier1,
			FText::FromString(TEXT("Fundamental construction techniques")),
			FText::FromString(TEXT("Enables building basic structures. +10% construction speed."))
		));

		AllTechs.Add(CreateTechNode(
			TEXT("ENG_MaterialScience"),
			TEXT("Material Science"),
			ETechCategory::Engineering,
			ETechTier::Tier1,
			FText::FromString(TEXT("Understanding material properties")),
			FText::FromString(TEXT("Improves hull durability. +15% structural integrity."))
		));

		// Tier 2
		AllTechs.Add(CreateTechNode(
			TEXT("ENG_AdvancedAlloys"),
			TEXT("Advanced Alloys"),
			ETechCategory::Engineering,
			ETechTier::Tier2,
			FText::FromString(TEXT("High-performance composite materials")),
			FText::FromString(TEXT("Stronger, lighter structures. +25% hull strength, -10% ship mass.")),
			{FTechPrerequisite(ETechCategory::Engineering, 15)}
		));

		AllTechs.Add(CreateTechNode(
			TEXT("ENG_ModularDesign"),
			TEXT("Modular Design"),
			ETechCategory::Engineering,
			ETechTier::Tier2,
			FText::FromString(TEXT("Standardized modular components")),
			FText::FromString(TEXT("Faster repairs and upgrades. +30% repair speed, +20% modification flexibility.")),
			{FTechPrerequisite(ETechCategory::Engineering, 20)}
		));

		// Tier 3
		AllTechs.Add(CreateTechNode(
			TEXT("ENG_NanofabricationTech"),
			TEXT("Nanofabrication"),
			ETechCategory::Engineering,
			ETechTier::Tier3,
			FText::FromString(TEXT("Molecular-scale manufacturing")),
			FText::FromString(TEXT("Revolutionary manufacturing capabilities. +50% resource efficiency, enables self-repair.")),
			{FTechPrerequisite(ETechCategory::Engineering, 30), FTechPrerequisite(ETechCategory::Computing, 20)}
		));
	}

	// ENERGY TECHS
	{
		// Tier 1
		AllTechs.Add(CreateTechNode(
			TEXT("ENR_SolarPower"),
			TEXT("Solar Power"),
			ETechCategory::Energy,
			ETechTier::Tier1,
			FText::FromString(TEXT("Photovoltaic energy collection")),
			FText::FromString(TEXT("Basic renewable energy. +20% passive energy generation."))
		));

		AllTechs.Add(CreateTechNode(
			TEXT("ENR_FissionReactor"),
			TEXT("Fission Reactor"),
			ETechCategory::Energy,
			ETechTier::Tier1,
			FText::FromString(TEXT("Nuclear fission power generation")),
			FText::FromString(TEXT("Reliable power source. +50% energy capacity."))
		));

		// Tier 2
		AllTechs.Add(CreateTechNode(
			TEXT("ENR_FusionReactor"),
			TEXT("Fusion Reactor"),
			ETechCategory::Energy,
			ETechTier::Tier2,
			FText::FromString(TEXT("Nuclear fusion power generation")),
			FText::FromString(TEXT("Clean, abundant energy. +100% energy capacity, +50% efficiency.")),
			{FTechPrerequisite(ETechCategory::Energy, 20), FTechPrerequisite(ETechCategory::Physics, 15)}
		));

		AllTechs.Add(CreateTechNode(
			TEXT("ENR_PowerDistribution"),
			TEXT("Advanced Power Distribution"),
			ETechCategory::Energy,
			ETechTier::Tier2,
			FText::FromString(TEXT("Efficient energy transfer systems")),
			FText::FromString(TEXT("Reduces power loss. +25% grid efficiency, faster power rerouting.")),
			{FTechPrerequisite(ETechCategory::Energy, 15), FTechPrerequisite(ETechCategory::Engineering, 10)}
		));

		// Tier 3
		AllTechs.Add(CreateTechNode(
			TEXT("ENR_AntimatterReactor"),
			TEXT("Antimatter Reactor"),
			ETechCategory::Energy,
			ETechTier::Tier3,
			FText::FromString(TEXT("Matter-antimatter annihilation power")),
			FText::FromString(TEXT("Ultimate power source. +300% energy capacity, enables energy weapons.")),
			{FTechPrerequisite(ETechCategory::Energy, 35), FTechPrerequisite(ETechCategory::Physics, 30)}
		));
	}

	// COMPUTING TECHS
	{
		// Tier 1
		AllTechs.Add(CreateTechNode(
			TEXT("COM_BasicAutomation"),
			TEXT("Basic Automation"),
			ETechCategory::Computing,
			ETechTier::Tier1,
			FText::FromString(TEXT("Simple automated systems")),
			FText::FromString(TEXT("Reduces crew workload. +10% efficiency on automated tasks."))
		));

		AllTechs.Add(CreateTechNode(
			TEXT("COM_DataProcessing"),
			TEXT("Data Processing"),
			ETechCategory::Computing,
			ETechTier::Tier1,
			FText::FromString(TEXT("Advanced computational algorithms")),
			FText::FromString(TEXT("Faster calculations. +15% sensor analysis speed."))
		));

		// Tier 2
		AllTechs.Add(CreateTechNode(
			TEXT("COM_QuantumComputing"),
			TEXT("Quantum Computing"),
			ETechCategory::Computing,
			ETechTier::Tier2,
			FText::FromString(TEXT("Quantum-based computation")),
			FText::FromString(TEXT("Exponential processing power. +100% calculation speed, enables advanced AI.")),
			{FTechPrerequisite(ETechCategory::Computing, 20), FTechPrerequisite(ETechCategory::Physics, 15)}
		));

		AllTechs.Add(CreateTechNode(
			TEXT("COM_AdvancedAI"),
			TEXT("Advanced AI"),
			ETechCategory::Computing,
			ETechTier::Tier2,
			FText::FromString(TEXT("Sophisticated artificial intelligence")),
			FText::FromString(TEXT("AI assistants for complex tasks. +25% automation efficiency, +20% decision quality.")),
			{FTechPrerequisite(ETechCategory::Computing, 25)}
		));

		// Tier 3
		AllTechs.Add(CreateTechNode(
			TEXT("COM_SentientAI"),
			TEXT("Sentient AI"),
			ETechCategory::Computing,
			ETechTier::Tier3,
			FText::FromString(TEXT("Self-aware artificial intelligence")),
			FText::FromString(TEXT("Revolutionary AI capabilities. +50% to all automated systems, unlocks AI crew.")),
			{FTechPrerequisite(ETechCategory::Computing, 35), FTechPrerequisite(ETechCategory::Biology, 20)}
		));
	}

	// BIOLOGY TECHS
	{
		// Tier 1
		AllTechs.Add(CreateTechNode(
			TEXT("BIO_LifeSupport"),
			TEXT("Life Support Systems"),
			ETechCategory::Biology,
			ETechTier::Tier1,
			FText::FromString(TEXT("Basic environmental control")),
			FText::FromString(TEXT("Maintains habitable conditions. +20% crew survivability."))
		));

		AllTechs.Add(CreateTechNode(
			TEXT("BIO_Hydroponics"),
			TEXT("Hydroponics"),
			ETechCategory::Biology,
			ETechTier::Tier1,
			FText::FromString(TEXT("Soilless plant cultivation")),
			FText::FromString(TEXT("Food production in space. +30% food generation, +10% crew morale."))
		));

		// Tier 2
		AllTechs.Add(CreateTechNode(
			TEXT("BIO_GeneticEngineering"),
			TEXT("Genetic Engineering"),
			ETechCategory::Biology,
			ETechTier::Tier2,
			FText::FromString(TEXT("Direct DNA manipulation")),
			FText::FromString(TEXT("Improve species traits. +15% to all race attributes, enables trait modifications.")),
			{FTechPrerequisite(ETechCategory::Biology, 20)}
		));

		AllTechs.Add(CreateTechNode(
			TEXT("BIO_MedicalAdvances"),
			TEXT("Advanced Medicine"),
			ETechCategory::Biology,
			ETechTier::Tier2,
			FText::FromString(TEXT("Cutting-edge medical technology")),
			FText::FromString(TEXT("Superior healthcare. +40% healing rate, +25% disease resistance.")),
			{FTechPrerequisite(ETechCategory::Biology, 15)}
		));

		// Tier 3
		AllTechs.Add(CreateTechNode(
			TEXT("BIO_Cybernetics"),
			TEXT("Cybernetics"),
			ETechCategory::Biology,
			ETechTier::Tier3,
			FText::FromString(TEXT("Bio-mechanical integration")),
			FText::FromString(TEXT("Enhance crew with technology. +30% crew efficiency, +20% resilience.")),
			{FTechPrerequisite(ETechCategory::Biology, 30), FTechPrerequisite(ETechCategory::Engineering, 25)}
		));

		AllTechs.Add(CreateTechNode(
			TEXT("BIO_CloningTechnology"),
			TEXT("Cloning Technology"),
			ETechCategory::Biology,
			ETechTier::Tier3,
			FText::FromString(TEXT("Rapid organism replication")),
			FText::FromString(TEXT("Accelerated population growth. +50% crew replacement rate, enables rapid colonization.")),
			{FTechPrerequisite(ETechCategory::Biology, 35)}
		));
	}

	// SENSORS TECHS
	{
		// Tier 1
		AllTechs.Add(CreateTechNode(
			TEXT("SEN_BasicSensors"),
			TEXT("Basic Sensors"),
			ETechCategory::Sensors,
			ETechTier::Tier1,
			FText::FromString(TEXT("Fundamental detection systems")),
			FText::FromString(TEXT("Detect nearby objects. +30% detection range."))
		));

		AllTechs.Add(CreateTechNode(
			TEXT("SEN_SpectralAnalysis"),
			TEXT("Spectral Analysis"),
			ETechCategory::Sensors,
			ETechTier::Tier1,
			FText::FromString(TEXT("Electromagnetic spectrum analysis")),
			FText::FromString(TEXT("Identify materials remotely. +40% resource scanning accuracy."))
		));

		// Tier 2
		AllTechs.Add(CreateTechNode(
			TEXT("SEN_LongRangeSensors"),
			TEXT("Long Range Sensors"),
			ETechCategory::Sensors,
			ETechTier::Tier2,
			FText::FromString(TEXT("Extended detection capabilities")),
			FText::FromString(TEXT("Scan distant systems. +100% detection range, early threat warning.")),
			{FTechPrerequisite(ETechCategory::Sensors, 15)}
		));

		AllTechs.Add(CreateTechNode(
			TEXT("SEN_GravimetricSensors"),
			TEXT("Gravimetric Sensors"),
			ETechCategory::Sensors,
			ETechTier::Tier2,
			FText::FromString(TEXT("Gravity field detection")),
			FText::FromString(TEXT("Detect massive objects. +50% asteroid detection, improves navigation safety.")),
			{FTechPrerequisite(ETechCategory::Sensors, 20), FTechPrerequisite(ETechCategory::Physics, 15)}
		));

		// Tier 3
		AllTechs.Add(CreateTechNode(
			TEXT("SEN_QuantumSensors"),
			TEXT("Quantum Sensors"),
			ETechCategory::Sensors,
			ETechTier::Tier3,
			FText::FromString(TEXT("Quantum-entanglement based detection")),
			FText::FromString(TEXT("Unprecedented sensor accuracy. +200% detection range, reveals hidden phenomena.")),
			{FTechPrerequisite(ETechCategory::Sensors, 30), FTechPrerequisite(ETechCategory::Physics, 25), FTechPrerequisite(ETechCategory::Computing, 20)}
		));
	}

	UE_LOG(LogTemp, Log, TEXT("TechTreeManager: Initialized %d technologies"), AllTechs.Num());
}

FTechNode UTechTreeManager::CreateTechNode(
	const FString& TechID,
	const FString& TechName,
	ETechCategory Category,
	ETechTier Tier,
	const FText& Description,
	const FText& DetailedEffect,
	const TArray<FTechPrerequisite>& Prerequisites)
{
	FTechNode Node;
	Node.TechID = TechID;
	Node.TechName = TechName;
	Node.Category = Category;
	Node.Tier = Tier;
	Node.Description = Description;
	Node.DetailedEffect = DetailedEffect;
	Node.Prerequisites = Prerequisites;
	Node.bIsUnlocked = false;

	return Node;
}

TArray<FTechNode> UTechTreeManager::GetTechsByCategory(ETechCategory Category) const
{
	return AllTechs.FilterByPredicate([Category](const FTechNode& Tech)
	{
		return Tech.Category == Category;
	});
}

TArray<FTechNode> UTechTreeManager::GetTechsByTier(ETechTier Tier) const
{
	return AllTechs.FilterByPredicate([Tier](const FTechNode& Tech)
	{
		return Tech.Tier == Tier;
	});
}

FTechNode UTechTreeManager::GetTechByID(const FString& TechID) const
{
	const FTechNode* Found = AllTechs.FindByPredicate([&TechID](const FTechNode& Tech)
	{
		return Tech.TechID == TechID;
	});

	return Found ? *Found : FTechNode();
}

bool UTechTreeManager::CheckPrerequisites(const TArray<FTechPrerequisite>& Prerequisites) const
{
	for (const FTechPrerequisite& Prereq : Prerequisites)
	{
		// Check point requirements
		if (PlayerTechState.AllocatedPoints.GetPoints(Prereq.Category) < Prereq.MinimumPoints)
		{
			return false;
		}

		// Check specific tech requirements
		if (!Prereq.RequiredTechID.IsEmpty())
		{
			if (!PlayerTechState.UnlockedTechs.Contains(Prereq.RequiredTechID))
			{
				return false;
			}
		}
	}

	return true;
}

bool UTechTreeManager::CanUnlockTech(const FString& TechID) const
{
	const FTechNode* Tech = AllTechs.FindByPredicate([&TechID](const FTechNode& T)
	{
		return T.TechID == TechID;
	});

	if (!Tech)
	{
		return false;
	}

	if (Tech->bIsUnlocked)
	{
		return false;
	}

	return CheckPrerequisites(Tech->Prerequisites);
}

bool UTechTreeManager::UnlockTech(const FString& TechID)
{
	if (!CanUnlockTech(TechID))
	{
		return false;
	}

	// Find and unlock in main array
	for (FTechNode& Tech : AllTechs)
	{
		if (Tech.TechID == TechID)
		{
			Tech.bIsUnlocked = true;
			PlayerTechState.UnlockedTechs.AddUnique(TechID);

			UE_LOG(LogTemp, Log, TEXT("TechTreeManager: Unlocked tech %s"), *Tech.TechName);
			SaveTechState();
			return true;
		}
	}

	return false;
}

bool UTechTreeManager::IsTechUnlocked(const FString& TechID) const
{
	return PlayerTechState.UnlockedTechs.Contains(TechID);
}

TArray<FTechNode> UTechTreeManager::GetAvailableTechs() const
{
	return AllTechs.FilterByPredicate([this](const FTechNode& Tech)
	{
		return !Tech.bIsUnlocked && CheckPrerequisites(Tech.Prerequisites);
	});
}

int32 UTechTreeManager::GetCategoryPoints(ETechCategory Category) const
{
	return PlayerTechState.AllocatedPoints.GetPoints(Category);
}

void UTechTreeManager::SetCategoryPoints(ETechCategory Category, int32 Points)
{
	PlayerTechState.AllocatedPoints.SetPoints(Category, Points);
}

void UTechTreeManager::AddCategoryPoints(ETechCategory Category, int32 Points)
{
	PlayerTechState.AllocatedPoints.AddPoints(Category, Points);

	// Check if any new techs can be unlocked
	TArray<FTechNode> Available = GetAvailableTechs();
	UE_LOG(LogTemp, Log, TEXT("TechTreeManager: %d techs now available"), Available.Num());
}

void UTechTreeManager::InitializeTechPointsWithRaceBonus(const FTechBonuses& RaceBonuses)
{
	// Start with base points
	PlayerTechState.AllocatedPoints.Physics = BaseTechPoints + RaceBonuses.Physics;
	PlayerTechState.AllocatedPoints.Engineering = BaseTechPoints + RaceBonuses.Engineering;
	PlayerTechState.AllocatedPoints.Energy = BaseTechPoints + RaceBonuses.Energy;
	PlayerTechState.AllocatedPoints.Computing = BaseTechPoints + RaceBonuses.Computing;
	PlayerTechState.AllocatedPoints.Biology = BaseTechPoints + RaceBonuses.Biology;
	PlayerTechState.AllocatedPoints.Sensors = BaseTechPoints + RaceBonuses.Sensors;

	UE_LOG(LogTemp, Log, TEXT("TechTreeManager: Initialized tech points with race bonuses. Total: %d"),
		PlayerTechState.AllocatedPoints.GetTotalPoints());
}

void UTechTreeManager::AllocateBonusPoints(ETechCategory Category, int32 Points, int32& RemainingBonusPoints)
{
	if (Points <= RemainingBonusPoints && Points > 0)
	{
		AddCategoryPoints(Category, Points);
		RemainingBonusPoints -= Points;

		UE_LOG(LogTemp, Log, TEXT("TechTreeManager: Allocated %d bonus points to category. Remaining: %d"),
			Points, RemainingBonusPoints);
	}
}

bool UTechTreeManager::StartResearch(const FString& TechID)
{
	if (!CanUnlockTech(TechID))
	{
		return false;
	}

	PlayerTechState.CurrentlyResearchingTech = TechID;

	// Find and mark as researching
	for (FTechNode& Tech : AllTechs)
	{
		if (Tech.TechID == TechID)
		{
			Tech.bIsResearching = true;
			Tech.ResearchProgress = 0.0f;
			UE_LOG(LogTemp, Log, TEXT("TechTreeManager: Started research on %s"), *Tech.TechName);
			return true;
		}
	}

	return false;
}

void UTechTreeManager::UpdateResearch(float DeltaTime)
{
	if (PlayerTechState.CurrentlyResearchingTech.IsEmpty())
	{
		return;
	}

	// Find researching tech
	for (FTechNode& Tech : AllTechs)
	{
		if (Tech.TechID == PlayerTechState.CurrentlyResearchingTech && Tech.bIsResearching)
		{
			Tech.ResearchProgress += DeltaTime * 0.01f; // Adjust speed as needed

			if (Tech.ResearchProgress >= 1.0f)
			{
				// Research complete
				UnlockTech(Tech.TechID);
				Tech.bIsResearching = false;
				Tech.ResearchProgress = 1.0f;
				PlayerTechState.CurrentlyResearchingTech.Empty();

				UE_LOG(LogTemp, Log, TEXT("TechTreeManager: Research complete for %s"), *Tech.TechName);
			}
			break;
		}
	}
}

void UTechTreeManager::ResetTechTree()
{
	// Clear all unlocks
	PlayerTechState.UnlockedTechs.Empty();
	PlayerTechState.CurrentlyResearchingTech.Empty();

	// Reset all tech nodes
	for (FTechNode& Tech : AllTechs)
	{
		Tech.bIsUnlocked = false;
		Tech.bIsResearching = false;
		Tech.ResearchProgress = 0.0f;
	}

	// Reset to base points
	PlayerTechState.AllocatedPoints = FTechPoints();

	UE_LOG(LogTemp, Log, TEXT("TechTreeManager: Tech tree reset"));
	SaveTechState();
}

void UTechTreeManager::SaveTechState()
{
	// TODO: Integrate with save system
	UE_LOG(LogTemp, Log, TEXT("TechTreeManager: SaveTechState called (not yet implemented)"));
}

void UTechTreeManager::LoadTechState()
{
	// TODO: Integrate with save system
	UE_LOG(LogTemp, Log, TEXT("TechTreeManager: LoadTechState called (not yet implemented)"));
}

void UTechTreeManager::SetPlayerTechState(const FPlayerTechState& NewState)
{
	PlayerTechState = NewState;

	// Update AllTechs array to match loaded state
	for (FTechNode& Tech : AllTechs)
	{
		Tech.bIsUnlocked = PlayerTechState.UnlockedTechs.Contains(Tech.TechID);
		Tech.bIsResearching = (Tech.TechID == PlayerTechState.CurrentlyResearchingTech);
	}

	UE_LOG(LogTemp, Log, TEXT("TechTreeManager: Loaded tech state with %d unlocked techs"),
		PlayerTechState.UnlockedTechs.Num());
}
