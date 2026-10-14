#pragma once

#include "CoreMinimal.h"
#include "AegisLootDropInfo.generated.h"

/**
 * FAegisLootDropInfo
 * Defines a single loot table entry with drop probability and quantity range.
 */
USTRUCT(BlueprintType)
struct AEGISCOMBAT_API FAegisLootDropInfo
{
	GENERATED_BODY()

	/** Actor class to spawn on death (legacy or custom actor) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	TSubclassOf<AActor> ItemClass;

	/** RPG Item Definition to drop as an AAegisWorldPickup */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<class UAegisItemDefinitionDataAsset> ItemDefinition;

	/** Probability of this item dropping (0.0 - 1.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DropChance = 0.5f;

	/** Minimum quantity to drop */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "1"))
	int32 MinQuantity = 1;

	/** Maximum quantity to drop */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "1"))
	int32 MaxQuantity = 1;
};
