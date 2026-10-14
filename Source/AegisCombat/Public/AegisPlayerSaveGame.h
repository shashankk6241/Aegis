#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "AegisItemTypes.h"
#include "AegisPlayerSaveGame.generated.h"

/**
 * Snapshot of all persistent player RPG data for saving and loading.
 */
USTRUCT(BlueprintType)
struct AEGISCOMBAT_API FAegisPlayerSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int32 SaveVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	FDateTime Timestamp = FDateTime::Now();

	// Progression & Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int32 PlayerLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int32 CurrentXP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int32 UnspentAttributePoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int32 UnspentSkillPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int32 Vigor = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int32 Endurance = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int32 Strength = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int32 Resilience = 10;

	// Inventory & Equipment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	TArray<FAegisItemInstance> InventoryItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	TMap<EAegisEquipmentSlot, FAegisItemInstance> EquippedItems;

	// Skill Tree Unlocks (NodeId -> Rank)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	TMap<FName, int32> UnlockedSkills;

	// Checkpoint Transform
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	FVector SavedLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	FRotator SavedRotation = FRotator::ZeroRotator;
};

/**
 * UAegisPlayerSaveGame
 * Versioned SaveGame object storing the complete state of the player character.
 */
UCLASS()
class AEGISCOMBAT_API UAegisPlayerSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UAegisPlayerSaveGame();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString SlotName = TEXT("AegisPlayerSave_0");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 UserIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FAegisPlayerSaveData PlayerData;
};
