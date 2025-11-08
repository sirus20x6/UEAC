// /thearray/git/UnrealEngine/astrochicken/Source/astrochicken/UI/DynamicSettingsWidget.cpp

#include "DynamicSettingsWidget.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/ScrollBox.h"
#include "Components/Border.h"
#include "Components/Spacer.h"
#include "Components/SizeBox.h"
#include "Components/CanvasPanel.h"
#include "Components/Overlay.h"
#include "Components/SlateWrapperTypes.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBoxSlot.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Styling/SlateColor.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"

void UDynamicSettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UE_LOG(LogTemp, Warning, TEXT("DynamicSettingsWidget::NativeConstruct() called"));
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("DynamicSettingsWidget::NativeConstruct() called"));
    }

    // Get the game user settings with multiple fallback methods
    GameUserSettings = UGameUserSettings::GetGameUserSettings();
    
    if (!GameUserSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get GameUserSettings from GetGameUserSettings()!"));
        
        // Try alternative method
        if (GEngine)
        {
            GameUserSettings = GEngine->GetGameUserSettings();
            if (GameUserSettings)
            {
                UE_LOG(LogTemp, Warning, TEXT("Successfully got GameUserSettings from GEngine!"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to get GameUserSettings from GEngine as well!"));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Successfully got GameUserSettings in NativeConstruct"));
    }
    
    // Calculate layout configuration based on current screen resolution
    CalculateLayoutConfig();

    // Load settings layout from JSON - but don't generate UI yet
    // UI will be generated when PopulateContainer is called
    bool JsonLoaded = LoadSettingsLayout();
    UE_LOG(LogTemp, Warning, TEXT("JSON loading result: %s"), JsonLoaded ? TEXT("SUCCESS") : TEXT("FAILED"));
    UE_LOG(LogTemp, Warning, TEXT("Parsed sections count: %d"), SettingsLayout.Sections.Num());
    
    if (!JsonLoaded)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load settings layout!"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Settings layout loaded successfully - %d sections found"), SettingsLayout.Sections.Num());
    }
}

void UDynamicSettingsWidget::PopulateContainer(UVerticalBox* Container)
{
    UE_LOG(LogTemp, Warning, TEXT("PopulateContainer called"));
    if (!Container)
    {
        UE_LOG(LogTemp, Error, TEXT("Container is null!"));
        return;
    }

    TargetContainer = Container;

    // Clear existing content
    TargetContainer->ClearChildren();
    
    // Ensure GameUserSettings is available
    if (!GameUserSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameUserSettings is null in PopulateContainer, attempting to get it..."));
        GameUserSettings = UGameUserSettings::GetGameUserSettings();
        
        if (!GameUserSettings && GEngine)
        {
            GameUserSettings = GEngine->GetGameUserSettings();
        }
        
        if (GameUserSettings)
        {
            UE_LOG(LogTemp, Warning, TEXT("Successfully obtained GameUserSettings in PopulateContainer"));
            GameUserSettings->LoadSettings();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get GameUserSettings in PopulateContainer!"));
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERROR: Cannot get GameUserSettings!"));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("GameUserSettings is valid in PopulateContainer"));
    }
    
    // Recalculate layout for current resolution
    CalculateLayoutConfig();

    // Load JSON here since NativeConstruct might not be called
    UE_LOG(LogTemp, Warning, TEXT("Loading JSON from PopulateContainer..."));
    bool JsonLoaded = LoadSettingsLayout();
    UE_LOG(LogTemp, Warning, TEXT("JSON loading result: %s"), JsonLoaded ? TEXT("SUCCESS") : TEXT("FAILED"));
    UE_LOG(LogTemp, Warning, TEXT("Parsed sections count: %d"), SettingsLayout.Sections.Num());

    if (JsonLoaded && SettingsLayout.Sections.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Generating full settings UI with %d sections"), SettingsLayout.Sections.Num());
        GenerateSettingsUI();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No settings layout loaded, creating fallback UI"));
        CreateFallbackUI();
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Settings UI populated in container!"));
    }
}

bool UDynamicSettingsWidget::LoadSettingsLayout()
{
    // Try to load from Content/Data/SettingsLayout.json
    FString JsonFilePath = FPaths::ProjectContentDir() + TEXT("Data/SettingsLayout.json");
    FString JsonString;
    
    UE_LOG(LogTemp, Warning, TEXT("Attempting to load JSON from: %s"), *JsonFilePath);
    
    if (FFileHelper::LoadFileToString(JsonString, *JsonFilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("JSON file loaded successfully, size: %d characters"), JsonString.Len());
        ParseJsonToLayout(JsonString);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Could not load settings layout file: %s"), *JsonFilePath);
        return false;
    }
}

void UDynamicSettingsWidget::ParseJsonToLayout(const FString& JsonString)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        if (const TSharedPtr<FJsonObject>* LayoutObject = nullptr; JsonObject->TryGetObjectField(TEXT("SettingsLayout"), LayoutObject))
        {
            const TSharedPtr<FJsonObject>& Layout = *LayoutObject;
            
            // Parse title
            Layout->TryGetStringField(TEXT("Title"), SettingsLayout.Title);
            
            // Parse sections
            const TArray<TSharedPtr<FJsonValue>>* SectionsArray;
            if (Layout->TryGetArrayField(TEXT("Sections"), SectionsArray))
            {
                for (const auto& SectionValue : *SectionsArray)
                {
                    const TSharedPtr<FJsonObject>& SectionObj = SectionValue->AsObject();
                    FSettingsSection Section;
                    
                    SectionObj->TryGetStringField(TEXT("Name"), Section.Name);
                    SectionObj->TryGetStringField(TEXT("Type"), Section.Type);
                    
                    // Parse controls
                    const TArray<TSharedPtr<FJsonValue>>* ControlsArray;
                    if (SectionObj->TryGetArrayField(TEXT("Controls"), ControlsArray))
                    {
                        for (const auto& ControlValue : *ControlsArray)
                        {
                            const TSharedPtr<FJsonObject>& ControlObj = ControlValue->AsObject();
                            FSettingsControl Control;
                            
                            ControlObj->TryGetStringField(TEXT("Type"), Control.Type);
                            ControlObj->TryGetStringField(TEXT("Name"), Control.Name);
                            ControlObj->TryGetStringField(TEXT("Label"), Control.Label);
                            ControlObj->TryGetNumberField(TEXT("Min"), Control.Min);
                            ControlObj->TryGetNumberField(TEXT("Max"), Control.Max);
                            ControlObj->TryGetNumberField(TEXT("Step"), Control.Step);
                            ControlObj->TryGetBoolField(TEXT("ShowValue"), Control.ShowValue);
                            ControlObj->TryGetNumberField(TEXT("ValueMultiplier"), Control.ValueMultiplier);
                            ControlObj->TryGetStringField(TEXT("ValueSuffix"), Control.ValueSuffix);
                            ControlObj->TryGetStringField(TEXT("ZeroText"), Control.ZeroText);
                            ControlObj->TryGetStringField(TEXT("Style"), Control.Style);
                            
                            // Parse options array
                            const TArray<TSharedPtr<FJsonValue>>* OptionsArray;
                            if (ControlObj->TryGetArrayField(TEXT("Options"), OptionsArray))
                            {
                                for (const auto& OptionValue : *OptionsArray)
                                {
                                    Control.Options.Add(OptionValue->AsString());
                                }
                            }
                            
                            Section.Controls.Add(Control);
                        }
                    }
                    
                    SettingsLayout.Sections.Add(Section);
                }
            }
            
            // Parse action buttons
            const TArray<TSharedPtr<FJsonValue>>* ButtonsArray;
            if (Layout->TryGetArrayField(TEXT("ActionButtons"), ButtonsArray))
            {
                for (const auto& ButtonValue : *ButtonsArray)
                {
                    const TSharedPtr<FJsonObject>& ButtonObj = ButtonValue->AsObject();
                    FSettingsControl Button;
                    
                    ButtonObj->TryGetStringField(TEXT("Type"), Button.Type);
                    ButtonObj->TryGetStringField(TEXT("Name"), Button.Name);
                    ButtonObj->TryGetStringField(TEXT("Label"), Button.Label);
                    ButtonObj->TryGetStringField(TEXT("Style"), Button.Style);
                    
                    SettingsLayout.ActionButtons.Add(Button);
                }
            }
        }
    }
}

