#include "AegisGameMode.h"
#include "AegisCombatCharacter.h"
#include "AegisPlayerController.h"

AAegisGameMode::AAegisGameMode()
{
	DefaultPawnClass = AAegisCombatCharacter::StaticClass();
	PlayerControllerClass = AAegisPlayerController::StaticClass();
}
