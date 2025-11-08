// /thearray/git/UnrealEngine/astrochicken/Source/astrochicken/GameSettingsManager.cpp

#include "GameSettingsManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

UGameSettingsManager::UGameSettingsManager()
{
    // Set default values
    MasterVolume = 1.0f;
    MusicVolume = 1.0f;
    SFXVolume = 1.0f;
    MouseSensitivity = 1.0f;
    bInvertMouseY = false;
}

UGameSettingsManager* UGameSettingsManager::GetGameSettingsManager(UObject* WorldContext)
{
    if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContext))
    {
        return GameInstance->GetSubsystem<UGameSettingsManager>();
    }
    return nullptr;
}

FString UGameSettingsManager::GetSettingsFilePath() const
{
    return FPaths::ProjectSavedDir() + TEXT("GameSettings.json");
}

void UGameSettingsManager::SaveSettings()
{
    // Create JSON object to store settings
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    
    // Audio settings
    JsonObject->SetNumberField(TEXT("MasterVolume"), MasterVolume);
    JsonObject->SetNumberField(TEXT("MusicVolume"), MusicVolume);
    JsonObject->SetNumberField(TEXT("SFXVolume"), SFXVolume);
    
    // Gameplay settings
    JsonObject->SetNumberField(TEXT("MouseSensitivity"), MouseSensitivity);
    JsonObject->SetBoolField(TEXT("InvertMouseY"), bInvertMouseY);

    // Convert to JSON string
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    // Save to file
    FString FilePath = GetSettingsFilePath();
    if (!FFileHelper::SaveStringToFile(OutputString, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save game settings to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Game settings saved to: %s"), *FilePath);
    }
}

void UGameSettingsManager::LoadSettings()
{
    FString FilePath = GetSettingsFilePath();
    FString JsonString;
    
    // Check if file exists and load it
    if (FFileHelper::LoadFileToString(JsonString, *FilePath))
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
        
        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            // Load audio settings
            if (JsonObject->HasField(TEXT("MasterVolume")))
                MasterVolume = JsonObject->GetNumberField(TEXT("MasterVolume"));
            if (JsonObject->HasField(TEXT("MusicVolume")))
                MusicVolume = JsonObject->GetNumberField(TEXT("MusicVolume"));
            if (JsonObject->HasField(TEXT("SFXVolume")))
                SFXVolume = JsonObject->GetNumberField(TEXT("SFXVolume"));
                
            // Load gameplay settings
            if (JsonObject->HasField(TEXT("MouseSensitivity")))
                MouseSensitivity = JsonObject->GetNumberField(TEXT("MouseSensitivity"));
            if (JsonObject->HasField(TEXT("InvertMouseY")))
                bInvertMouseY = JsonObject->GetBoolField(TEXT("InvertMouseY"));
                
            UE_LOG(LogTemp, Log, TEXT("Game settings loaded from: %s"), *FilePath);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to parse game settings JSON"));
            ResetToDefaults();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Game settings file not found, using defaults: %s"), *FilePath);
        ResetToDefaults();
    }
}

void UGameSettingsManager::ResetToDefaults()
{
    MasterVolume = 1.0f;
    MusicVolume = 1.0f;
    SFXVolume = 1.0f;
    MouseSensitivity = 1.0f;
    bInvertMouseY = false;
    
    SaveSettings();
    UE_LOG(LogTemp, Log, TEXT("Game settings reset to defaults"));
}
