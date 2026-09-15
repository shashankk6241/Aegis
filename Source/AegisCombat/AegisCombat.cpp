#include "AegisCombat.h"
#include "Public/AegisGameplayTags.h"

void FAegisCombatModule::StartupModule()
{
	// Initialize native gameplay tags during engine startup
	FAegisGameplayTags::InitializeNativeGameplayTags();
}

void FAegisCombatModule::ShutdownModule()
{
}

IMPLEMENT_PRIMARY_GAME_MODULE(FAegisCombatModule, AegisCombat, "AegisCombat");
