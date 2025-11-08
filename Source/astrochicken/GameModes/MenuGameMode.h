// MyProject/Public/GameModes/MenuGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MenuGameMode.generated.h"

// ADD THIS LINE
UCLASS() 
class ASTROCHICKEN_API AMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMenuGameMode();
};