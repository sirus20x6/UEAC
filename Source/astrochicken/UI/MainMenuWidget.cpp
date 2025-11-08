// /thearray/git/UnrealEngine/astrochicken/Source/astrochicken/UI/MainMenuWidget.cpp

#include "MainMenuWidget.h"
#include "DynamicSettingsWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UE_LOG(LogTemp, Warning, TEXT("MainMenuWidget::NativeConstruct() called"));
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("MainMenuWidget::NativeConstruct() called"));
    }

    // Check widget binding status
    UE_LOG(LogTemp, Warning, TEXT("Widget Binding Status:"));
    UE_LOG(LogTemp, Warning, TEXT("  StartGameButton: %s"), StartGameButton ? TEXT("BOUND") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("  SettingsButton: %s"), SettingsButton ? TEXT("BOUND") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("  QuitButton: %s"), QuitButton ? TEXT("BOUND") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("  MenuSwitcher: %s"), MenuSwitcher ? TEXT("BOUND") : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("  SettingsContainer: %s"), SettingsContainer ? TEXT("BOUND") : TEXT("NULL"));

    // Ensure the buttons are valid before binding events
    if (StartGameButton)
    {
        StartGameButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnStartGameClicked);
        UE_LOG(LogTemp, Warning, TEXT("StartGameButton event bound"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("StartGameButton is NULL - check Blueprint widget binding!"));
    }

    if (SettingsButton)
    {
        SettingsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSettingsClicked);
        UE_LOG(LogTemp, Warning, TEXT("SettingsButton event bound"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsButton is NULL - check Blueprint widget binding!"));
    }

    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
        UE_LOG(LogTemp, Warning, TEXT("QuitButton event bound"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("QuitButton is NULL - check Blueprint widget binding!"));
    }
    
    // Check MenuSwitcher setup
    if (MenuSwitcher)
    {
        int32 ChildCount = MenuSwitcher->GetChildrenCount();
        UE_LOG(LogTemp, Warning, TEXT("MenuSwitcher found with %d children"), ChildCount);
        
        for (int32 i = 0; i < ChildCount; i++)
        {
            UWidget* Child = MenuSwitcher->GetChildAt(i);
            UE_LOG(LogTemp, Warning, TEXT("  Child %d: %s"), i, Child ? *Child->GetClass()->GetName() : TEXT("NULL"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MenuSwitcher is NULL - check Blueprint widget binding!"));
    }
    
    // Create the DynamicSettingsWidget for managing settings
    if (SettingsContainer)
    {
        SettingsWidgetRef = CreateWidget<UDynamicSettingsWidget>(this, UDynamicSettingsWidget::StaticClass());
        if (SettingsWidgetRef)
        {
            SettingsWidgetRef->OnBackToMainMenu.AddDynamic(this, &UMainMenuWidget::OnBackToMainMenu);
            UE_LOG(LogTemp, Warning, TEXT("DynamicSettingsWidget created and back navigation bound"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create DynamicSettingsWidget!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsContainer is NULL - check Blueprint widget binding!"));
    }
    
    // Start with main menu visible
    ShowMainMenu();
    
    UE_LOG(LogTemp, Warning, TEXT("MainMenuWidget::NativeConstruct() completed"));
}

void UMainMenuWidget::OnStartGameClicked()
{
    // A simple log to confirm it's working
    UE_LOG(LogTemp, Warning, TEXT("Start Game Button Clicked!"));

    // Get the world and open the main game level
    // Replace "GameLevel" with the actual name of your first playable map
    UGameplayStatics::OpenLevel(this, FName("GameLevel")); 
}

void UMainMenuWidget::OnSettingsClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Settings Button Clicked!"));
    
    // Add on-screen debug message
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Settings Button Clicked!"));
    }
    
    if (!MenuSwitcher)
    {
        UE_LOG(LogTemp, Error, TEXT("MenuSwitcher is null!"));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERROR: MenuSwitcher is null!"));
        }
        return;
    }
    
    if (!SettingsContainer)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsContainer is null!"));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERROR: SettingsContainer is null!"));
        }
        return;
    }
    
    if (!SettingsWidgetRef)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsWidgetRef is null!"));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERROR: SettingsWidgetRef is null!"));
        }
        return;
    }
    
    // Use the new approach: let DynamicSettingsWidget populate the container directly
    UE_LOG(LogTemp, Warning, TEXT("Calling PopulateContainer on DynamicSettingsWidget"));
    SettingsWidgetRef->PopulateContainer(SettingsContainer);
    
    UE_LOG(LogTemp, Warning, TEXT("SettingsContainer populated with dynamic content"));
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Settings populated using new approach!"));
    }
    
    ShowSettingsMenu();
}

void UMainMenuWidget::OnQuitClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Quit Button Clicked!"));
    
    // Get the player controller to execute the quit command
    APlayerController* PlayerController = GetOwningPlayer();
    if (PlayerController)
    {
        UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, true);
    }
}

void UMainMenuWidget::OnBackToMainMenu()
{
    UE_LOG(LogTemp, Warning, TEXT("Returning to Main Menu from Settings"));
    ShowMainMenu();
}

void UMainMenuWidget::ShowMainMenu()
{
    if (MenuSwitcher)
    {
        MenuSwitcher->SetActiveWidgetIndex(0); // Assuming main menu is at index 0
        UE_LOG(LogTemp, Warning, TEXT("Switched to Main Menu"));
    }
}

void UMainMenuWidget::ShowSettingsMenu()
{
    if (MenuSwitcher)
    {
        int32 ChildCount = MenuSwitcher->GetChildrenCount();
        UE_LOG(LogTemp, Warning, TEXT("MenuSwitcher has %d children"), ChildCount);
        
        // Log each child widget
        for (int32 i = 0; i < ChildCount; i++)
        {
            UWidget* Child = MenuSwitcher->GetChildAt(i);
            UE_LOG(LogTemp, Warning, TEXT("Child %d: %s (Visible: %d)"), i, Child ? *Child->GetClass()->GetName() : TEXT("NULL"), Child ? (int32)Child->GetVisibility() : -1);
            
            // If this is the settings container, check its children too
            if (i == 1 && Child)
            {
                if (UVerticalBox* VBox = Cast<UVerticalBox>(Child))
                {
                    int32 VBoxChildren = VBox->GetChildrenCount();
                    UE_LOG(LogTemp, Warning, TEXT("  SettingsContainer has %d children:"), VBoxChildren);
                    for (int32 j = 0; j < VBoxChildren; j++)
                    {
                        UWidget* VBoxChild = VBox->GetChildAt(j);
                        UE_LOG(LogTemp, Warning, TEXT("    VBox Child %d: %s (Visible: %d)"), j, VBoxChild ? *VBoxChild->GetClass()->GetName() : TEXT("NULL"), VBoxChild ? (int32)VBoxChild->GetVisibility() : -1);
                    }
                }
            }
        }
        
        int32 CurrentIndex = MenuSwitcher->GetActiveWidgetIndex();
        UE_LOG(LogTemp, Warning, TEXT("Current active widget index: %d"), CurrentIndex);
        
        MenuSwitcher->SetActiveWidgetIndex(1);
        
        int32 NewIndex = MenuSwitcher->GetActiveWidgetIndex();
        UE_LOG(LogTemp, Warning, TEXT("New active widget index: %d"), NewIndex);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Purple, FString::Printf(TEXT("Switched from index %d to %d"), CurrentIndex, NewIndex));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MenuSwitcher is null in ShowSettingsMenu!"));
    }
}

// Remove the CreateDirectTestWidgets and OnDirectButtonClicked methods since we don't need them anymore
