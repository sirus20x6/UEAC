// /thearray/git/UnrealEngine/astrochicken/Source/astrochicken/UI/SettingsWidget.cpp

#include "SettingsWidget.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetSystemLibrary.h"

void USettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Get the game user settings
    GameUserSettings = UGameUserSettings::GetGameUserSettings();

    // Bind button events
    if (ApplyButton)
    {
        ApplyButton->OnClicked.AddDynamic(this, &USettingsWidget::OnApplyClicked);
    }

    if (ResetToDefaultsButton)
    {
        ResetToDefaultsButton->OnClicked.AddDynamic(this, &USettingsWidget::OnResetToDefaultsClicked);
    }

    if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &USettingsWidget::OnBackClicked);
    }

    // Bind slider events
    if (ViewDistanceSlider)
    {
        ViewDistanceSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnViewDistanceChanged);
    }

    if (ShadowQualitySlider)
    {
        ShadowQualitySlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnShadowQualityChanged);
    }

    if (TextureQualitySlider)
    {
        TextureQualitySlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnTextureQualityChanged);
    }

    if (AntiAliasingSlider)
    {
        AntiAliasingSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnAntiAliasingChanged);
    }

    if (PostProcessSlider)
    {
        PostProcessSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnPostProcessChanged);
    }

    if (EffectsQualitySlider)
    {
        EffectsQualitySlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnEffectsQualityChanged);
    }

    if (FoliageQualitySlider)
    {
        FoliageQualitySlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnFoliageQualityChanged);
    }

    if (ShadingQualitySlider)
    {
        ShadingQualitySlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnShadingQualityChanged);
    }

    if (FrameRateLimitSlider)
    {
        FrameRateLimitSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnFrameRateLimitChanged);
    }

    if (MasterVolumeSlider)
    {
        MasterVolumeSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnMasterVolumeChanged);
    }

    if (MusicVolumeSlider)
    {
        MusicVolumeSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnMusicVolumeChanged);
    }

    if (SFXVolumeSlider)
    {
        SFXVolumeSlider->OnValueChanged.AddDynamic(this, &USettingsWidget::OnSFXVolumeChanged);
    }

    // Bind ComboBox events
    if (ResolutionComboBox)
    {
        ResolutionComboBox->OnSelectionChanged.AddDynamic(this, &USettingsWidget::OnResolutionSelectionChanged);
    }

    if (WindowModeComboBox)
    {
        WindowModeComboBox->OnSelectionChanged.AddDynamic(this, &USettingsWidget::OnWindowModeSelectionChanged);
    }

    if (GraphicsQualityComboBox)
    {
        GraphicsQualityComboBox->OnSelectionChanged.AddDynamic(this, &USettingsWidget::OnGraphicsQualitySelectionChanged);
    }

    // Bind CheckBox events
    if (VSyncCheckBox)
    {
        VSyncCheckBox->OnCheckStateChanged.AddDynamic(this, &USettingsWidget::OnVSyncToggled);
    }

    // Populate combo boxes and load current settings
    PopulateResolutionComboBox();
    PopulateWindowModeComboBox();
    PopulateGraphicsQualityComboBox();
    LoadCurrentSettings();
}

void USettingsWidget::PopulateResolutionComboBox()
{
    if (!ResolutionComboBox) return;

    ResolutionComboBox->ClearOptions();
    
    // Add common resolutions
    TArray<FString> Resolutions = {
        TEXT("1920x1080"),
        TEXT("1680x1050"),
        TEXT("1600x900"),
        TEXT("1440x900"),
        TEXT("1366x768"),
        TEXT("1280x720"),
        TEXT("2560x1440"),
        TEXT("3840x2160")
    };

    for (const FString& Resolution : Resolutions)
    {
        ResolutionComboBox->AddOption(Resolution);
    }
}

void USettingsWidget::PopulateWindowModeComboBox()
{
    if (!WindowModeComboBox) return;

    WindowModeComboBox->ClearOptions();
    WindowModeComboBox->AddOption(TEXT("Fullscreen"));
    WindowModeComboBox->AddOption(TEXT("Windowed Fullscreen"));
    WindowModeComboBox->AddOption(TEXT("Windowed"));
}

void USettingsWidget::PopulateGraphicsQualityComboBox()
{
    if (!GraphicsQualityComboBox) return;

    GraphicsQualityComboBox->ClearOptions();
    GraphicsQualityComboBox->AddOption(TEXT("Low"));
    GraphicsQualityComboBox->AddOption(TEXT("Medium"));
    GraphicsQualityComboBox->AddOption(TEXT("High"));
    GraphicsQualityComboBox->AddOption(TEXT("Epic"));
    GraphicsQualityComboBox->AddOption(TEXT("Cinematic"));
}

