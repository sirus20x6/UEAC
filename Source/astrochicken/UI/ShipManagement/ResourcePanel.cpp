// Source/astrochicken/UI/ShipManagement/ResourcePanel.cpp

#include "ResourcePanel.h"
#include "../../Infrastructure/ShipInfrastructureManager.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UResourcePanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		InfraManager = GameInstance->GetSubsystem<UShipInfrastructureManager>();
	}

	RefreshDisplay();
}

void UResourcePanel::RefreshDisplay()
{
	if (!InfraManager)
	{
		return;
	}

	FResourceNetwork Resources = InfraManager->GetResourceNetwork();

	// Life Support
	if (LifeSupportStatusText)
	{
		FString Status = Resources.LifeSupport.bIsOperational ? TEXT("Operational") : TEXT("OFFLINE");
		FLinearColor Color = Resources.LifeSupport.bIsOperational ? FLinearColor::Green : FLinearColor::Red;
		LifeSupportStatusText->SetText(FText::FromString(Status));
		LifeSupportStatusText->SetColorAndOpacity(Color);
	}

	if (OxygenProductionBar)
	{
		float Efficiency = Resources.LifeSupport.GetEfficiency();
		OxygenProductionBar->SetPercent(Efficiency);
	}

	if (CrewSupportText)
	{
		CrewSupportText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d Crew"),
			Resources.LifeSupport.CurrentCrew, Resources.LifeSupport.MaxCrewCapacity)));
	}

	// Coolant System
	if (CoolantStatusText)
	{
		FString Status = Resources.Coolant.bIsOperational ? TEXT("Operational") : TEXT("OFFLINE");
		FLinearColor Color = Resources.Coolant.bIsOperational ? FLinearColor::Green : FLinearColor::Red;

		if (Resources.Coolant.IsOverloaded())
		{
			Status = TEXT("OVERLOADED");
			Color = FLinearColor::Red;
		}

		CoolantStatusText->SetText(FText::FromString(Status));
		CoolantStatusText->SetColorAndOpacity(Color);
	}

	if (CoolantLevelBar && CoolantLevelText)
	{
		float Percent = Resources.Coolant.GetCoolantLevelPercent();
		CoolantLevelBar->SetPercent(Percent / 100.0f);
		CoolantLevelText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), Percent)));
	}

	if (HeatLoadBar && HeatLoadText)
	{
		float LoadPercent = (Resources.Coolant.HeatDissipationRate > 0.0f) ?
			FMath::Clamp(Resources.Coolant.CurrentHeatLoad / Resources.Coolant.HeatDissipationRate, 0.0f, 1.0f) : 0.0f;

		HeatLoadBar->SetPercent(LoadPercent);
		HeatLoadText->SetText(FText::FromString(FString::Printf(TEXT("%.1f / %.1f kW"),
			Resources.Coolant.CurrentHeatLoad, Resources.Coolant.HeatDissipationRate)));

		FLinearColor Color = LoadPercent > 0.9f ? FLinearColor::Red :
		                     LoadPercent > 0.75f ? FLinearColor::Yellow :
		                     FLinearColor::Green;
		HeatLoadBar->SetFillColorAndOpacity(Color);
	}

	// Leaking Pipes
	if (LeakingPipesText)
	{
		LeakingPipesText->SetText(FText::FromString(FString::Printf(TEXT("%d Leaking Pipes"), Resources.LeakingPipes)));

		FLinearColor Color = Resources.LeakingPipes > 0 ? FLinearColor::Red : FLinearColor::Green;
		LeakingPipesText->SetColorAndOpacity(Color);
	}
}
