#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AegisPlayerController.generated.h"

class UInputMappingContext;

/**
 * AAegisPlayerController
 * Initializes Enhanced Input Subsystems and sets up combat mapping contexts.
 */
UCLASS()
class AEGISCOMBAT_API AAegisPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAegisPlayerController();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
};