void USettingsWidget::LoadCurrentSettings()
{
    if (!GameUserSettings) return;

    // Load resolution
    FIntPoint CurrentResolution = GameUserSettings->GetScreenResolution();
    FString CurrentResolutionString = FString::Printf(TEXT("%dx%d"), CurrentResolution.X, CurrentResolution.Y);
    if (ResolutionComboBox)
    {
        ResolutionComboBox->SetSelectedOption(CurrentResolutionString);
    }

    // Load window mode
    if (WindowModeComboBox)
    {
        EWindowMode::Type WindowMode = GameUserSettings->GetFullscreenMode();
        switch (WindowMode)
        {
        case EWindowMode::Fullscreen:
            WindowModeComboBox->SetSelectedOption(TEXT("Fullscreen"));
            break;
        case EWindowMode::WindowedFullscreen:
            WindowModeComboBox->SetSelectedOption(TEXT("Windowed Fullscreen"));
            break;
        case EWindowMode::Windowed:
            WindowModeComboBox->SetSelectedOption(TEXT("Windowed"));
            break;
        }
    }

    // Load overall graphics quality
    if (GraphicsQualityComboBox)
    {
        int32 OverallQuality = GameUserSettings->GetOverallScalabilityLevel();
        GraphicsQualityComboBox->SetSelectedOption(QualityLevelToString(OverallQuality));
    }

    // Load individual quality settings
    if (ViewDistanceSlider)
    {
        ViewDistanceSlider->SetValue(GameUserSettings->GetViewDistanceQuality());
    }

    if (ShadowQualitySlider)
    {
        ShadowQualitySlider->SetValue(GameUserSettings->GetShadowQuality());
    }

    if (TextureQualitySlider)
    {
        TextureQualitySlider->SetValue(GameUserSettings->GetTextureQuality());
    }

    if (AntiAliasingSlider)
    {
        AntiAliasingSlider->SetValue(GameUserSettings->GetAntiAliasingQuality());
    }

    if (PostProcessSlider)
    {
        PostProcessSlider->SetValue(GameUserSettings->GetPostProcessingQuality());
    }

    if (EffectsQualitySlider)
    {
        EffectsQualitySlider->SetValue(GameUserSettings->GetVisualEffectQuality());
    }

    if (FoliageQualitySlider)
    {
        FoliageQualitySlider->SetValue(GameUserSettings->GetFoliageQuality());
    }

    if (ShadingQualitySlider)
    {
        ShadingQualitySlider->SetValue(GameUserSettings->GetShadingQuality());
    }

    // Load VSync
    if (VSyncCheckBox)
    {
        VSyncCheckBox->SetIsChecked(GameUserSettings->IsVSyncEnabled());
    }

    // Load frame rate limit
    if (FrameRateLimitSlider)
    {
        float FrameRateLimit = GameUserSettings->GetFrameRateLimit();
        FrameRateLimitSlider->SetValue(FrameRateLimit);
    }

    // Set default audio values (for now just defaults)
    if (MasterVolumeSlider) MasterVolumeSlider->SetValue(1.0f);
    if (MusicVolumeSlider) MusicVolumeSlider->SetValue(0.8f);
    if (SFXVolumeSlider) SFXVolumeSlider->SetValue(1.0f);

    // Update all value text displays
    UpdateSliderValueTexts();
}

void USettingsWidget::UpdateSliderValueTexts()
{
    if (ViewDistanceValueText && ViewDistanceSlider)
    {
        ViewDistanceValueText->SetText(FText::FromString(QualityLevelToString(ViewDistanceSlider->GetValue())));
    }

    if (ShadowQualityValueText && ShadowQualitySlider)
    {
        ShadowQualityValueText->SetText(FText::FromString(QualityLevelToString(ShadowQualitySlider->GetValue())));
    }

    if (TextureQualityValueText && TextureQualitySlider)
    {
        TextureQualityValueText->SetText(FText::FromString(QualityLevelToString(TextureQualitySlider->GetValue())));
    }

    if (AntiAliasingValueText && AntiAliasingSlider)
    {
        AntiAliasingValueText->SetText(FText::FromString(QualityLevelToString(AntiAliasingSlider->GetValue())));
    }

    if (PostProcessValueText && PostProcessSlider)
    {
        PostProcessValueText->SetText(FText::FromString(QualityLevelToString(PostProcessSlider->GetValue())));
    }

    if (EffectsQualityValueText && EffectsQualitySlider)
    {
        EffectsQualityValueText->SetText(FText::FromString(QualityLevelToString(EffectsQualitySlider->GetValue())));
    }

    if (FoliageQualityValueText && FoliageQualitySlider)
    {
        FoliageQualityValueText->SetText(FText::FromString(QualityLevelToString(FoliageQualitySlider->GetValue())));
    }

    if (ShadingQualityValueText && ShadingQualitySlider)
    {
        ShadingQualityValueText->SetText(FText::FromString(QualityLevelToString(ShadingQualitySlider->GetValue())));
    }

    if (FrameRateLimitValueText && FrameRateLimitSlider)
    {
        float Value = FrameRateLimitSlider->GetValue();
        FString ValueText = Value <= 0 ? TEXT("Unlimited") : FString::Printf(TEXT("%.0f FPS"), Value);
        FrameRateLimitValueText->SetText(FText::FromString(ValueText));
    }

    if (MasterVolumeValueText && MasterVolumeSlider)
    {
        float Value = MasterVolumeSlider->GetValue();
        MasterVolumeValueText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), Value * 100)));
    }

    if (MusicVolumeValueText && MusicVolumeSlider)
    {
        float Value = MusicVolumeSlider->GetValue();
        MusicVolumeValueText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), Value * 100)));
    }

    if (SFXVolumeValueText && SFXVolumeSlider)
    {
        float Value = SFXVolumeSlider->GetValue();
        SFXVolumeValueText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), Value * 100)));
    }
}

