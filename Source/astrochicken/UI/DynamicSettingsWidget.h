// /thearray/git/UnrealEngine/astrochicken/Source/astrochicken/UI/DynamicSettingsWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "Components/SlateWrapperTypes.h"
#include "DynamicSettingsWidget.generated.h"

class UButton;
class UComboBoxString;
class USlider;
class UCheckBox;
class UTextBlock;
class UVerticalBox;
class UHorizontalBox;
class UScrollBox;
class UBorder;
class USpacer;
class UCanvasPanel;
class UOverlay;

USTRUCT(BlueprintType)
struct FSettingsControl
{
    GENERATED_BODY()

    UPROPERTY()
    FString Type;
    
    UPROPERTY()
    FString Name;
    
    UPROPERTY()
    FString Label;
    
    UPROPERTY()
    TArray<FString> Options;
    
    UPROPERTY()
    float Min = 0.0f;
    
    UPROPERTY()
    float Max = 1.0f;
    
    UPROPERTY()
    float Step = 0.1f;
    
    UPROPERTY()
    bool ShowValue = false;
    
    UPROPERTY()
    float ValueMultiplier = 1.0f;
    
    UPROPERTY()
    FString ValueSuffix;
    
    UPROPERTY()
    FString ZeroText;
    
    UPROPERTY()
    FString Style;
};

USTRUCT(BlueprintType)
struct FSettingsSection
{
    GENERATED_BODY()

    UPROPERTY()
    FString Name;
    
    UPROPERTY()
    FString Type;
    
    UPROPERTY()
    TArray<FSettingsControl> Controls;
};

USTRUCT(BlueprintType)
struct FSettingsLayout
{
    GENERATED_BODY()

    UPROPERTY()
    FString Title;
    
    UPROPERTY()
    TArray<FSettingsSection> Sections;
    
    UPROPERTY()
    TArray<FSettingsControl> ActionButtons;
};

// Layout configuration structure for responsive design
USTRUCT(BlueprintType)
struct FUILayoutConfig
{
    GENERATED_BODY()

    UPROPERTY()
    float MinWindowWidth = 1280.0f;
    
    UPROPERTY()
    float MinWindowHeight = 720.0f;
    
    UPROPERTY()
    float MaxContentWidth = 800.0f;
    
    UPROPERTY()
    float ControlHeight = 40.0f;
    
    UPROPERTY()
    float ControlSpacing = 12.0f;
    
    UPROPERTY()
    float SectionSpacing = 24.0f;
    
    UPROPERTY()
    float SidePadding = 32.0f;
    
    UPROPERTY()
    float TopBottomPadding = 24.0f;
    
    UPROPERTY()
    float LabelWidthRatio = 0.4f;
    
    UPROPERTY()
    float ControlWidthRatio = 0.5f;
    
    UPROPERTY()
    float ValueDisplayWidth = 80.0f;
};

UCLASS()
class ASTROCHICKEN_API UDynamicSettingsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // This will be called by MainMenuWidget to populate the container
    void PopulateContainer(UVerticalBox* Container);

protected:
    virtual void NativeConstruct() override;

private:
    // Settings data
    UPROPERTY()
    FSettingsLayout SettingsLayout;
    
    // Widget references for binding
    UPROPERTY()
    TMap<FString, UWidget*> WidgetMap;
    
    // Game User Settings reference
    UPROPERTY()
    UGameUserSettings* GameUserSettings;

    // Reference to the container we're populating
    UPROPERTY()
    UVerticalBox* TargetContainer;
    
    // Layout configuration based on screen resolution
    UPROPERTY()
    FUILayoutConfig LayoutConfig;
    
    // Flag to track if mouse recalibration is needed after resolution change
    bool bNeedsMouseRecalibration = false;
    
    // Flag to track automatic window mode toggle for mouse fix
    bool bAutoToggleInProgress = false;

    // JSON loading and parsing
    bool LoadSettingsLayout();
    void ParseJsonToLayout(const FString& JsonString);
    
    // Layout and responsiveness
    void CalculateLayoutConfig();
    FVector2D GetCurrentViewportSize();
    float GetScaledSize(float BaseSize);
    
    // UI Generation - now populates external container with improved styling
    void GenerateSettingsUI();
    void CreateFallbackUI();
    
    // Enhanced widget creation with proper styling
    UWidget* CreateControlWidget(const FSettingsControl& Control);
    UWidget* CreateSliderWithLabel(const FSettingsControl& Control);
    UWidget* CreateComboBoxWithLabel(const FSettingsControl& Control);
    UWidget* CreateCheckBoxWithLabel(const FSettingsControl& Control);
    UWidget* CreateButton(const FSettingsControl& Control);
    UWidget* CreateSectionHeader(const FString& SectionName);
    UWidget* CreateActionButtonsRow();
    
    // Styling helper functions
    UBorder* CreateStyledBorder();
    UBorder* CreateControlContainer();
    UTextBlock* CreateStyledLabel(const FString& LabelText);
    UTextBlock* CreateStyledValueDisplay();
    UTextBlock* CreateStyledSectionHeader(const FString& SectionName);
    USpacer* CreateVerticalSpacer(float Height);
    USpacer* CreateHorizontalSpacer(float Width);
    
    // Widget styling functions
    void ApplyButtonStyle(UButton* Button, const FString& StyleType);
    void ApplySliderStyle(USlider* Slider);
    void ApplyComboBoxStyle(UComboBoxString* ComboBox);
    void ApplyCheckBoxStyle(UCheckBox* CheckBox);
    void ApplyTextStyle(UTextBlock* TextBlock, const FString& StyleType);
    
    // Event handlers
    UFUNCTION()
    void OnSliderValueChanged(float Value);
    
    UFUNCTION()
    void OnComboBoxSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    
    UFUNCTION()
    void OnCheckBoxStateChanged(bool bIsChecked);
    
    UFUNCTION()
    void OnButtonClicked();
    
    UFUNCTION()
    void OnApplySettingsClicked();
    
    UFUNCTION()
    void OnResetToDefaultsClicked();
    
    // Settings management
    void LoadCurrentSettings();
    void ApplySettings();
    void ResetToDefaults();
    void UpdateValueDisplay(const FString& SliderName, float Value, const FSettingsControl& Control);
    void RecalibrateMouseInput();
    void AutoToggleWindowModeForMouseFix();
    
    // Helper functions
    FString QualityLevelToString(int32 QualityLevel);
    FString FormatSliderValue(float Value, const FSettingsControl& Control);
    UWidget* GetControlFromWidget(UWidget* SenderWidget);
    void PopulateComboBoxOptions(UComboBoxString* ComboBox, const FSettingsControl& Control);
    void ApplyGraphicsQualitySettings();
    void ApplyAudioSettings();
    
    // Color and styling constants
    FLinearColor GetPrimaryColor() { return FLinearColor(0.1f, 0.4f, 0.8f, 1.0f); }
    FLinearColor GetSecondaryColor() { return FLinearColor(0.2f, 0.2f, 0.2f, 1.0f); }
    FLinearColor GetBackgroundColor() { return FLinearColor(0.05f, 0.05f, 0.05f, 0.9f); }
    FLinearColor GetTextColor() { return FLinearColor(0.9f, 0.9f, 0.9f, 1.0f); }
    FLinearColor GetAccentColor() { return FLinearColor(0.3f, 0.6f, 1.0f, 1.0f); }

public:
    // Delegate for back navigation
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackToMainMenu);
    UPROPERTY(BlueprintAssignable)
    FOnBackToMainMenu OnBackToMainMenu;
};
