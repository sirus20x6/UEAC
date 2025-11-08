// Source/astrochicken/Race/RaceSelectionWidget.cpp

#include "RaceSelectionWidget.h"
#include "RaceManager.h"
#include "RaceButtonWidget.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/EditableText.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void URaceSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get the RaceManager subsystem
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		RaceManager = GameInstance->GetSubsystem<URaceManager>();
	}

	// Bind button events
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &URaceSelectionWidget::OnConfirmButtonClicked);
	}

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &URaceSelectionWidget::OnBackButtonClicked);
	}

	// Set default colony name
	if (ColonyNameInput)
	{
		ColonyNameInput->SetText(FText::FromString(TEXT("New Colony")));
	}

	// Populate the race list
	PopulateRaceList();

	// Initial state
	UpdateConfirmButtonState();
}

void URaceSelectionWidget::PopulateRaceList()
{
	if (!RaceManager || !RaceListScrollBox || !RaceButtonWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("RaceSelectionWidget: Missing required components for PopulateRaceList"));
		return;
	}

	// Clear existing buttons
	RaceListScrollBox->ClearChildren();
	RaceButtons.Empty();

	// Get unlocked races
	TArray<FRaceData> UnlockedRaces = RaceManager->GetUnlockedRaces();

	UE_LOG(LogTemp, Log, TEXT("RaceSelectionWidget: Populating %d unlocked races"), UnlockedRaces.Num());

	// Create a button for each unlocked race
	for (const FRaceData& Race : UnlockedRaces)
	{
		URaceButtonWidget* RaceButton = CreateWidget<URaceButtonWidget>(this, RaceButtonWidgetClass);
		if (RaceButton)
		{
			RaceButton->SetRaceData(Race);

			// Bind click event
			RaceButton->OnRaceClicked.AddDynamic(this, &URaceSelectionWidget::OnRaceButtonClicked);

			RaceListScrollBox->AddChild(RaceButton);
			RaceButtons.Add(RaceButton);
		}
	}

	// Auto-select first race
	if (UnlockedRaces.Num() > 0)
	{
		DisplayRaceDetails(UnlockedRaces[0]);
		CurrentlySelectedPhylum = UnlockedRaces[0].Phylum;
		bHasSelectedRace = true;
	}
}

void URaceSelectionWidget::OnRaceButtonClicked(EPhylum Phylum)
{
	if (!RaceManager)
	{
		return;
	}

	CurrentlySelectedPhylum = Phylum;
	bHasSelectedRace = true;

	FRaceData RaceData = RaceManager->GetRaceData(Phylum);
	DisplayRaceDetails(RaceData);
	UpdateConfirmButtonState();

	// Update visual selection state for all buttons
	for (URaceButtonWidget* Button : RaceButtons)
	{
		if (Button)
		{
			Button->SetSelected(Button->GetPhylum() == Phylum);
		}
	}
}

void URaceSelectionWidget::DisplayRaceDetails(const FRaceData& RaceData)
{
	// Update race info
	if (RaceNameText)
	{
		FString DisplayName = FString::Printf(TEXT("%s (%s)"), *RaceData.RaceName, *RaceData.PhylumName);
		RaceNameText->SetText(FText::FromString(DisplayName));
	}

	if (RaceDescriptionText)
	{
		RaceDescriptionText->SetText(FText::FromString(RaceData.Description));
	}

	if (RaceLoreText)
	{
		RaceLoreText->SetText(RaceData.Lore);
	}

	// Update icon
	if (RaceIconImage && !RaceData.RaceIcon.IsNull())
	{
		if (UTexture2D* IconTexture = RaceData.RaceIcon.LoadSynchronous())
		{
			RaceIconImage->SetBrushFromTexture(IconTexture);
		}
	}

	// Update attributes
	if (AdaptabilityText)
		AdaptabilityText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), RaceData.Attributes.Adaptability)));
	if (ResilienceText)
		ResilienceText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), RaceData.Attributes.Resilience)));
	if (IntelligenceText)
		IntelligenceText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), RaceData.Attributes.Intelligence)));
	if (SocialText)
		SocialText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), RaceData.Attributes.Social)));
	if (EfficiencyText)
		EfficiencyText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), RaceData.Attributes.Efficiency)));
	if (SpaceAdaptationText)
		SpaceAdaptationText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), RaceData.Attributes.SpaceAdaptation)));
	if (ResourceExtractionText)
		ResourceExtractionText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), RaceData.Attributes.ResourceExtraction)));
	if (TechAffinityText)
		TechAffinityText->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), RaceData.Attributes.TechAffinity)));

	// Update tech bonuses
	if (TechBonusesText)
	{
		TArray<FString> Bonuses;
		if (RaceData.TechBonuses.Physics > 0)
			Bonuses.Add(FString::Printf(TEXT("Physics +%d"), RaceData.TechBonuses.Physics));
		if (RaceData.TechBonuses.Engineering > 0)
			Bonuses.Add(FString::Printf(TEXT("Engineering +%d"), RaceData.TechBonuses.Engineering));
		if (RaceData.TechBonuses.Energy > 0)
			Bonuses.Add(FString::Printf(TEXT("Energy +%d"), RaceData.TechBonuses.Energy));
		if (RaceData.TechBonuses.Computing > 0)
			Bonuses.Add(FString::Printf(TEXT("Computing +%d"), RaceData.TechBonuses.Computing));
		if (RaceData.TechBonuses.Biology > 0)
			Bonuses.Add(FString::Printf(TEXT("Biology +%d"), RaceData.TechBonuses.Biology));
		if (RaceData.TechBonuses.Sensors > 0)
			Bonuses.Add(FString::Printf(TEXT("Sensors +%d"), RaceData.TechBonuses.Sensors));

		FString BonusString = FString::Join(Bonuses, TEXT("\n"));
		TechBonusesText->SetText(FText::FromString(BonusString));
	}
}

void URaceSelectionWidget::UpdateConfirmButtonState()
{
	if (ConfirmButton && ColonyNameInput)
	{
		FString ColonyName = ColonyNameInput->GetText().ToString().TrimStartAndEnd();
		bool bCanConfirm = bHasSelectedRace && !ColonyName.IsEmpty();
		ConfirmButton->SetIsEnabled(bCanConfirm);
	}
}

void URaceSelectionWidget::OnConfirmButtonClicked()
{
	if (!RaceManager || !bHasSelectedRace || !ColonyNameInput)
	{
		return;
	}

	FString ColonyName = ColonyNameInput->GetText().ToString().TrimStartAndEnd();

	if (ColonyName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("RaceSelectionWidget: Colony name is empty"));
		return;
	}

	// Set the player's race selection
	RaceManager->SetPlayerRaceSelection(CurrentlySelectedPhylum, ColonyName);

	// Broadcast confirmation event
	OnRaceConfirmed.Broadcast(CurrentlySelectedPhylum, ColonyName);

	UE_LOG(LogTemp, Log, TEXT("RaceSelectionWidget: Confirmed selection - Phylum: %d, Colony: %s"),
		static_cast<int32>(CurrentlySelectedPhylum), *ColonyName);
}

void URaceSelectionWidget::OnBackButtonClicked()
{
	// Remove this widget from viewport
	RemoveFromParent();
}