FString USettingsWidget::QualityLevelToString(int32 QualityLevel)
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

// Button event handlers
void USettingsWidget::OnApplyClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Settings Applied!"));
    ApplyGraphicsSettings();
}

void USettingsWidget::OnResetToDefaultsClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Reset to Defaults!"));
    if (GameUserSettings)
    {
        GameUserSettings->SetToDefaults();
        LoadCurrentSettings();
    }
}

void USettingsWidget::OnBackClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Back to Main Menu!"));
    OnBackToMainMenu.Broadcast();
}

// Slider event handlers
void USettingsWidget::OnViewDistanceChanged(float Value)
{
    if (ViewDistanceValueText)
    {
        ViewDistanceValueText->SetText(FText::FromString(QualityLevelToString(Value)));
    }
}

void USettingsWidget::OnShadowQualityChanged(float Value)
{
    if (ShadowQualityValueText)
    {
        ShadowQualityValueText->SetText(FText::FromString(QualityLevelToString(Value)));
    }
}

void USettingsWidget::OnTextureQualityChanged(float Value)
{
    if (TextureQualityValueText)
    {
        TextureQualityValueText->SetText(FText::FromString(QualityLevelToString(Value)));
    }
}

void USettingsWidget::OnAntiAliasingChanged(float Value)
{
    if (AntiAliasingValueText)
    {
        AntiAliasingValueText->SetText(FText::FromString(QualityLevelToString(Value)));
    }
}

void USettingsWidget::OnPostProcessChanged(float Value)
{
    if (PostProcessValueText)
    {
        PostProcessValueText->SetText(FText::FromString(QualityLevelToString(Value)));
    }
}

void USettingsWidget::OnEffectsQualityChanged(float Value)
{
    if (EffectsQualityValueText)
    {
        EffectsQualityValueText->SetText(FText::FromString(QualityLevelToString(Value)));
    }
}

void USettingsWidget::OnFoliageQualityChanged(float Value)
{
    if (FoliageQualityValueText)
    {
        FoliageQualityValueText->SetText(FText::FromString(QualityLevelToString(Value)));
    }
}

void USettingsWidget::OnShadingQualityChanged(float Value)
{
    if (ShadingQualityValueText)
    {
        ShadingQualityValueText->SetText(FText::FromString(QualityLevelToString(Value)));
    }
}

void USettingsWidget::OnFrameRateLimitChanged(float Value)
{
    if (FrameRateLimitValueText)
    {
        FString ValueText = Value <= 0 ? TEXT("Unlimited") : FString::Printf(TEXT("%.0f FPS"), Value);
        FrameRateLimitValueText->SetText(FText::FromString(ValueText));
    }
}

void USettingsWidget::OnMasterVolumeChanged(float Value)
{
    if (MasterVolumeValueText)
    {
        MasterVolumeValueText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), Value * 100)));
    }
    UE_LOG(LogTemp, Warning, TEXT("Master Volume: %.2f"), Value);
}

void USettingsWidget::OnMusicVolumeChanged(float Value)
{
    if (MusicVolumeValueText)
    {
        MusicVolumeValueText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), Value * 100)));
    }
    UE_LOG(LogTemp, Warning, TEXT("Music Volume: %.2f"), Value);
}

void USettingsWidget::OnSFXVolumeChanged(float Value)
{
    if (SFXVolumeValueText)
    {
        SFXVolumeValueText->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), Value * 100)));
    }
    UE_LOG(LogTemp, Warning, TEXT("SFX Volume: %.2f"), Value);
}

// ComboBox event handlers
void USettingsWidget::OnResolutionSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    UE_LOG(LogTemp, Warning, TEXT("Resolution changed to: %s"), *SelectedItem);
}

