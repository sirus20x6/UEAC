// Source/astrochicken/Race/RaceButtonWidget.cpp

#include "RaceButtonWidget.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void URaceButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button click
	if (RaceButton)
	{
		RaceButton->OnClicked.AddDynamic(this, &URaceButtonWidget::OnButtonClicked);
	}
}

void URaceButtonWidget::SetRaceData(const FRaceData& InRaceData)
{
	RaceData = InRaceData;

	// Update race name
	if (RaceNameText)
	{
		RaceNameText->SetText(FText::FromString(RaceData.RaceName));
	}

	// Update phylum text
	if (RacePhylumText)
	{
		RacePhylumText->SetText(FText::FromString(RaceData.PhylumName));
	}

	// Update icon
	if (RaceImage && !RaceData.RaceIcon.IsNull())
	{
		// Try to load the icon texture
		if (UTexture2D* IconTexture = RaceData.RaceIcon.LoadSynchronous())
		{
			RaceImage->SetBrushFromTexture(IconTexture);
		}
		else
		{
			// Icon not found - this is expected until assets are created
			UE_LOG(LogTemp, Verbose, TEXT("RaceButtonWidget: Icon not found for %s at %s"),
				*RaceData.RaceName, *RaceData.RaceIcon.ToString());
		}
	}

	// Set locked/unlocked state
	if (RaceButton)
	{
		RaceButton->SetIsEnabled(RaceData.bIsUnlocked);
	}

	// Visual indication of unlock state
	if (RaceBorder)
	{
		// Dim the border if locked
		FLinearColor BorderColor = RaceData.bIsUnlocked
			? FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)
			: FLinearColor(0.5f, 0.5f, 0.5f, 0.5f);
		RaceBorder->SetBrushColor(BorderColor);
	}
}

void URaceButtonWidget::SetSelected(bool bSelected)
{
	bIsSelected = bSelected;

	if (RaceBorder)
	{
		// Highlight border when selected
		FLinearColor BorderColor = bIsSelected
			? FLinearColor(0.1f, 0.4f, 0.8f, 1.0f) // Blue highlight
			: FLinearColor(0.2f, 0.2f, 0.2f, 1.0f); // Dark gray

		// Dim if locked
		if (!RaceData.bIsUnlocked)
		{
			BorderColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.5f);
		}

		RaceBorder->SetBrushColor(BorderColor);
	}
}

void URaceButtonWidget::OnButtonClicked()
{
	if (RaceData.bIsUnlocked)
	{
		UE_LOG(LogTemp, Log, TEXT("RaceButtonWidget: Clicked %s"), *RaceData.RaceName);
		OnRaceClicked.Broadcast(RaceData.Phylum);
	}
}
