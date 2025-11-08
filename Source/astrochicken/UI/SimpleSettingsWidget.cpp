// /thearray/git/UnrealEngine/astrochicken/Source/astrochicken/UI/SimpleSettingsWidget.cpp

#include "SimpleSettingsWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/Engine.h"

void USimpleSettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UE_LOG(LogTemp, Warning, TEXT("SimpleSettingsWidget::NativeConstruct() called"));
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("SimpleSettingsWidget constructed!"));
    }

    // Create a simple UI programmatically
    UVerticalBox* MainContainer = NewObject<UVerticalBox>(this);
    
    // Add title
    UTextBlock* TitleText = NewObject<UTextBlock>(this);
    TitleText->SetText(FText::FromString(TEXT("Settings Menu")));
    MainContainer->AddChild(TitleText);
    
    // Add some placeholder text
    UTextBlock* PlaceholderText = NewObject<UTextBlock>(this);
    PlaceholderText->SetText(FText::FromString(TEXT("This is a simple settings menu.\nMore settings will be added here.")));
    MainContainer->AddChild(PlaceholderText);
    
    // Add back button
    UButton* BackButton = NewObject<UButton>(this);
    BackButton->OnClicked.AddDynamic(this, &USimpleSettingsWidget::OnBackButtonClicked);
    
    UTextBlock* BackButtonText = NewObject<UTextBlock>(this);
    BackButtonText->SetText(FText::FromString(TEXT("Back to Main Menu")));
    BackButton->AddChild(BackButtonText);
    
    MainContainer->AddChild(BackButton);
    
    // Create canvas and set as root
    UCanvasPanel* RootCanvas = NewObject<UCanvasPanel>(this);
    UCanvasPanelSlot* Slot = RootCanvas->AddChildToCanvas(MainContainer);
    Slot->SetAnchors(FAnchors(0, 0, 1, 1));
    Slot->SetOffsets(FMargin(50, 50, 50, 50));
    
    WidgetTree->RootWidget = RootCanvas;
    
    UE_LOG(LogTemp, Warning, TEXT("SimpleSettingsWidget UI created successfully"));
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, TEXT("Simple settings UI ready!"));
    }
}

void USimpleSettingsWidget::OnBackButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Back button clicked in SimpleSettingsWidget"));
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Going back to main menu..."));
    }
    OnBackToMainMenu.Broadcast();
}
