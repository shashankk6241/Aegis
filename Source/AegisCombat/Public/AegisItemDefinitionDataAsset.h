#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AegisItemTypes.h"
#include "AegisItemDefinitionDataAsset.generated.h"

class AAegisWeapon;
class UGameplayAbility;
class UGameplayEffect;
class UTexture2D;
class USkeletalMesh;
class UStaticMesh;

/**
 * UAegisItemDefinitionDataAsset
 * Static template defining immutable properties, equipment slots, visual meshes,
 * and Gameplay Abilities / Effects for an item.
 */
UCLASS(BlueprintType, Blueprintable)
class AEGISCOMBAT_API UAegisItemDefinitionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAegisItemDefinitionDataAsset();

	// ==========================================
	// Basic Info & UI
	// ==========================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Identity")
	FName ItemId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Identity")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Identity", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|UI")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Classification")
	EAegisItemType ItemType = EAegisItemType::Consumable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Classification")
	EAegisItemRarity Rarity = EAegisItemRarity::Common;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Economy", meta = (ClampMin = "1"))
	int32 MaxStackSize = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Economy", meta = (ClampMin = "0"))
	int32 VendorValue = 10;

	// ==========================================
	// Equipment Configuration
	// ==========================================

	/** Equipment slot this item attaches to */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Equipment", meta = (EditCondition = "ItemType == EAegisItemType::Weapon || ItemType == EAegisItemType::Shield || ItemType == EAegisItemType::Armor"))
	EAegisEquipmentSlot TargetSlot = EAegisEquipmentSlot::None;

	/** Weapon actor class to spawn when equipped */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Equipment", meta = (EditCondition = "ItemType == EAegisItemType::Weapon || ItemType == EAegisItemType::Shield"))
	TSubclassOf<AAegisWeapon> WeaponClass;

	/** Mesh for armor piece */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Equipment", meta = (EditCondition = "ItemType == EAegisItemType::Armor"))
	TObjectPtr<USkeletalMesh> ArmorMesh;

	/** Socket on the character mesh to attach to */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Equipment")
	FName AttachSocket = FName("hand_r_socket");

	/** Custom Light Attack / Combo ability provided by this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Equipment|GAS")
	TSubclassOf<UGameplayAbility> GrantedMeleeAbility;

	/** Custom Heavy Attack ability provided by this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Equipment|GAS")
	TSubclassOf<UGameplayAbility> GrantedHeavyAbility;

	/** Custom Special / Weapon Art ability provided by this weapon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Equipment|GAS")
	TSubclassOf<UGameplayAbility> GrantedSpecialAbility;

	/** Passive abilities granted while equipped */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Equipment|GAS")
	TArray<TSubclassOf<UGameplayAbility>> GrantedPassiveAbilities;

	/** Stat-modifying Gameplay Effects applied while equipped (e.g. +Attack Power, +Poise) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Equipment|GAS")
	TArray<TSubclassOf<UGameplayEffect>> GrantedEquipEffects;

	// ==========================================
	// Consumable Configuration
	// ==========================================

	/** Ability triggered when consuming this item */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Consumable", meta = (EditCondition = "ItemType == EAegisItemType::Consumable"))
	TSubclassOf<UGameplayAbility> ConsumeAbility;

	/** Direct effect applied when consumed (e.g. instant heal) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Consumable", meta = (EditCondition = "ItemType == EAegisItemType::Consumable"))
	TSubclassOf<UGameplayEffect> ConsumeEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Consumable")
	bool bConsumeOnUse = true;

	// ==========================================
	// World Representation
	// ==========================================

	/** Static mesh for world pickups */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|World")
	TObjectPtr<UStaticMesh> WorldMesh;
};
