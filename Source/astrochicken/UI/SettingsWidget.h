// /thearray/git/UnrealEngine/astrochicken/Source/astrochicken/UI/SettingsWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "SettingsWidget.generated.h"

class UButton;
class UComboBoxString;
class USlider;
class UCheckBox;
class UTextBlock;

UCLASS()
class ASTROCHICKEN_API USettingsWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

private:
    // UI Elements - Graphics
    UPROPERTY(meta = (BindWidget))
    UComboBoxString* ResolutionComboBox;
    
    UPROPERTY(meta = (BindWidget))
    UComboBoxString* WindowModeComboBox;
    
    UPROPERTY(meta = (BindWidget))
    UComboBoxString* GraphicsQualityComboBox;
    
    UPROPERTY(meta = (BindWidget))
    USlider* ViewDistanceSlider;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ViewDistanceValueText;
    
    UPROPERTY(meta = (BindWidget))
    USlider* ShadowQualitySlider;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ShadowQualityValueText;
    
    UPROPERTY(meta = (BindWidget))
    USlider* TextureQualitySlider;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TextureQualityValueText;
    
    UPROPERTY(meta = (BindWidget))
    USlider* AntiAliasingSlider;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* AntiAliasingValueText;
    
    UPROPERTY(meta = (BindWidget))
    USlider* PostProcessSlider;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* PostProcessValueText;
    
    UPROPERTY(meta = (BindWidget))
    USlider* EffectsQualitySlider;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* EffectsQualityValueText;
    
    UPROPERTY(meta = (BindWidget))
    USlider* FoliageQualitySlider;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* FoliageQualityValueText;
    
    UPROPERTY(meta = (BindWidget))
    USlider* ShadingQualitySlider;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ShadingQualityValueText;
    
    UPROPERTY(meta = (BindWidget))
    UCheckBox* VSyncCheckBox;
    
    UPROPERTY(meta = (BindWidget))
    USlider* FrameRateLimitSlider;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* FrameRateLimitValueText;

    // UI Elements - Audio
    UPROPERTY(meta = (BindWidget))
    USlider* MasterVolumeSlider;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* MasterVolumeValueText;
    
    UPROPERTY(meta = (BindWidget))
    USlider* MusicVolumeSlider;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* MusicVolumeValueText;
    
    UPROPERTY(meta = (BindWidget))
    USlider* SFXVolumeSlider;
    
    UPROPERTY(meta = (BindWidget))
    UTextBlock* SFXVolumeValueText;

    // Control buttons
    UPROPERTY(meta = (BindWidget))
    UButton* ApplyButton;
    
    UPROPERTY(meta = (BindWidget))
    UButton* ResetToDefaultsButton;
    
    UPROPERTY(meta = (BindWidget))
    UButton* BackButton;

    // Game User Settings reference
    UPROPERTY()
    UGameUserSettings* GameUserSettings;

    // Button event handlers
    UFUNCTION()
    void OnApplyClicked();
    
    UFUNCTION()
    void OnResetToDefaultsClicked();
    
    UFUNCTION()
    void OnBackClicked();

    // Slider event handlers
    UFUNCTION()
    void OnViewDistanceChanged(float Value);
    
    UFUNCTION()
    void OnShadowQualityChanged(float Value);
    
    UFUNCTION()
    void OnTextureQualityChanged(float Value);
    
    UFUNCTION()
    void OnAntiAliasingChanged(float Value);
    
    UFUNCTION()
    void OnPostProcessChanged(float Value);
    
    UFUNCTION()
    void OnEffectsQualityChanged(float Value);
    
    UFUNCTION()
    void OnFoliageQualityChanged(float Value);
    
    UFUNCTION()
    void OnShadingQualityChanged(float Value);
    
    UFUNCTION()
    void OnFrameRateLimitChanged(float Value);
    
    UFUNCTION()
    void OnMasterVolumeChanged(float Value);
    
    UFUNCTION()
    void OnMusicVolumeChanged(float Value);
    
    UFUNCTION()
    void OnSFXVolumeChanged(float Value);

    // ComboBox event handlers
    UFUNCTION()
    void OnResolutionSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    
    UFUNCTION()
    void OnWindowModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    
    UFUNCTION()
    void OnGraphicsQualitySelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    // CheckBox event handlers
    UFUNCTION()
    void OnVSyncToggled(bool bIsChecked);

    // Helper functions
    void PopulateResolutionComboBox();
    void PopulateWindowModeComboBox();
    void PopulateGraphicsQualityComboBox();
    void LoadCurrentSettings();
    void ApplyGraphicsSettings();
    void UpdateSliderValueTexts();
    FString QualityLevelToString(int32 QualityLevel);

public:
    // Delegate to notify when we want to go back to main menu
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackToMainMenu);
    UPROPERTY(BlueprintAssignable)
    FOnBackToMainMenu OnBackToMainMenu;
};