void UDynamicSettingsWidget::AutoToggleWindowModeForMouseFix()
{
    if (!GameUserSettings || bAutoToggleInProgress)
    {
        return;
    }
    
    // Only auto-toggle if we're in windowed mode (where the issue occurs)
    if (GameUserSettings->GetFullscreenMode() != EWindowMode::Windowed)
    {
        return;
    }
    
    bAutoToggleInProgress = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Starting automatic window mode toggle to fix mouse offset..."));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("Auto-fixing mouse offset..."));
    }
    
    // Step 1: Switch to fullscreen briefly
    GameUserSettings->SetFullscreenMode(EWindowMode::Fullscreen);
    GameUserSettings->ApplyResolutionSettings(false);
    
    // Step 2: After a short delay, switch back to windowed
    FTimerHandle ToggleBackTimer;
    GetWorld()->GetTimerManager().SetTimer(ToggleBackTimer, [this]()
    {
        if (GameUserSettings)
        {
            // Switch back to windowed mode
            GameUserSettings->SetFullscreenMode(EWindowMode::Windowed);
            GameUserSettings->ApplyResolutionSettings(false);
            
            UE_LOG(LogTemp, Warning, TEXT("Window mode toggle completed - mouse should be fixed"));
            
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Mouse offset auto-fix complete!"));
            }
            
            // Reset the flag after another short delay
            FTimerHandle ResetFlagTimer;
            GetWorld()->GetTimerManager().SetTimer(ResetFlagTimer, [this]()
            {
                bAutoToggleInProgress = false;
            }, 1.0f, false);
        }
    }, 0.5f, false); // Half second delay
}

void UDynamicSettingsWidget::RecalibrateMouseInput()
{
    UE_LOG(LogTemp, Warning, TEXT("Recalibrating mouse input after resolution change..."));
    
    // Get the player controller to recalibrate mouse input
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            // Force the viewport to recalculate mouse coordinates
            if (GEngine && GEngine->GameViewport)
            {
                // Get the current viewport size to ensure mouse scaling is correct
                FVector2D ViewportSize;
                GEngine->GameViewport->GetViewportSize(ViewportSize);
                
                // Get window mode to determine if we need special handling
                bool bIsWindowed = false;
                if (GameUserSettings)
                {
                    EWindowMode::Type WindowMode = GameUserSettings->GetFullscreenMode();
                    bIsWindowed = (WindowMode == EWindowMode::Windowed);
                }
                
                UE_LOG(LogTemp, Warning, TEXT("New viewport size for mouse calibration: %.0fx%.0f (Windowed: %s)"), 
                       ViewportSize.X, ViewportSize.Y, bIsWindowed ? TEXT("YES") : TEXT("NO"));
                
                // Force input system to recalculate mouse coordinates
                PC->FlushPressedKeys();
                
                // Clear any cached mouse state
                if (FSlateApplication::IsInitialized())
                {
                    FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::Cleared);
                    FSlateApplication::Get().ResetToDefaultInputSettings();
                }
                
                // Reset the input mode to force mouse coordinate recalculation
                FInputModeUIOnly InputModeData;
                InputModeData.SetWidgetToFocus(this->TakeWidget());
                InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                
                PC->SetInputMode(InputModeData);
                PC->bShowMouseCursor = true;
                
                // Force engine to recalculate mouse scaling
                if (GEngine->GameViewport->Viewport)
                {
                    GEngine->GameViewport->Viewport->InvalidateDisplay();
                }
                
                // Set a timer for delayed mouse recalibration as a backup
                FTimerHandle MouseRecalibrationTimer;
                GetWorld()->GetTimerManager().SetTimer(MouseRecalibrationTimer, [this, PC, bIsWindowed]()
                {
                    if (PC && GEngine && GEngine->GameViewport)
                    {
                        // Secondary recalibration after a small delay
                        FInputModeUIOnly DelayedInputMode;
                        DelayedInputMode.SetWidgetToFocus(this->TakeWidget());
                        DelayedInputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                        PC->SetInputMode(DelayedInputMode);
                        PC->bShowMouseCursor = true;
                        
                        // Additional mouse state clearing
                        PC->FlushPressedKeys();
                        
                        // Force viewport refresh
                        if (GEngine->GameViewport->Viewport)
                        {
                            GEngine->GameViewport->Viewport->InvalidateDisplay();
                        }
                        
                        UE_LOG(LogTemp, Warning, TEXT("Delayed mouse recalibration completed (Windowed: %s)"), 
                               bIsWindowed ? TEXT("YES") : TEXT("NO"));
                    }
                }, 0.1f, false); // 100ms delay
                
                // Additional longer delay for windowed mode title bar compensation
                if (bIsWindowed)
                {
                    FTimerHandle ExtendedRecalibrationTimer;
                    GetWorld()->GetTimerManager().SetTimer(ExtendedRecalibrationTimer, [this, PC]()
                    {
                        if (PC && GEngine && GEngine->GameViewport)
                        {
                            // Force complete mouse input refresh for windowed mode
                            PC->FlushPressedKeys();
                            
                            // Clear slate application mouse state
                            if (FSlateApplication::IsInitialized())
                            {
                                FSlateApplication::Get().ResetToDefaultInputSettings();
                            }
                            
                            // Reset input mode one more time
                            FInputModeUIOnly FinalInputMode;
                            FinalInputMode.SetWidgetToFocus(this->TakeWidget());
                            FinalInputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                            PC->SetInputMode(FinalInputMode);
                            PC->bShowMouseCursor = true;
                            
                            // Force viewport refresh
                            if (GEngine->GameViewport->Viewport)
                            {
                                GEngine->GameViewport->Viewport->InvalidateDisplay();
                            }
                            
                            UE_LOG(LogTemp, Warning, TEXT("Extended windowed mode recalibration completed"));
                        }
                    }, 0.3f, false); // 300ms delay for windowed mode
                }
                
                UE_LOG(LogTemp, Warning, TEXT("Mouse input recalibrated successfully"));
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, 
                                                      FString::Printf(TEXT("Mouse recalibrated (%s)"), 
                                                                    bIsWindowed ? TEXT("Windowed") : TEXT("Fullscreen")));
                }
            }
        }
    }
}

