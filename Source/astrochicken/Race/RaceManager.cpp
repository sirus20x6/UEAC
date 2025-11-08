// Source/astrochicken/Race/RaceManager.cpp

#include "RaceManager.h"
#include "Kismet/GameplayStatics.h"

void URaceManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("RaceManager: Initializing"));

	InitializeRaceDatabase();
	LoadUnlockState();
}

void URaceManager::Deinitialize()
{
	SaveUnlockState();
	Super::Deinitialize();
}

void URaceManager::InitializeRaceDatabase()
{
	AllRaces.Empty();

	// PLAYABLE RACES (Unlocked by default)

	// 1. Chordata (Vertebranes) - Social leaders, intelligence-focused
	{
		FRaceAttributes Attrs;
		Attrs.Intelligence = 70.0f;
		Attrs.Social = 75.0f;
		Attrs.Adaptability = 60.0f;
		Attrs.Resilience = 55.0f;
		Attrs.Efficiency = 50.0f;
		Attrs.SpaceAdaptation = 45.0f;
		Attrs.ResourceExtraction = 50.0f;
		Attrs.TechAffinity = 65.0f;

		FTechBonuses Tech;
		Tech.Computing = 3;
		Tech.Biology = 2;
		Tech.Engineering = 1;

		FText Lore = FText::FromString(
			TEXT("The Vertebranes evolved from vertebrate ancestors, developing complex social structures ")
			TEXT("and advanced cognitive abilities. Their hierarchical societies and natural leadership ")
			TEXT("make them excellent coordinators of large-scale colonization efforts. While not the most ")
			TEXT("physically resilient, their intelligence and social cohesion allow them to overcome challenges ")
			TEXT("through cooperation and innovation.")
		);

		AllRaces.Add(CreateRaceData(
			EPhylum::Chordata,
			TEXT("Vertebranes"),
			TEXT("Chordata"),
			TEXT("Intelligent social organizers with natural leadership abilities"),
			Lore,
			Attrs,
			Tech,
			true // Unlocked by default
		));
	}

	// 2. Arthropoda (Chitinoids) - Resilient, efficient, resource extraction specialists
	{
		FRaceAttributes Attrs;
		Attrs.Resilience = 75.0f;
		Attrs.Efficiency = 70.0f;
		Attrs.ResourceExtraction = 75.0f;
		Attrs.Adaptability = 60.0f;
		Attrs.Intelligence = 55.0f;
		Attrs.Social = 50.0f;
		Attrs.SpaceAdaptation = 55.0f;
		Attrs.TechAffinity = 50.0f;

		FTechBonuses Tech;
		Tech.Engineering = 3;
		Tech.Energy = 2;
		Tech.Physics = 1;

		FText Lore = FText::FromString(
			TEXT("The Chitinoids evolved from arthropod stock, developing incredibly efficient metabolisms ")
			TEXT("and hardy exoskeletons. Their segmented bodies and modular biology make them naturally ")
			TEXT("suited to space travel and resource extraction. Chitinoid colonies operate with remarkable ")
			TEXT("efficiency, wasting nothing and maximizing every resource. Their tough exteriors and ")
			TEXT("adaptable physiology allow them to survive in the harshest environments.")
		);

		AllRaces.Add(CreateRaceData(
			EPhylum::Arthropoda,
			TEXT("Chitinoids"),
			TEXT("Arthropoda"),
			TEXT("Efficient and resilient resource extraction specialists"),
			Lore,
			Attrs,
			Tech,
			true
		));
	}

	// 3. Cnidaria (Medusians) - Highly adaptable, space-adapted
	{
		FRaceAttributes Attrs;
		Attrs.Adaptability = 80.0f;
		Attrs.SpaceAdaptation = 75.0f;
		Attrs.Resilience = 60.0f;
		Attrs.Efficiency = 65.0f;
		Attrs.Intelligence = 60.0f;
		Attrs.Social = 55.0f;
		Attrs.ResourceExtraction = 50.0f;
		Attrs.TechAffinity = 55.0f;

		FTechBonuses Tech;
		Tech.Biology = 3;
		Tech.Sensors = 2;
		Tech.Energy = 1;

		FText Lore = FText::FromString(
			TEXT("The Medusians descended from radially symmetric aquatic organisms, developing the ability ")
			TEXT("to survive in zero-gravity environments more easily than most other species. Their distributed ")
			TEXT("nervous systems and regenerative capabilities make them incredibly adaptable to new environments. ")
			TEXT("Medusian colonies can quickly adjust to changing conditions, and their unique sensory organs ")
			TEXT("allow them to perceive phenomena invisible to other races.")
		);

		AllRaces.Add(CreateRaceData(
			EPhylum::Cnidaria,
			TEXT("Medusians"),
			TEXT("Cnidaria"),
			TEXT("Highly adaptable space-adapted explorers"),
			Lore,
			Attrs,
			Tech,
			true
		));
	}

	// 4. Mollusca (Cephalopods) - Intelligent, technology-focused
	{
		FRaceAttributes Attrs;
		Attrs.Intelligence = 80.0f;
		Attrs.TechAffinity = 75.0f;
		Attrs.Adaptability = 65.0f;
		Attrs.Social = 45.0f;
		Attrs.Efficiency = 60.0f;
		Attrs.Resilience = 50.0f;
		Attrs.SpaceAdaptation = 55.0f;
		Attrs.ResourceExtraction = 50.0f;

		FTechBonuses Tech;
		Tech.Computing = 3;
		Tech.Physics = 2;
		Tech.Sensors = 2;

		FText Lore = FText::FromString(
			TEXT("The Cephalopods are masters of technology and problem-solving, evolved from highly intelligent ")
			TEXT("marine mollusks. Their multiple manipulator appendages and complex brains make them natural ")
			TEXT("engineers and scientists. While less socially inclined than some races, their individual brilliance ")
			TEXT("and technological innovation make up for smaller colony sizes. Cephalopod technology often appears ")
			TEXT("elegant and efficient, reflecting their sophisticated understanding of physics and engineering.")
		);

		AllRaces.Add(CreateRaceData(
			EPhylum::Mollusca,
			TEXT("Cephalopods"),
			TEXT("Mollusca"),
			TEXT("Brilliant technological innovators and engineers"),
			Lore,
			Attrs,
			Tech,
			true
		));
	}

	// UNLOCKABLE RACES (Future content)

	// 5. Echinodermata (Starforms)
	{
		FRaceAttributes Attrs;
		Attrs.Resilience = 70.0f;
		Attrs.Adaptability = 70.0f;
		Attrs.SpaceAdaptation = 65.0f;
		Attrs.Intelligence = 50.0f;
		Attrs.Social = 60.0f;
		Attrs.Efficiency = 55.0f;
		Attrs.ResourceExtraction = 60.0f;
		Attrs.TechAffinity = 50.0f;

		FTechBonuses Tech;
		Tech.Biology = 2;
		Tech.Engineering = 2;
		Tech.Energy = 1;

		FText Lore = FText::FromString(
			TEXT("The Starforms evolved from radially symmetric echinoderms, developing remarkable regenerative ")
			TEXT("abilities and distributed intelligence. Their pentaradial symmetry allows them to approach ")
			TEXT("problems from multiple angles simultaneously. Starform colonies exhibit collective decision-making ")
			TEXT("and can regenerate lost members from fragments, making them remarkably resilient to disasters.")
		);

		AllRaces.Add(CreateRaceData(
			EPhylum::Echinodermata,
			TEXT("Starforms"),
			TEXT("Echinodermata"),
			TEXT("Regenerative collective with distributed intelligence"),
			Lore,
			Attrs,
			Tech,
			false // Locked by default
		));
	}

	// 6. Annelida (Segmentines)
	{
		FRaceAttributes Attrs;
		Attrs.Efficiency = 75.0f;
		Attrs.Adaptability = 65.0f;
		Attrs.Resilience = 65.0f;
		Attrs.ResourceExtraction = 70.0f;
		Attrs.Intelligence = 50.0f;
		Attrs.Social = 65.0f;
		Attrs.SpaceAdaptation = 50.0f;
		Attrs.TechAffinity = 45.0f;

		FTechBonuses Tech;
		Tech.Biology = 2;
		Tech.Engineering = 2;
		Tech.Physics = 1;

		FText Lore = FText::FromString(
			TEXT("The Segmentines evolved from segmented worms, developing modular body plans that allow for ")
			TEXT("incredible adaptability. Each segment can specialize for different tasks, making Segmentine ")
			TEXT("colonies highly efficient at resource processing. Their burrowing ancestry gives them unique ")
			TEXT("insights into asteroid mining and underground construction.")
		);

		AllRaces.Add(CreateRaceData(
			EPhylum::Annelida,
			TEXT("Segmentines"),
			TEXT("Annelida"),
			TEXT("Modular specialists excelling at resource processing"),
			Lore,
			Attrs,
			Tech,
			false
		));
	}

	// 7. Porifera (Spongians)
	{
		FRaceAttributes Attrs;
		Attrs.Resilience = 80.0f;
		Attrs.Efficiency = 70.0f;
		Attrs.Adaptability = 75.0f;
		Attrs.Intelligence = 40.0f;
		Attrs.Social = 70.0f;
		Attrs.SpaceAdaptation = 60.0f;
		Attrs.ResourceExtraction = 65.0f;
		Attrs.TechAffinity = 40.0f;

		FTechBonuses Tech;
		Tech.Biology = 3;
		Tech.Energy = 1;
		Tech.Sensors = 1;

		FText Lore = FText::FromString(
			TEXT("The Spongians evolved from filter-feeding organisms, developing a collective consciousness ")
			TEXT("spread across millions of individual cells. While individually simple, their colonies achieve ")
			TEXT("emergent intelligence through chemical signaling and collective processing. Spongian structures ")
			TEXT("are incredibly resilient, able to rebuild from even catastrophic damage. They excel at ")
			TEXT("large-scale resource filtration and environmental adaptation.")
		);

		AllRaces.Add(CreateRaceData(
			EPhylum::Porifera,
			TEXT("Spongians"),
			TEXT("Porifera"),
			TEXT("Resilient collective consciousness with emergent intelligence"),
			Lore,
			Attrs,
			Tech,
			false
		));
	}

	// 8. Platyhelminthes (Flatworlds)
	{
		FRaceAttributes Attrs;
		Attrs.Adaptability = 70.0f;
		Attrs.Resilience = 55.0f;
		Attrs.Intelligence = 55.0f;
		Attrs.Social = 50.0f;
		Attrs.Efficiency = 65.0f;
		Attrs.SpaceAdaptation = 60.0f;
		Attrs.ResourceExtraction = 55.0f;
		Attrs.TechAffinity = 60.0f;

		FTechBonuses Tech;
		Tech.Biology = 2;
		Tech.Computing = 1;
		Tech.Sensors = 2;

		FText Lore = FText::FromString(
			TEXT("The Flatworlds evolved from flatworms, developing thin, flexible body plans ideal for ")
			TEXT("maneuvering in tight spaces. Their bilateral symmetry and simple nervous systems evolved ")
			TEXT("into sophisticated distributed processing networks. Flatworld colonies excel at infiltration, ")
			TEXT("exploration, and sensory gathering, able to squeeze into areas other races cannot access.")
		);

		AllRaces.Add(CreateRaceData(
			EPhylum::Platyhelminthes,
			TEXT("Flatworlds"),
			TEXT("Platyhelminthes"),
			TEXT("Flexible explorers with distributed sensory networks"),
			Lore,
			Attrs,
			Tech,
			false
		));
	}

	// 9. Nematoda (Threadlings)
	{
		FRaceAttributes Attrs;
		Attrs.Efficiency = 80.0f;
		Attrs.Resilience = 75.0f;
		Attrs.Adaptability = 70.0f;
		Attrs.Intelligence = 45.0f;
		Attrs.Social = 55.0f;
		Attrs.SpaceAdaptation = 55.0f;
		Attrs.ResourceExtraction = 60.0f;
		Attrs.TechAffinity = 45.0f;

		FTechBonuses Tech;
		Tech.Biology = 3;
		Tech.Efficiency = 2;

		FText Lore = FText::FromString(
			TEXT("The Threadlings evolved from microscopic roundworms, achieving sentience through collective ")
			TEXT("swarm intelligence. Individually tiny but collectively powerful, Threadling colonies can number ")
			TEXT("in the billions. Their extreme efficiency and resilience come from evolutionary adaptation to ")
			TEXT("survive in nearly any environment. Threadling swarms can break down and process resources at ")
			TEXT("the molecular level.")
		);

		AllRaces.Add(CreateRaceData(
			EPhylum::Nematoda,
			TEXT("Threadlings"),
			TEXT("Nematoda"),
			TEXT("Efficient swarm intelligence with extreme resilience"),
			Lore,
			Attrs,
			Tech,
			false
		));
	}

	// 10. Fungi (Myconids)
	{
		FRaceAttributes Attrs;
		Attrs.Resilience = 70.0f;
		Attrs.Adaptability = 75.0f;
		Attrs.Efficiency = 75.0f;
		Attrs.ResourceExtraction = 70.0f;
		Attrs.Intelligence = 55.0f;
		Attrs.Social = 60.0f;
		Attrs.SpaceAdaptation = 50.0f;
		Attrs.TechAffinity = 50.0f;

		FTechBonuses Tech;
		Tech.Biology = 3;
		Tech.Engineering = 1;
		Tech.Energy = 1;

		FText Lore = FText::FromString(
			TEXT("The Myconids evolved from fungal networks, achieving consciousness through vast underground ")
			TEXT("mycorrhizal connections. Their distributed intelligence spans entire planetoids, communicating ")
			TEXT("through chemical and electrical signals. Myconid colonies excel at breaking down complex ")
			TEXT("materials and recycling resources with near-perfect efficiency. Their slow but inexorable ")
			TEXT("growth can transform entire asteroids into living networks.")
		);

		AllRaces.Add(CreateRaceData(
			EPhylum::Fungi,
			TEXT("Myconids"),
			TEXT("Fungi"),
			TEXT("Networked decomposers with perfect resource recycling"),
			Lore,
			Attrs,
			Tech,
			false
		));
	}

	// 11. Plantae (Chlorophytes)
	{
		FRaceAttributes Attrs;
		Attrs.Efficiency = 80.0f;
		Attrs.Resilience = 65.0f;
		Attrs.Adaptability = 70.0f;
		Attrs.Intelligence = 50.0f;
		Attrs.Social = 65.0f;
		Attrs.SpaceAdaptation = 55.0f;
		Attrs.ResourceExtraction = 55.0f;
		Attrs.TechAffinity = 50.0f;

		FTechBonuses Tech;
		Tech.Energy = 3;
		Tech.Biology = 2;
		Tech.Sensors = 1;

		FText Lore = FText::FromString(
			TEXT("The Chlorophytes evolved from photosynthetic organisms, developing the ability to harness ")
			TEXT("energy directly from stellar radiation. Their modular, branching growth patterns allow them ")
			TEXT("to maximize energy collection while their root networks process materials with incredible ")
			TEXT("efficiency. Chlorophyte colonies are patient and methodical, growing steadily and sustainably. ")
			TEXT("Their mastery of energy conversion makes them nearly self-sufficient.")
		);

		AllRaces.Add(CreateRaceData(
			EPhylum::Plantae,
			TEXT("Chlorophytes"),
			TEXT("Plantae"),
			TEXT("Photosynthetic energy masters with sustainable growth"),
			Lore,
			Attrs,
			Tech,
			false
		));
	}

	UE_LOG(LogTemp, Log, TEXT("RaceManager: Initialized %d races"), AllRaces.Num());
}

