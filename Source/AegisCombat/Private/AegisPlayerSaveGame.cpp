#include "AegisPlayerSaveGame.h"

UAegisPlayerSaveGame::UAegisPlayerSaveGame()
{
	SlotName = TEXT("AegisPlayerSave_0");
	UserIndex = 0;
	PlayerData.SaveVersion = 1;
	PlayerData.Timestamp = FDateTime::Now();
}