void USettingsWidget::OnWindowModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    UE_LOG(LogTemp, Warning, TEXT("Window mode changed to: %s"), *SelectedItem);
}

void USettingsWidget::OnGraphicsQualitySelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (!GameUserSettings) return;
    
    int32 QualityLevel = 1; // Default to Medium
    
    if (SelectedItem == TEXT("Low")) QualityLevel = 0;
    else if (SelectedItem == TEXT("Medium")) QualityLevel = 1;
    else if (SelectedItem == TEXT("High")) QualityLevel = 2;
    else if (SelectedItem == TEXT("Epic")) QualityLevel = 3;
    else if (SelectedItem == TEXT("Cinematic")) QualityLevel = 4;

    // Set all quality settings to the selected level
    if (ViewDistanceSlider) ViewDistanceSlider->SetValue(QualityLevel);
    if (ShadowQualitySlider) ShadowQualitySlider->SetValue(QualityLevel);
    if (TextureQualitySlider) TextureQualitySlider->SetValue(QualityLevel);
    if (AntiAliasingSlider) AntiAliasingSlider->SetValue(QualityLevel);
    if (PostProcessSlider) PostProcessSlider->SetValue(QualityLevel);
    if (EffectsQualitySlider) EffectsQualitySlider->SetValue(QualityLevel);
    if (FoliageQualitySlider) FoliageQualitySlider->SetValue(QualityLevel);
    if (ShadingQualitySlider) ShadingQualitySlider->SetValue(QualityLevel);

    UpdateSliderValueTexts();
    
    UE_LOG(LogTemp, Warning, TEXT("Graphics quality preset changed to: %s"), *SelectedItem);
}

// CheckBox event handlers
void USettingsWidget::OnVSyncToggled(bool bIsChecked)
{
    UE_LOG(LogTemp, Warning, TEXT("VSync toggled: %s"), bIsChecked ? TEXT("ON") : TEXT("OFF"));
}

void USettingsWidget::ApplyGraphicsSettings()
{
    if (!GameUserSettings) return;

    // Apply resolution
    if (ResolutionComboBox)
    {
        FString SelectedResolution = ResolutionComboBox->GetSelectedOption();
        FString WidthStr, HeightStr;
        SelectedResolution.Split(TEXT("x"), &WidthStr, &HeightStr);
        
        int32 Width = FCString::Atoi(*WidthStr);
        int32 Height = FCString::Atoi(*HeightStr);
        
        GameUserSettings->SetScreenResolution(FIntPoint(Width, Height));
    }

    // Apply window mode
    if (WindowModeComboBox)
    {
        FString SelectedWindowMode = WindowModeComboBox->GetSelectedOption();
        if (SelectedWindowMode == TEXT("Fullscreen"))
        {
            GameUserSettings->SetFullscreenMode(EWindowMode::Fullscreen);
        }
        else if (SelectedWindowMode == TEXT("Windowed Fullscreen"))
        {
            GameUserSettings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
        }
        else if (SelectedWindowMode == TEXT("Windowed"))
        {
            GameUserSettings->SetFullscreenMode(EWindowMode::Windowed);
        }
    }

    // Apply individual quality settings
    if (ViewDistanceSlider)
    {
        GameUserSettings->SetViewDistanceQuality(ViewDistanceSlider->GetValue());
    }

    if (ShadowQualitySlider)
    {
        GameUserSettings->SetShadowQuality(ShadowQualitySlider->GetValue());
    }

    if (TextureQualitySlider)
    {
        GameUserSettings->SetTextureQuality(TextureQualitySlider->GetValue());
    }

    if (AntiAliasingSlider)
    {
        GameUserSettings->SetAntiAliasingQuality(AntiAliasingSlider->GetValue());
    }

    if (PostProcessSlider)
    {
        GameUserSettings->SetPostProcessingQuality(PostProcessSlider->GetValue());
    }

    if (EffectsQualitySlider)
    {
        GameUserSettings->SetVisualEffectQuality(EffectsQualitySlider->GetValue());
    }

    if (FoliageQualitySlider)
    {
        GameUserSettings->SetFoliageQuality(FoliageQualitySlider->GetValue());
    }

    if (ShadingQualitySlider)
    {
        GameUserSettings->SetShadingQuality(ShadingQualitySlider->GetValue());
    }

    // Apply VSync
    if (VSyncCheckBox)
    {
        GameUserSettings->SetVSyncEnabled(VSyncCheckBox->IsChecked());
    }

    // Apply frame rate limit
    if (FrameRateLimitSlider)
    {
        GameUserSettings->SetFrameRateLimit(FrameRateLimitSlider->GetValue());
    }

    // Apply and save settings
    GameUserSettings->ApplySettings(false);
    GameUserSettings->SaveSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("All graphics settings applied and saved!"));
}