FRaceData URaceManager::CreateRaceData(
	EPhylum Phylum,
	const FString& RaceName,
	const FString& PhylumName,
	const FString& Description,
	const FText& Lore,
	const FRaceAttributes& Attributes,
	const FTechBonuses& TechBonuses,
	bool bUnlocked)
{
	FRaceData Data;
	Data.Phylum = Phylum;
	Data.RaceName = RaceName;
	Data.PhylumName = PhylumName;
	Data.Description = Description;
	Data.Lore = Lore;
	Data.Attributes = Attributes;
	Data.TechBonuses = TechBonuses;
	Data.bIsUnlocked = bUnlocked;

	// Icon path will be set later when assets are created
	// Format: /Game/UI/RaceIcons/Icon_{RaceName}
	FString IconPath = FString::Printf(TEXT("/Game/UI/RaceIcons/Icon_%s"), *RaceName);
	Data.RaceIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(IconPath));

	return Data;
}

TArray<FRaceData> URaceManager::GetUnlockedRaces() const
{
	return AllRaces.FilterByPredicate([](const FRaceData& Race)
	{
		return Race.bIsUnlocked;
	});
}

FRaceData URaceManager::GetRaceData(EPhylum Phylum) const
{
	const FRaceData* Found = AllRaces.FindByPredicate([Phylum](const FRaceData& Race)
	{
		return Race.Phylum == Phylum;
	});

	return Found ? *Found : FRaceData();
}

