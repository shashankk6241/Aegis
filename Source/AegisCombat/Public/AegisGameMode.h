#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AegisGameMode.generated.h"

/**
 * AAegisGameMode
 * Core GameMode setting up default pawn and player controller.
 */
UCLASS()
class AEGISCOMBAT_API AAegisGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAegisGameMode();
};
