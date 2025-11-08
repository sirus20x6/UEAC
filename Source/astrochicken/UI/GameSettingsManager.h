// /thearray/git/UnrealEngine/astrochicken/Source/astrochicken/GameSettingsManager.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameSettingsManager.generated.h"

UCLASS(BlueprintType)
class ASTROCHICKEN_API UGameSettingsManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UGameSettingsManager();

    // Audio settings
    UPROPERTY(BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Audio Settings")
    float MusicVolume = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Audio Settings")
    float SFXVolume = 1.0f;

    // Gameplay settings
    UPROPERTY(BlueprintReadWrite, Category = "Gameplay Settings")
    float MouseSensitivity = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Gameplay Settings")
    bool bInvertMouseY = false;

    // Functions to save/load settings
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SaveSettings();

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void LoadSettings();

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void ResetToDefaults();

    // Static function to get the settings manager instance
    UFUNCTION(BlueprintCallable, Category = "Settings")
    static UGameSettingsManager* GetGameSettingsManager(UObject* WorldContext);

private:
    // File path for saving settings
    FString GetSettingsFilePath() const;
};
