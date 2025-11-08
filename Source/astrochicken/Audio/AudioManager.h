// /thearray/git/UnrealEngine/astrochicken/Source/astrochicken/Audio/AudioManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundClass.h"
#include "AudioManager.generated.h"

UCLASS()
class ASTROCHICKEN_API AAudioManager : public AActor
{
    GENERATED_BODY()
    
public:    
    AAudioManager();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;

    // Audio volume controls
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);
    
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMusicVolume(float Volume);
    
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetSFXVolume(float Volume);
    
    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetMasterVolume() const;
    
    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetMusicVolume() const;
    
    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetSFXVolume() const;
    
    // Audio muting
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterMuted(bool bMuted);
    
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMusicMuted(bool bMuted);
    
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetSFXMuted(bool bMuted);
    
    // Get singleton instance
    UFUNCTION(BlueprintCallable, Category = "Audio")
    static AAudioManager* GetInstance(UWorld* World);

protected:
    // Sound class references
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    USoundClass* MasterSoundClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    USoundClass* MusicSoundClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    USoundClass* SFXSoundClass;

private:
    // Volume levels (0.0 to 1.0)
    float MasterVolume;
    float MusicVolume;
    float SFXVolume;
    
    // Mute states
    bool bMasterMuted;
    bool bMusicMuted;
    bool bSFXMuted;
    
    // Singleton instance
    static AAudioManager* Instance;
    
    // Apply volume changes to sound classes
    void ApplyVolumeChanges();
    
    // Load settings from game user settings
    void LoadAudioSettings();
    
    // Save settings to game user settings
    void SaveAudioSettings();
};
