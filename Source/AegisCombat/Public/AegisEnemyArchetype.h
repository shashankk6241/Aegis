#pragma once

#include "CoreMinimal.h"
#include "AegisEnemyArchetype.generated.h"

/**
 * EAegisEnemyArchetype
 * Defines the combat role and behavioral template for enemy actors.
 * Affects default stat scaling, AI behavior, and visual treatment.
 */
UENUM(BlueprintType)
enum class EAegisEnemyArchetype : uint8
{
	Melee_Grunt     UMETA(DisplayName = "Melee Grunt"),
	Ranged_Caster   UMETA(DisplayName = "Ranged Caster"),
	Elite           UMETA(DisplayName = "Elite"),
	Boss            UMETA(DisplayName = "Boss")
};
