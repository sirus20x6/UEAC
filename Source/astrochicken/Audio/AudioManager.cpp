// /thearray/git/UnrealEngine/astrochicken/Source/astrochicken/Audio/AudioManager.cpp

#include "AudioManager.h"
#include "Sound/SoundClass.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"

// Initialize static instance
AAudioManager* AAudioManager::Instance = nullptr;

AAudioManager::AAudioManager()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Initialize default values
    MasterVolume = 1.0f;
    MusicVolume = 1.0f;
    SFXVolume = 1.0f;
    
    bMasterMuted = false;
    bMusicMuted = false;
    bSFXMuted = false;
    
    // Set as singleton instance
    Instance = this;
}

void AAudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    LoadAudioSettings();
    ApplyVolumeChanges();
}

void AAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

AAudioManager* AAudioManager::GetInstance(UWorld* World)
{
    if (!Instance && World)
    {
        // Try to find existing instance in world
        for (TActorIterator<AAudioManager> ActorItr(World); ActorItr; ++ActorItr)
        {
            Instance = *ActorItr;
            break;
        }
        
        // If no instance found, create one
        if (!Instance)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Name = TEXT("AudioManager");
            Instance = World->SpawnActor<AAudioManager>(AAudioManager::StaticClass(), SpawnParams);
        }
    }
    
    return Instance;
}

void AAudioManager::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    ApplyVolumeChanges();
    SaveAudioSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Master Volume set to: %f"), MasterVolume);
}

void AAudioManager::SetMusicVolume(float Volume)
{
    MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    ApplyVolumeChanges();
    SaveAudioSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Music Volume set to: %f"), MusicVolume);
}

void AAudioManager::SetSFXVolume(float Volume)
{
    SFXVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    ApplyVolumeChanges();
    SaveAudioSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("SFX Volume set to: %f"), SFXVolume);
}

float AAudioManager::GetMasterVolume() const
{
    return MasterVolume;
}

float AAudioManager::GetMusicVolume() const
{
    return MusicVolume;
}

float AAudioManager::GetSFXVolume() const
{
    return SFXVolume;
}

void AAudioManager::SetMasterMuted(bool bMuted)
{
    bMasterMuted = bMuted;
    ApplyVolumeChanges();
    SaveAudioSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Master Audio %s"), bMuted ? TEXT("MUTED") : TEXT("UNMUTED"));
}

void AAudioManager::SetMusicMuted(bool bMuted)
{
    bMusicMuted = bMuted;
    ApplyVolumeChanges();
    SaveAudioSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Music Audio %s"), bMuted ? TEXT("MUTED") : TEXT("UNMUTED"));
}

void AAudioManager::SetSFXMuted(bool bMuted)
{
    bSFXMuted = bMuted;
    ApplyVolumeChanges();
    SaveAudioSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("SFX Audio %s"), bMuted ? TEXT("MUTED") : TEXT("UNMUTED"));
}

void AAudioManager::ApplyVolumeChanges()
{
    if (MasterSoundClass)
    {
        float FinalMasterVolume = bMasterMuted ? 0.0f : MasterVolume;
        MasterSoundClass->Properties.Volume = FinalMasterVolume;
    }
    
    if (MusicSoundClass)
    {
        float FinalMusicVolume = (bMasterMuted || bMusicMuted) ? 0.0f : (MasterVolume * MusicVolume);
        MusicSoundClass->Properties.Volume = FinalMusicVolume;
    }
    
    if (SFXSoundClass)
    {
        float FinalSFXVolume = (bMasterMuted || bSFXMuted) ? 0.0f : (MasterVolume * SFXVolume);
        SFXSoundClass->Properties.Volume = FinalSFXVolume;
    }
}

void AAudioManager::LoadAudioSettings()
{
    // For now, we'll use simple default values
    // In a full implementation, you'd load these from GameUserSettings or a config file
    MasterVolume = 1.0f;
    MusicVolume = 0.8f;
    SFXVolume = 1.0f;
    
    bMasterMuted = false;
    bMusicMuted = false;
    bSFXMuted = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Audio settings loaded: Master=%f, Music=%f, SFX=%f"), 
           MasterVolume, MusicVolume, SFXVolume);
}

void AAudioManager::SaveAudioSettings()
{
    // For now, just log the save attempt
    // In a full implementation, you'd save these to GameUserSettings or a config file
    UE_LOG(LogTemp, Warning, TEXT("Audio settings saved: Master=%f, Music=%f, SFX=%f"), 
           MasterVolume, MusicVolume, SFXVolume);
}
