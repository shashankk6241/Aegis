#include "AegisPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

AAegisPlayerController::AAegisPlayerController()
{
}

void AAegisPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}