bool URaceManager::IsRaceUnlocked(EPhylum Phylum) const
{
	const FRaceData* Found = AllRaces.FindByPredicate([Phylum](const FRaceData& Race)
	{
		return Race.Phylum == Phylum;
	});

	return Found && Found->bIsUnlocked;
}

void URaceManager::SetPlayerRaceSelection(EPhylum Phylum, const FString& ColonyName)
{
	PlayerSelection.SelectedPhylum = Phylum;
	PlayerSelection.ColonyName = ColonyName;
	PlayerSelection.RaceData = GetRaceData(Phylum);
	bHasSelection = true;

	UE_LOG(LogTemp, Log, TEXT("RaceManager: Player selected %s, Colony: %s"),
		*PlayerSelection.RaceData.RaceName, *ColonyName);
}

void URaceManager::UnlockRace(EPhylum Phylum)
{
	for (FRaceData& Race : AllRaces)
	{
		if (Race.Phylum == Phylum)
		{
			Race.bIsUnlocked = true;
			UE_LOG(LogTemp, Log, TEXT("RaceManager: Unlocked race %s"), *Race.RaceName);
			SaveUnlockState();
			return;
		}
	}
}

void URaceManager::ResetUnlocks()
{
	// Reset all races to default unlock state
	for (FRaceData& Race : AllRaces)
	{
		// Keep the 4 default races unlocked
		bool bShouldBeUnlocked = (Race.Phylum == EPhylum::Chordata ||
		                         Race.Phylum == EPhylum::Arthropoda ||
		                         Race.Phylum == EPhylum::Cnidaria ||
		                         Race.Phylum == EPhylum::Mollusca);
		Race.bIsUnlocked = bShouldBeUnlocked;
	}

	SaveUnlockState();
}

void URaceManager::SaveUnlockState()
{
	// TODO: Implement save system integration
	// For now, just log
	UE_LOG(LogTemp, Log, TEXT("RaceManager: SaveUnlockState called (not yet implemented)"));
}

void URaceManager::LoadUnlockState()
{
	// TODO: Implement save system integration
	// For now, just log
	UE_LOG(LogTemp, Log, TEXT("RaceManager: LoadUnlockState called (not yet implemented)"));
}
