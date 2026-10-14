#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "ActiveGameplayEffectHandle.h"
#include "AegisItemTypes.h"
#include "AegisEquipmentComponent.generated.h"

class AAegisWeapon;
class AAegisCombatCharacter;
class UAegisInventoryComponent;
class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct AEGISCOMBAT_API FAegisSlotRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	FAegisItemInstance ItemInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<AAegisWeapon> SpawnedWeapon = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<USkeletalMeshComponent> SpawnedArmorMesh = nullptr;

	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;
	TArray<FActiveGameplayEffectHandle> ActiveEffectHandles;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipmentSlotChanged, EAegisEquipmentSlot, Slot, const FAegisItemInstance&, ItemInstance);

/**
 * UAegisEquipmentComponent
 * Manages character equipment slots, visual weapon/armor attachment,
 * and dynamic GAS ability/stat effect granting and revocation.
 */
UCLASS(ClassGroup = (Aegis), meta = (BlueprintSpawnableComponent))
class AEGISCOMBAT_API UAegisEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAegisEquipmentComponent();

	virtual void BeginPlay() override;

	// ==========================================
	// Operations
	// ==========================================

	/** Equip an item from the specified inventory slot */
	UFUNCTION(BlueprintCallable, Category = "Aegis|Equipment")
	bool EquipFromInventory(int32 InventorySlotIndex, EAegisEquipmentSlot TargetSlot = EAegisEquipmentSlot::None);

	/** Unequip the item in the specified slot back into inventory */
	UFUNCTION(BlueprintCallable, Category = "Aegis|Equipment")
	bool UnequipSlotToInventory(EAegisEquipmentSlot Slot);

	/** Directly equip an item instance (e.g. on character initialization) */
	UFUNCTION(BlueprintCallable, Category = "Aegis|Equipment")
	bool EquipItemInstance(const FAegisItemInstance& ItemInstance, EAegisEquipmentSlot TargetSlot = EAegisEquipmentSlot::None);

	// ==========================================
	// Queries
	// ==========================================

	UFUNCTION(BlueprintPure, Category = "Aegis|Equipment")
	FAegisItemInstance GetEquippedItem(EAegisEquipmentSlot Slot) const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Equipment")
	bool IsSlotEquipped(EAegisEquipmentSlot Slot) const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Equipment")
	AAegisWeapon* GetEquippedWeaponActor(EAegisEquipmentSlot Slot = EAegisEquipmentSlot::MainHand) const;

	// ==========================================
	// Delegates
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Aegis|Equipment")
	FOnEquipmentSlotChanged OnEquipmentSlotChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Equipment")
	TMap<EAegisEquipmentSlot, FAegisSlotRuntimeState> EquippedSlots;

	void ApplyItemGrants(EAegisEquipmentSlot Slot, FAegisSlotRuntimeState& State, AAegisCombatCharacter* Character, UAbilitySystemComponent* ASC);
	void RemoveItemGrants(EAegisEquipmentSlot Slot, FAegisSlotRuntimeState& State, AAegisCombatCharacter* Character, UAbilitySystemComponent* ASC);

	UAegisInventoryComponent* GetInventoryComponent() const;
};
