#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AegisItemTypes.generated.h"

class UAegisItemDefinitionDataAsset;

/**
 * Categorization of items in the Aegis RPG system.
 */
UENUM(BlueprintType)
enum class EAegisItemType : uint8
{
	None			UMETA(DisplayName = "None"),
	Weapon			UMETA(DisplayName = "Weapon"),
	Shield			UMETA(DisplayName = "Shield"),
	Armor			UMETA(DisplayName = "Armor"),
	Consumable		UMETA(DisplayName = "Consumable"),
	Material		UMETA(DisplayName = "Material"),
	KeyItem			UMETA(DisplayName = "Key Item")
};

/**
 * Visual and quality tier for items.
 */
UENUM(BlueprintType)
enum class EAegisItemRarity : uint8
{
	Common			UMETA(DisplayName = "Common"),
	Uncommon		UMETA(DisplayName = "Uncommon"),
	Rare			UMETA(DisplayName = "Rare"),
	Epic			UMETA(DisplayName = "Epic"),
	Legendary		UMETA(DisplayName = "Legendary")
};

/**
 * Dedicated equipment slots on the player character.
 */
UENUM(BlueprintType)
enum class EAegisEquipmentSlot : uint8
{
	None			UMETA(DisplayName = "None"),
	MainHand		UMETA(DisplayName = "Main Hand"),
	OffHand			UMETA(DisplayName = "Off Hand"),
	Head			UMETA(DisplayName = "Head"),
	Chest			UMETA(DisplayName = "Chest"),
	Hands			UMETA(DisplayName = "Hands"),
	Legs			UMETA(DisplayName = "Legs"),
	Ring1			UMETA(DisplayName = "Ring 1"),
	Ring2			UMETA(DisplayName = "Ring 2")
};

/**
 * Runtime instance of an item in the player's inventory or equipment.
 */
USTRUCT(BlueprintType)
struct AEGISCOMBAT_API FAegisItemInstance
{
	GENERATED_BODY()

	/** Unique identifier for this specific instance (safe for persistence/saving) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FGuid InstanceId;

	/** Static template defining item attributes, mesh, and abilities */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UAegisItemDefinitionDataAsset> ItemDefinition = nullptr;

	/** Current stack quantity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Quantity = 1;

	/** Upgrade/reinforcement level (e.g., +1, +5) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 UpgradeLevel = 0;

	FAegisItemInstance()
		: InstanceId(FGuid::NewGuid())
		, ItemDefinition(nullptr)
		, Quantity(1)
		, UpgradeLevel(0)
	{}

	FAegisItemInstance(UAegisItemDefinitionDataAsset* InDef, int32 InQuantity = 1)
		: InstanceId(FGuid::NewGuid())
		, ItemDefinition(InDef)
		, Quantity(InQuantity)
		, UpgradeLevel(0)
	{}

	bool IsValid() const
	{
		return ItemDefinition != nullptr && Quantity > 0;
	}

	bool CanStackWith(const FAegisItemInstance& Other) const;
};