void UDynamicSettingsWidget::ApplyGraphicsQualitySettings()
{
    if (!GameUserSettings) return;
    
    // Apply individual quality sliders
    if (WidgetMap.Contains(TEXT("ViewDistanceSlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("ViewDistanceSlider")]);
        if (Slider)
        {
            int32 Quality = FMath::RoundToInt(Slider->GetValue());
            GameUserSettings->SetViewDistanceQuality(Quality);
            UE_LOG(LogTemp, Warning, TEXT("Applied View Distance Quality: %d"), Quality);
        }
    }
    
    if (WidgetMap.Contains(TEXT("ShadowQualitySlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("ShadowQualitySlider")]);
        if (Slider)
        {
            int32 Quality = FMath::RoundToInt(Slider->GetValue());
            GameUserSettings->SetShadowQuality(Quality);
            UE_LOG(LogTemp, Warning, TEXT("Applied Shadow Quality: %d"), Quality);
        }
    }
    
    if (WidgetMap.Contains(TEXT("TextureQualitySlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("TextureQualitySlider")]);
        if (Slider)
        {
            int32 Quality = FMath::RoundToInt(Slider->GetValue());
            GameUserSettings->SetTextureQuality(Quality);
            UE_LOG(LogTemp, Warning, TEXT("Applied Texture Quality: %d"), Quality);
        }
    }
    
    if (WidgetMap.Contains(TEXT("AntiAliasingSlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("AntiAliasingSlider")]);
        if (Slider)
        {
            int32 Quality = FMath::RoundToInt(Slider->GetValue());
            GameUserSettings->SetAntiAliasingQuality(Quality);
            UE_LOG(LogTemp, Warning, TEXT("Applied Anti-Aliasing Quality: %d"), Quality);
        }
    }
    
    // Apply additional quality sliders
    if (WidgetMap.Contains(TEXT("PostProcessSlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("PostProcessSlider")]);
        if (Slider)
        {
            int32 Quality = FMath::RoundToInt(Slider->GetValue());
            GameUserSettings->SetPostProcessingQuality(Quality);
            UE_LOG(LogTemp, Warning, TEXT("Applied Post Processing Quality: %d"), Quality);
        }
    }
    
    if (WidgetMap.Contains(TEXT("EffectsQualitySlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("EffectsQualitySlider")]);
        if (Slider)
        {
            int32 Quality = FMath::RoundToInt(Slider->GetValue());
            GameUserSettings->SetVisualEffectQuality(Quality);
            UE_LOG(LogTemp, Warning, TEXT("Applied Effects Quality: %d"), Quality);
        }
    }
    
    if (WidgetMap.Contains(TEXT("FoliageQualitySlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("FoliageQualitySlider")]);
        if (Slider)
        {
            int32 Quality = FMath::RoundToInt(Slider->GetValue());
            GameUserSettings->SetFoliageQuality(Quality);
            UE_LOG(LogTemp, Warning, TEXT("Applied Foliage Quality: %d"), Quality);
        }
    }
    
    if (WidgetMap.Contains(TEXT("ShadingQualitySlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("ShadingQualitySlider")]);
        if (Slider)
        {
            int32 Quality = FMath::RoundToInt(Slider->GetValue());
            GameUserSettings->SetShadingQuality(Quality);
            UE_LOG(LogTemp, Warning, TEXT("Applied Shading Quality: %d"), Quality);
        }
    }
    
    // Apply VSync setting
    if (WidgetMap.Contains(TEXT("VSyncCheckBox")))
    {
        UCheckBox* CheckBox = Cast<UCheckBox>(WidgetMap[TEXT("VSyncCheckBox")]);
        if (CheckBox)
        {
            bool bVSync = CheckBox->IsChecked();
            GameUserSettings->SetVSyncEnabled(bVSync);
            UE_LOG(LogTemp, Warning, TEXT("Applied VSync: %s"), bVSync ? TEXT("ON") : TEXT("OFF"));
        }
    }
    
    // Apply Frame Rate Limit setting
    if (WidgetMap.Contains(TEXT("FrameRateLimitSlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("FrameRateLimitSlider")]);
        if (Slider)
        {
            float FrameRate = Slider->GetValue();
            GameUserSettings->SetFrameRateLimit(FrameRate);
            UE_LOG(LogTemp, Warning, TEXT("Applied Frame Rate Limit: %.0f FPS"), FrameRate);
        }
    }
}

void UDynamicSettingsWidget::ApplyAudioSettings()
{
    // Audio settings would typically be applied to an audio manager
    // For now, we'll just log the values
    UE_LOG(LogTemp, Warning, TEXT("Applying audio settings..."));
    
    if (WidgetMap.Contains(TEXT("MasterVolumeSlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("MasterVolumeSlider")]);
        if (Slider)
        {
            float Volume = Slider->GetValue();
            UE_LOG(LogTemp, Warning, TEXT("Master Volume set to: %.2f"), Volume);
            // In a real implementation, you would apply this to your audio system
        }
    }
}

void UDynamicSettingsWidget::CalculateLayoutConfig()
{
    FVector2D ViewportSize = GetCurrentViewportSize();
    
    UE_LOG(LogTemp, Warning, TEXT("Calculating layout for viewport size: %.0fx%.0f"), ViewportSize.X, ViewportSize.Y);
    
    // Base configuration
    LayoutConfig.MinWindowWidth = 1280.0f;
    LayoutConfig.MinWindowHeight = 720.0f;
    
    // Scale content width based on resolution with improved constraints
    float ScreenWidth = FMath::Max(ViewportSize.X, LayoutConfig.MinWindowWidth);
    // Improved responsive width - take up more screen space but leave margin
    float MaxAllowedWidth = ScreenWidth * 0.85f; // Use 85% of screen width
    LayoutConfig.MaxContentWidth = FMath::Clamp(ScreenWidth * 0.7f, 500.0f, MaxAllowedWidth);
    
    // Scale UI elements based on resolution
    float ScaleFactor = FMath::Clamp(ScreenWidth / 1920.0f, 0.8f, 1.5f);
    
    LayoutConfig.ControlHeight = GetScaledSize(40.0f);
    LayoutConfig.ControlSpacing = GetScaledSize(12.0f);
    LayoutConfig.SectionSpacing = GetScaledSize(24.0f);
    LayoutConfig.SidePadding = GetScaledSize(32.0f);
    LayoutConfig.TopBottomPadding = GetScaledSize(24.0f);
    LayoutConfig.ValueDisplayWidth = GetScaledSize(80.0f);
    
    // Label and control ratios
    LayoutConfig.LabelWidthRatio = 0.4f;
    LayoutConfig.ControlWidthRatio = 0.5f;
    
    UE_LOG(LogTemp, Warning, TEXT("Layout Config - Content Width: %.0f, Control Height: %.0f"), 
           LayoutConfig.MaxContentWidth, LayoutConfig.ControlHeight);
}

FVector2D UDynamicSettingsWidget::GetCurrentViewportSize()
{
    // Try to get viewport size from the widget layout library
    FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
    
    // Fallback to engine viewport if widget method fails
    if (ViewportSize.X <= 0 || ViewportSize.Y <= 0)
    {
        if (GEngine && GEngine->GameViewport)
        {
            GEngine->GameViewport->GetViewportSize(ViewportSize);
        }
        else
        {
            // Ultimate fallback
            ViewportSize = FVector2D(1920.0f, 1080.0f);
        }
    }
    
    return ViewportSize;
}

float UDynamicSettingsWidget::GetScaledSize(float BaseSize)
{
    FVector2D ViewportSize = GetCurrentViewportSize();
    float ScaleFactor = FMath::Clamp(ViewportSize.X / 1920.0f, 0.8f, 1.5f);
    return BaseSize * ScaleFactor;
}

void UDynamicSettingsWidget::GenerateSettingsUI()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating enhanced settings UI directly in target container"));
    
    if (!TargetContainer)
    {
        UE_LOG(LogTemp, Error, TEXT("TargetContainer is null!"));
        return;
    }
    
    // Create size box to constrain width
    USizeBox* SizeConstraint = NewObject<USizeBox>(this);
    SizeConstraint->SetMaxDesiredWidth(LayoutConfig.MaxContentWidth);
    UVerticalBoxSlot* SizeBoxSlot = TargetContainer->AddChildToVerticalBox(SizeConstraint);
    SizeBoxSlot->SetHorizontalAlignment(HAlign_Center);
    
    // Create main content container with padding
    UBorder* MainBorder = CreateStyledBorder();
    SizeConstraint->AddChild(MainBorder);
    
    UVerticalBox* ContentBox = NewObject<UVerticalBox>(this);
    MainBorder->SetContent(ContentBox);
    
    // Add title with proper styling
    if (!SettingsLayout.Title.IsEmpty())
    {
        UTextBlock* TitleText = CreateStyledSectionHeader(SettingsLayout.Title);
        ApplyTextStyle(TitleText, TEXT("Title"));
        ContentBox->AddChild(TitleText);
        ContentBox->AddChild(CreateVerticalSpacer(LayoutConfig.SectionSpacing));
    }
    
    // Create scrollable area for settings
    UScrollBox* ScrollBox = NewObject<UScrollBox>(this);
    ContentBox->AddChild(ScrollBox);
    
    // Generate sections with improved layout
    for (const FSettingsSection& Section : SettingsLayout.Sections)
    {
        // Add section header
        UTextBlock* SectionHeader = CreateStyledSectionHeader(Section.Name + TEXT(" Settings"));
        ApplyTextStyle(SectionHeader, TEXT("SectionHeader"));
        ScrollBox->AddChild(SectionHeader);
        ScrollBox->AddChild(CreateVerticalSpacer(LayoutConfig.ControlSpacing));
        
        // Add section controls with proper spacing
        for (const FSettingsControl& Control : Section.Controls)
        {
            UWidget* ControlWidget = CreateControlWidget(Control);
            if (ControlWidget)
            {
                ScrollBox->AddChild(ControlWidget);
                ScrollBox->AddChild(CreateVerticalSpacer(LayoutConfig.ControlSpacing));
                
                UE_LOG(LogTemp, Warning, TEXT("Added enhanced control: %s"), *Control.Name);
            }
        }
        
        // Add spacer between sections
        ScrollBox->AddChild(CreateVerticalSpacer(LayoutConfig.SectionSpacing));
    }
    
    // Add action buttons at the bottom
    UWidget* ActionButtons = CreateActionButtonsRow();
    ContentBox->AddChild(ActionButtons);
    
    // Load current settings into all controls
    LoadCurrentSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Enhanced settings UI generated successfully"));
}

void UDynamicSettingsWidget::CreateFallbackUI()
{
    if (!TargetContainer)
    {
        UE_LOG(LogTemp, Error, TEXT("TargetContainer is null for fallback UI!"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Creating fallback UI in target container"));
    
    // Add error text
    UTextBlock* ErrorText = NewObject<UTextBlock>(this);
    ErrorText->SetText(FText::FromString(TEXT("Settings Failed to Load\nUsing Simple Interface")));
    TargetContainer->AddChild(ErrorText);
    
    // Add spacer
    USpacer* Spacer = NewObject<USpacer>(this);
    Spacer->SetSize(FVector2D(1, 20));
    TargetContainer->AddChild(Spacer);
    
    // Add back button
    UButton* BackButton = NewObject<UButton>(this);
    BackButton->OnClicked.AddDynamic(this, &UDynamicSettingsWidget::OnButtonClicked);
    
    UTextBlock* BackButtonText = NewObject<UTextBlock>(this);
    BackButtonText->SetText(FText::FromString(TEXT("Back to Main Menu")));
    BackButton->AddChild(BackButtonText);
    
    TargetContainer->AddChild(BackButton);
    
    UE_LOG(LogTemp, Warning, TEXT("Fallback UI created"));
}

UWidget* UDynamicSettingsWidget::CreateControlWidget(const FSettingsControl& Control)
{
    if (Control.Type == TEXT("Slider"))
    {
        return CreateSliderWithLabel(Control);
    }
    else if (Control.Type == TEXT("ComboBox"))
    {
        return CreateComboBoxWithLabel(Control);
    }
    else if (Control.Type == TEXT("CheckBox"))
    {
        return CreateCheckBoxWithLabel(Control);
    }
    
    // If unknown type, create a simple text widget
    UTextBlock* UnknownWidget = NewObject<UTextBlock>(this);
    UnknownWidget->SetText(FText::FromString(FString::Printf(TEXT("Unknown Control: %s"), *Control.Type)));
    return UnknownWidget;
}

UWidget* UDynamicSettingsWidget::CreateSliderWithLabel(const FSettingsControl& Control)
{
    // Create horizontal container with proper sizing
    UHorizontalBox* Container = NewObject<UHorizontalBox>(this);
    
    // Create styled label with fixed width
    UTextBlock* Label = CreateStyledLabel(Control.Label);
    UHorizontalBoxSlot* LabelSlot = Container->AddChildToHorizontalBox(Label);
    LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    LabelSlot->SetHorizontalAlignment(HAlign_Left);
    LabelSlot->SetVerticalAlignment(VAlign_Center);
    
    // Add spacer between label and control
    USpacer* Spacer = CreateHorizontalSpacer(LayoutConfig.ControlSpacing);
    Container->AddChildToHorizontalBox(Spacer);
    
    // Create slider with proper styling
    USlider* Slider = NewObject<USlider>(this);
    Slider->SetMinValue(Control.Min);
    Slider->SetMaxValue(Control.Max);
    Slider->SetStepSize(Control.Step);
    ApplySliderStyle(Slider);
    
    // Bind slider event
    Slider->OnValueChanged.AddDynamic(this, &UDynamicSettingsWidget::OnSliderValueChanged);
    
    UHorizontalBoxSlot* SliderSlot = Container->AddChildToHorizontalBox(Slider);
    SliderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    SliderSlot->SetHorizontalAlignment(HAlign_Fill);
    SliderSlot->SetVerticalAlignment(VAlign_Center);
    
    // Create value display if needed
    if (Control.ShowValue)
    {
        Container->AddChildToHorizontalBox(CreateHorizontalSpacer(LayoutConfig.ControlSpacing));
        
        UTextBlock* ValueText = CreateStyledValueDisplay();
        ValueText->SetText(FText::FromString(FormatSliderValue(Slider->GetValue(), Control)));
        UHorizontalBoxSlot* ValueSlot = Container->AddChildToHorizontalBox(ValueText);
        ValueSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
        ValueSlot->SetHorizontalAlignment(HAlign_Right);
        ValueSlot->SetVerticalAlignment(VAlign_Center);
        
        // Store reference to value text
        WidgetMap.Add(Control.Name + TEXT("ValueText"), ValueText);
    }
    
    // IMPORTANT: Store the actual slider widget in the map, not the container
    WidgetMap.Add(Control.Name, Slider);
    
    return Container;
}

UWidget* UDynamicSettingsWidget::CreateComboBoxWithLabel(const FSettingsControl& Control)
{
    // Create horizontal container with proper sizing
    UHorizontalBox* Container = NewObject<UHorizontalBox>(this);
    
    // Create styled label with fixed width
    UTextBlock* Label = CreateStyledLabel(Control.Label);
    UHorizontalBoxSlot* LabelSlot = Container->AddChildToHorizontalBox(Label);
    LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    LabelSlot->SetHorizontalAlignment(HAlign_Left);
    LabelSlot->SetVerticalAlignment(VAlign_Center);
    
    // Add spacer between label and control
    USpacer* Spacer = CreateHorizontalSpacer(LayoutConfig.ControlSpacing);
    Container->AddChildToHorizontalBox(Spacer);
    
    // Create combo box with proper styling and width
    UComboBoxString* ComboBox = NewObject<UComboBoxString>(this);
    
    // Populate with real options based on control name
    PopulateComboBoxOptions(ComboBox, Control);
    
    // Apply styling
    ApplyComboBoxStyle(ComboBox);
    
    // Bind combo box event
    ComboBox->OnSelectionChanged.AddDynamic(this, &UDynamicSettingsWidget::OnComboBoxSelectionChanged);
    
    UHorizontalBoxSlot* ComboSlot = Container->AddChildToHorizontalBox(ComboBox);
    ComboSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    ComboSlot->SetHorizontalAlignment(HAlign_Fill);
    ComboSlot->SetVerticalAlignment(VAlign_Center);
    
    // IMPORTANT: Store the actual combo box widget in the map, not the container
    WidgetMap.Add(Control.Name, ComboBox);
    
    return Container;
}

UWidget* UDynamicSettingsWidget::CreateCheckBoxWithLabel(const FSettingsControl& Control)
{
    // Create horizontal container with proper sizing
    UHorizontalBox* Container = NewObject<UHorizontalBox>(this);
    
    // Create styled label with fixed width
    UTextBlock* Label = CreateStyledLabel(Control.Label);
    UHorizontalBoxSlot* LabelSlot = Container->AddChildToHorizontalBox(Label);
    LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    LabelSlot->SetHorizontalAlignment(HAlign_Left);
    LabelSlot->SetVerticalAlignment(VAlign_Center);
    
    // Add spacer between label and control
    USpacer* Spacer = CreateHorizontalSpacer(LayoutConfig.ControlSpacing);
    Container->AddChildToHorizontalBox(Spacer);
    
    // Create checkbox with proper styling
    UCheckBox* CheckBox = NewObject<UCheckBox>(this);
    ApplyCheckBoxStyle(CheckBox);
    CheckBox->OnCheckStateChanged.AddDynamic(this, &UDynamicSettingsWidget::OnCheckBoxStateChanged);
    
    UHorizontalBoxSlot* CheckBoxSlot = Container->AddChildToHorizontalBox(CheckBox);
    CheckBoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
    CheckBoxSlot->SetHorizontalAlignment(HAlign_Left);
    CheckBoxSlot->SetVerticalAlignment(VAlign_Center);
    
    // IMPORTANT: Store the actual checkbox widget in the map, not the container
    WidgetMap.Add(Control.Name, CheckBox);
    
    return Container;
}

UWidget* UDynamicSettingsWidget::CreateButton(const FSettingsControl& Control)
{
    UButton* Button = NewObject<UButton>(this);
    
    // Bind different functions based on button name
    if (Control.Name == TEXT("ApplyButton"))
    {
        Button->OnClicked.AddDynamic(this, &UDynamicSettingsWidget::OnApplySettingsClicked);
    }
    else if (Control.Name == TEXT("ResetToDefaultsButton"))
    {
        Button->OnClicked.AddDynamic(this, &UDynamicSettingsWidget::OnResetToDefaultsClicked);
    }
    else if (Control.Name == TEXT("BackButton"))
    {
        Button->OnClicked.AddDynamic(this, &UDynamicSettingsWidget::OnButtonClicked);
    }
    else
    {
        Button->OnClicked.AddDynamic(this, &UDynamicSettingsWidget::OnButtonClicked);
    }
    
    // Apply button styling
    ApplyButtonStyle(Button, Control.Style);
    
    // Create button text
    UTextBlock* ButtonText = CreateStyledLabel(Control.Label);
    ButtonText->SetColorAndOpacity(GetTextColor());
    Button->AddChild(ButtonText);
    
    return Button;
}

UWidget* UDynamicSettingsWidget::CreateSectionHeader(const FString& SectionName)
{
    UTextBlock* Header = NewObject<UTextBlock>(this);
    Header->SetText(FText::FromString(SectionName));
    
    return Header;
}

UBorder* UDynamicSettingsWidget::CreateStyledBorder()
{
    UBorder* Border = NewObject<UBorder>(this);
    
    // Set background color
    Border->SetBrushColor(GetBackgroundColor());
    
    // Set padding based on layout config
    FMargin Padding(LayoutConfig.SidePadding, LayoutConfig.TopBottomPadding, LayoutConfig.SidePadding, LayoutConfig.TopBottomPadding);
    Border->SetPadding(Padding);
    
    return Border;
}

UTextBlock* UDynamicSettingsWidget::CreateStyledLabel(const FString& LabelText)
{
    UTextBlock* Label = NewObject<UTextBlock>(this);
    Label->SetText(FText::FromString(LabelText));
    ApplyTextStyle(Label, TEXT("Label"));
    return Label;
}

UTextBlock* UDynamicSettingsWidget::CreateStyledSectionHeader(const FString& SectionName)
{
    UTextBlock* Header = NewObject<UTextBlock>(this);
    Header->SetText(FText::FromString(SectionName));
    ApplyTextStyle(Header, TEXT("SectionHeader"));
    return Header;
}

USpacer* UDynamicSettingsWidget::CreateVerticalSpacer(float Height)
{
    USpacer* Spacer = NewObject<USpacer>(this);
    Spacer->SetSize(FVector2D(1.0f, Height));
    return Spacer;
}

USpacer* UDynamicSettingsWidget::CreateHorizontalSpacer(float Width)
{
    USpacer* Spacer = NewObject<USpacer>(this);
    Spacer->SetSize(FVector2D(Width, 1.0f));
    return Spacer;
}

UTextBlock* UDynamicSettingsWidget::CreateStyledValueDisplay()
{
    UTextBlock* ValueText = NewObject<UTextBlock>(this);
    ApplyTextStyle(ValueText, TEXT("Value"));
    ValueText->SetColorAndOpacity(GetAccentColor());
    return ValueText;
}

UWidget* UDynamicSettingsWidget::CreateActionButtonsRow()
{
    UHorizontalBox* ButtonRow = NewObject<UHorizontalBox>(this);
    
    // Add action buttons from JSON configuration
    for (const FSettingsControl& Button : SettingsLayout.ActionButtons)
    {
        UWidget* ButtonWidget = CreateButton(Button);
        UHorizontalBoxSlot* ButtonSlot = ButtonRow->AddChildToHorizontalBox(ButtonWidget);
        ButtonSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
        ButtonSlot->SetPadding(FMargin(0, 0, LayoutConfig.ControlSpacing, 0));
        
        WidgetMap.Add(Button.Name, ButtonWidget);
    }
    
    return ButtonRow;
}

void UDynamicSettingsWidget::ApplyTextStyle(UTextBlock* TextBlock, const FString& StyleType)
{
    if (!TextBlock) return;
    
    TextBlock->SetColorAndOpacity(GetTextColor());
    
    // Apply different sizes based on style type
    if (StyleType == TEXT("Title"))
    {
        // Large title text
        TextBlock->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), GetScaledSize(24)));
    }
    else if (StyleType == TEXT("SectionHeader"))
    {
        // Medium header text  
        TextBlock->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), GetScaledSize(18)));
    }
    else if (StyleType == TEXT("Error"))
    {
        // Error text in red
        TextBlock->SetColorAndOpacity(FLinearColor::Red);
        TextBlock->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), GetScaledSize(14)));
    }
    else
    {
        // Regular text
        TextBlock->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), GetScaledSize(14)));
    }
}

void UDynamicSettingsWidget::ApplySliderStyle(USlider* Slider)
{
    if (!Slider) return;
    
    // Set slider appearance
    Slider->SetSliderBarColor(GetSecondaryColor());
    Slider->SetSliderHandleColor(GetPrimaryColor());
    
    // Additional styling can be added here
}

void UDynamicSettingsWidget::ApplyComboBoxStyle(UComboBoxString* ComboBox)
{
    if (!ComboBox) return;
    
    // Additional styling can be added here
}

void UDynamicSettingsWidget::ApplyCheckBoxStyle(UCheckBox* CheckBox)
{
    if (!CheckBox) return;
    
    // Additional checkbox styling can be added here
}

void UDynamicSettingsWidget::ApplyButtonStyle(UButton* Button, const FString& StyleType)
{
    if (!Button) return;
    
    // Set button colors based on style type
    if (StyleType == TEXT("Primary"))
    {
        Button->SetBackgroundColor(GetPrimaryColor());
    }
    else
    {
        Button->SetBackgroundColor(GetSecondaryColor());
    }
}

// Event handler implementations
void UDynamicSettingsWidget::OnSliderValueChanged(float Value)
{
    UE_LOG(LogTemp, Warning, TEXT("Slider changed to: %.2f"), Value);
    
    // TODO: Implement proper slider identification and value update
    // We need to identify which slider was changed and update its value display
}

void UDynamicSettingsWidget::OnComboBoxSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    UE_LOG(LogTemp, Warning, TEXT("ComboBox selection changed to: %s"), *SelectedItem);
}

void UDynamicSettingsWidget::OnCheckBoxStateChanged(bool bIsChecked)
{
    UE_LOG(LogTemp, Warning, TEXT("CheckBox toggled: %s"), bIsChecked ? TEXT("ON") : TEXT("OFF"));
}

void UDynamicSettingsWidget::OnButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Settings menu button clicked - going back to main menu"));
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, TEXT("Going back to main menu..."));
    }
    OnBackToMainMenu.Broadcast();
}

void UDynamicSettingsWidget::OnApplySettingsClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Apply Settings clicked"));
    ApplySettings();
}

void UDynamicSettingsWidget::OnResetToDefaultsClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Reset to Defaults clicked"));
    ResetToDefaults();
}

void UDynamicSettingsWidget::LoadCurrentSettings()
{
    UE_LOG(LogTemp, Warning, TEXT("Loading current settings into controls..."));
    
    if (!GameUserSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("GameUserSettings is null, cannot load current settings!"));
        return;
    }
    
    // Load graphics quality sliders
    if (WidgetMap.Contains(TEXT("ViewDistanceSlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("ViewDistanceSlider")]);
        if (Slider)
        {
            float CurrentValue = static_cast<float>(GameUserSettings->GetViewDistanceQuality());
            Slider->SetValue(CurrentValue);
            UE_LOG(LogTemp, Warning, TEXT("Loaded View Distance Quality: %.0f"), CurrentValue);
        }
    }
    
    if (WidgetMap.Contains(TEXT("ShadowQualitySlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("ShadowQualitySlider")]);
        if (Slider)
        {
            float CurrentValue = static_cast<float>(GameUserSettings->GetShadowQuality());
            Slider->SetValue(CurrentValue);
            UE_LOG(LogTemp, Warning, TEXT("Loaded Shadow Quality: %.0f"), CurrentValue);
        }
    }
    
    if (WidgetMap.Contains(TEXT("TextureQualitySlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("TextureQualitySlider")]);
        if (Slider)
        {
            float CurrentValue = static_cast<float>(GameUserSettings->GetTextureQuality());
            Slider->SetValue(CurrentValue);
            UE_LOG(LogTemp, Warning, TEXT("Loaded Texture Quality: %.0f"), CurrentValue);
        }
    }
    
    if (WidgetMap.Contains(TEXT("AntiAliasingSlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("AntiAliasingSlider")]);
        if (Slider)
        {
            float CurrentValue = static_cast<float>(GameUserSettings->GetAntiAliasingQuality());
            Slider->SetValue(CurrentValue);
            UE_LOG(LogTemp, Warning, TEXT("Loaded Anti-Aliasing Quality: %.0f"), CurrentValue);
        }
    }
    
    if (WidgetMap.Contains(TEXT("PostProcessSlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("PostProcessSlider")]);
        if (Slider)
        {
            float CurrentValue = static_cast<float>(GameUserSettings->GetPostProcessingQuality());
            Slider->SetValue(CurrentValue);
            UE_LOG(LogTemp, Warning, TEXT("Loaded Post Processing Quality: %.0f"), CurrentValue);
        }
    }
    
    if (WidgetMap.Contains(TEXT("EffectsQualitySlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("EffectsQualitySlider")]);
        if (Slider)
        {
            float CurrentValue = static_cast<float>(GameUserSettings->GetVisualEffectQuality());
            Slider->SetValue(CurrentValue);
            UE_LOG(LogTemp, Warning, TEXT("Loaded Effects Quality: %.0f"), CurrentValue);
        }
    }
    
    if (WidgetMap.Contains(TEXT("FoliageQualitySlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("FoliageQualitySlider")]);
        if (Slider)
        {
            float CurrentValue = static_cast<float>(GameUserSettings->GetFoliageQuality());
            Slider->SetValue(CurrentValue);
            UE_LOG(LogTemp, Warning, TEXT("Loaded Foliage Quality: %.0f"), CurrentValue);
        }
    }
    
    if (WidgetMap.Contains(TEXT("ShadingQualitySlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("ShadingQualitySlider")]);
        if (Slider)
        {
            float CurrentValue = static_cast<float>(GameUserSettings->GetShadingQuality());
            Slider->SetValue(CurrentValue);
            UE_LOG(LogTemp, Warning, TEXT("Loaded Shading Quality: %.0f"), CurrentValue);
        }
    }
    
    // Load VSync setting
    if (WidgetMap.Contains(TEXT("VSyncCheckBox")))
    {
        UCheckBox* CheckBox = Cast<UCheckBox>(WidgetMap[TEXT("VSyncCheckBox")]);
        if (CheckBox)
        {
            bool bVSyncEnabled = GameUserSettings->IsVSyncEnabled();
            CheckBox->SetIsChecked(bVSyncEnabled);
            UE_LOG(LogTemp, Warning, TEXT("Loaded VSync: %s"), bVSyncEnabled ? TEXT("ON") : TEXT("OFF"));
        }
    }
    
    // Load Frame Rate Limit setting
    if (WidgetMap.Contains(TEXT("FrameRateLimitSlider")))
    {
        USlider* Slider = Cast<USlider>(WidgetMap[TEXT("FrameRateLimitSlider")]);
        if (Slider)
        {
            float CurrentValue = GameUserSettings->GetFrameRateLimit();
            Slider->SetValue(CurrentValue);
            UE_LOG(LogTemp, Warning, TEXT("Loaded Frame Rate Limit: %.0f FPS"), CurrentValue);
        }
    }
    
    // Note: Resolution and Window Mode are loaded in PopulateComboBoxOptions
    
    UE_LOG(LogTemp, Warning, TEXT("Current settings loaded successfully!"));
}

void UDynamicSettingsWidget::ApplySettings()
{
    UE_LOG(LogTemp, Warning, TEXT("Applying settings..."));
    
    // Ensure GameUserSettings is available with multiple fallback attempts
    if (!GameUserSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameUserSettings is null in ApplySettings, attempting to get it..."));
        
        // Try multiple methods to get GameUserSettings
        GameUserSettings = UGameUserSettings::GetGameUserSettings();
        
        if (!GameUserSettings && GEngine)
        {
            GameUserSettings = GEngine->GetGameUserSettings();
            UE_LOG(LogTemp, Warning, TEXT("Tried GEngine->GetGameUserSettings()"));
        }
        
        // Last resort: try to create one
        if (!GameUserSettings)
        {
            UE_LOG(LogTemp, Error, TEXT("All attempts to get GameUserSettings failed!"));
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERROR: Cannot access GameUserSettings!"));
            }
            return;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Successfully obtained GameUserSettings in ApplySettings"));
            GameUserSettings->LoadSettings();
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GameUserSettings confirmed available for applying settings"));
    
    bool bSettingsChanged = false;
    
    // Apply resolution setting
    if (WidgetMap.Contains(TEXT("ResolutionComboBox")))
    {
        UComboBoxString* ResCombo = Cast<UComboBoxString>(WidgetMap[TEXT("ResolutionComboBox")]);
        if (ResCombo)
        {
            FString SelectedResolution = ResCombo->GetSelectedOption();
            UE_LOG(LogTemp, Warning, TEXT("Resolution ComboBox found - Selected: '%s'"), *SelectedResolution);
            
            if (!SelectedResolution.IsEmpty())
            {
                FString WidthStr, HeightStr;
                if (SelectedResolution.Split(TEXT("x"), &WidthStr, &HeightStr))
                {
                    int32 Width = FCString::Atoi(*WidthStr);
                    int32 Height = FCString::Atoi(*HeightStr);
                    
                    // Get current resolution to compare
                    FIntPoint CurrentRes = GameUserSettings->GetScreenResolution();
                    if (CurrentRes.X != Width || CurrentRes.Y != Height)
                    {
                    GameUserSettings->SetScreenResolution(FIntPoint(Width, Height));
                    bSettingsChanged = true;
                    UE_LOG(LogTemp, Warning, TEXT("Resolution changed from %dx%d to %dx%d"), CurrentRes.X, CurrentRes.Y, Width, Height);
                    if (GEngine)
                    {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, FString::Printf(TEXT("Resolution: %dx%d"), Width, Height));
                    }
                        
                    // Force mouse input recalibration after resolution change
                    bNeedsMouseRecalibration = true;
                }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Resolution unchanged: %dx%d"), Width, Height);
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to parse resolution string: %s"), *SelectedResolution);
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Resolution ComboBox selection is empty!"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to cast ResolutionComboBox widget to UComboBoxString!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ResolutionComboBox not found in WidgetMap!"));
    }
    
    // Apply window mode setting
    if (WidgetMap.Contains(TEXT("WindowModeComboBox")))
    {
        UComboBoxString* WindowCombo = Cast<UComboBoxString>(WidgetMap[TEXT("WindowModeComboBox")]);
        if (WindowCombo)
        {
            FString SelectedMode = WindowCombo->GetSelectedOption();
            UE_LOG(LogTemp, Warning, TEXT("Window Mode ComboBox found - Selected: '%s'"), *SelectedMode);
            
            if (!SelectedMode.IsEmpty())
            {
                EWindowMode::Type WindowMode = EWindowMode::Windowed;
                
                if (SelectedMode == TEXT("Fullscreen"))
                {
                    WindowMode = EWindowMode::Fullscreen;
                }
                else if (SelectedMode == TEXT("Windowed Fullscreen"))
                {
                    WindowMode = EWindowMode::WindowedFullscreen;
                }
                else if (SelectedMode == TEXT("Windowed"))
                {
                    WindowMode = EWindowMode::Windowed;
                }
                
                // Get current window mode to compare
                EWindowMode::Type CurrentMode = GameUserSettings->GetFullscreenMode();
                UE_LOG(LogTemp, Warning, TEXT("Current window mode: %d, Selected mode: %d"), (int32)CurrentMode, (int32)WindowMode);
                
                if (CurrentMode != WindowMode)
                {
                    GameUserSettings->SetFullscreenMode(WindowMode);
                    bSettingsChanged = true;
                    UE_LOG(LogTemp, Warning, TEXT("Window mode changed to: %s (enum: %d)"), *SelectedMode, (int32)WindowMode);
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, FString::Printf(TEXT("Window Mode: %s"), *SelectedMode));
                    }
                    
                    // Window mode changes can also affect mouse coordinates
                    bNeedsMouseRecalibration = true;
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Window mode unchanged: %s"), *SelectedMode);
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Window Mode ComboBox selection is empty!"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to cast WindowModeComboBox widget to UComboBoxString!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WindowModeComboBox not found in WidgetMap!"));
    }
    
    // Apply graphics quality settings
    ApplyGraphicsQualitySettings();
    
    // Apply audio settings
    ApplyAudioSettings();
    
    // Save and apply the settings with proper confirmation handling
    if (bSettingsChanged)
    {
        UE_LOG(LogTemp, Warning, TEXT("Applying resolution/window mode changes..."));
        
        // Apply non-resolution settings first
        GameUserSettings->ApplyNonResolutionSettings();
        
        // Then apply resolution settings with confirmation
        GameUserSettings->ApplyResolutionSettings(false); // false = don't show confirmation dialog
        
        // Save all settings
        GameUserSettings->SaveSettings();
        
        UE_LOG(LogTemp, Warning, TEXT("All settings applied and saved successfully!"));
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Settings Applied Successfully!"));
        }
        
        // Handle mouse recalibration if needed
        if (bNeedsMouseRecalibration)
        {
            RecalibrateMouseInput();
            bNeedsMouseRecalibration = false;
            
            // For windowed mode, try automatic toggle fix
            if (GameUserSettings && GameUserSettings->GetFullscreenMode() == EWindowMode::Windowed)
            {
                // Try the automatic toggle fix first
                AutoToggleWindowModeForMouseFix();
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Orange, 
                        TEXT("WINDOWED MODE: Attempting auto-fix for mouse offset. If still offset, manually toggle Fullscreen/Windowed."));
                }
            }
        }
    }
    else
    {
        // Still apply and save other settings even if resolution/window mode didn't change
        GameUserSettings->ApplySettings(false);
        GameUserSettings->SaveSettings();
        
        UE_LOG(LogTemp, Warning, TEXT("Settings saved (no resolution/window changes)"));
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Settings Saved!"));
        }
    }
}

void UDynamicSettingsWidget::ResetToDefaults()
{
    UE_LOG(LogTemp, Warning, TEXT("Resetting settings to defaults..."));
    
    if (!GameUserSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("GameUserSettings is null, cannot reset to defaults!"));
        return;
    }
    
    // Reset graphics settings to default values
    GameUserSettings->SetToDefaults();
    
    // Reload the current (now default) settings into the UI controls
    LoadCurrentSettings();
    
    // Apply and save the default settings
    GameUserSettings->ApplySettings(false);
    GameUserSettings->SaveSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Settings reset to defaults and applied successfully!"));
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, TEXT("Settings Reset to Defaults!"));
    }
}

void UDynamicSettingsWidget::UpdateValueDisplay(const FString& SliderName, float Value, const FSettingsControl& Control)
{
    UE_LOG(LogTemp, Warning, TEXT("Updating value display for %s: %.2f"), *SliderName, Value);
    
    // Find and update the value display widget
    FString ValueTextKey = SliderName + TEXT("ValueText");
    if (WidgetMap.Contains(ValueTextKey))
    {
        UTextBlock* ValueText = Cast<UTextBlock>(WidgetMap[ValueTextKey]);
        if (ValueText)
        {
            FString FormattedValue = FormatSliderValue(Value, Control);
            ValueText->SetText(FText::FromString(FormattedValue));
        }
    }
}

FString UDynamicSettingsWidget::QualityLevelToString(int32 QualityLevel)
{
    switch (QualityLevel)
    {
    case 0: return TEXT("Low");
    case 1: return TEXT("Medium");
    case 2: return TEXT("High");
    case 3: return TEXT("Epic");
    case 4: return TEXT("Cinematic");
    default: return TEXT("Medium");
    }
}

FString UDynamicSettingsWidget::FormatSliderValue(float Value, const FSettingsControl& Control)
{
    // Handle special zero text
    if (Value == 0.0f && !Control.ZeroText.IsEmpty())
    {
        return Control.ZeroText;
    }
    
    // Apply value multiplier (for percentages, etc.)
    float DisplayValue = Value * Control.ValueMultiplier;
    
    // Format based on step size
    FString FormattedValue;
    if (Control.Step >= 1.0f)
    {
        FormattedValue = FString::Printf(TEXT("%.0f"), DisplayValue);
    }
    else if (Control.Step >= 0.1f)
    {
        FormattedValue = FString::Printf(TEXT("%.1f"), DisplayValue);
    }
    else
    {
        FormattedValue = FString::Printf(TEXT("%.2f"), DisplayValue);
    }
    
    // Add suffix if specified
    if (!Control.ValueSuffix.IsEmpty())
    {
        FormattedValue += Control.ValueSuffix;
    }
    
    return FormattedValue;
}

void UDynamicSettingsWidget::PopulateComboBoxOptions(UComboBoxString* ComboBox, const FSettingsControl& Control)
{
    if (!ComboBox) return;
    
    // Populate based on control name with real values
    if (Control.Name == TEXT("ResolutionComboBox"))
    {
        // Clear any existing options
        ComboBox->ClearOptions();
        
        // Add common resolutions manually
        TArray<FIntPoint> Resolutions;
        Resolutions.Add(FIntPoint(1280, 720));
        Resolutions.Add(FIntPoint(1366, 768));
        Resolutions.Add(FIntPoint(1440, 900));
        Resolutions.Add(FIntPoint(1600, 900));
        Resolutions.Add(FIntPoint(1680, 1050));
        Resolutions.Add(FIntPoint(1920, 1080));
        Resolutions.Add(FIntPoint(2560, 1440));
        Resolutions.Add(FIntPoint(3840, 2160));
        
        // Add resolution options
        for (const FIntPoint& Resolution : Resolutions)
        {
            FString ResolutionString = FString::Printf(TEXT("%dx%d"), Resolution.X, Resolution.Y);
            ComboBox->AddOption(ResolutionString);
        }
        
        // Set current resolution as selected
        if (GameUserSettings)
        {
            FIntPoint CurrentRes = GameUserSettings->GetScreenResolution();
            FString CurrentResString = FString::Printf(TEXT("%dx%d"), CurrentRes.X, CurrentRes.Y);
            
            // Check if current resolution is in our list, if not add it
            int32 FoundIndex = ComboBox->FindOptionIndex(CurrentResString);
            if (FoundIndex == -1)
            {
                ComboBox->AddOption(CurrentResString);
                UE_LOG(LogTemp, Warning, TEXT("Added current resolution to list: %s"), *CurrentResString);
            }
            
            ComboBox->SetSelectedOption(CurrentResString);
            UE_LOG(LogTemp, Warning, TEXT("Set current resolution selection: %s"), *CurrentResString);
        }
    }
    else if (Control.Name == TEXT("WindowModeComboBox"))
    {
        // Clear any existing options
        ComboBox->ClearOptions();
        
        // Add window mode options
        ComboBox->AddOption(TEXT("Fullscreen"));
        ComboBox->AddOption(TEXT("Windowed Fullscreen"));
        ComboBox->AddOption(TEXT("Windowed"));
        
        // Set current window mode as selected
        if (GameUserSettings)
        {
            EWindowMode::Type CurrentMode = GameUserSettings->GetFullscreenMode();
            FString ModeString;
            switch (CurrentMode)
            {
            case EWindowMode::Fullscreen:
                ModeString = TEXT("Fullscreen");
                break;
            case EWindowMode::WindowedFullscreen:
                ModeString = TEXT("Windowed Fullscreen");
                break;
            case EWindowMode::Windowed:
                ModeString = TEXT("Windowed");
                break;
            default:
                ModeString = TEXT("Windowed");
                break;
            }
            ComboBox->SetSelectedOption(ModeString);
            UE_LOG(LogTemp, Warning, TEXT("Set current window mode selection: %s"), *ModeString);
        }
    }
    else
    {
        // Clear any existing options
        ComboBox->ClearOptions();
        
        // Use default options from JSON
        for (const FString& Option : Control.Options)
        {
            ComboBox->AddOption(Option);
        }
        
        // Set first option as default if available
        if (Control.Options.Num() > 0)
        {
            ComboBox->SetSelectedOption(Control.Options[0]);
            UE_LOG(LogTemp, Warning, TEXT("Set default option for %s: %s"), *Control.Name, *Control.Options[0]);
        }
    }
}
