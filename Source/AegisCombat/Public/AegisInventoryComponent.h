#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AegisItemTypes.h"
#include "AegisInventoryComponent.generated.h"

class UAegisItemDefinitionDataAsset;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryUpdated, const TArray<FAegisItemInstance>&, Slots);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemAdded, UAegisItemDefinitionDataAsset*, ItemDef, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemoved, UAegisItemDefinitionDataAsset*, ItemDef, int32, Quantity);

/**
 * UAegisInventoryComponent
 * Server-authoritative slot-based inventory component managing item stacks,
 * capacity checks, moving, splitting, and consumable usage.
 */
UCLASS(ClassGroup = (Aegis), meta = (BlueprintSpawnableComponent))
class AEGISCOMBAT_API UAegisInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAegisInventoryComponent();

	virtual void BeginPlay() override;

	// ==========================================
	// Configuration
	// ==========================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Inventory", meta = (ClampMin = "1", ClampMax = "120"))
	int32 MaxSlots = 24;

	// ==========================================
	// Operations
	// ==========================================

	/** Add an item by definition. Automatically stacks or occupies empty slots. */
	UFUNCTION(BlueprintCallable, Category = "Aegis|Inventory")
	bool AddItem(UAegisItemDefinitionDataAsset* ItemDef, int32 Quantity = 1);

	/** Add an exact item instance (preserves upgrade level and instance ID) */
	UFUNCTION(BlueprintCallable, Category = "Aegis|Inventory")
	bool AddItemInstance(const FAegisItemInstance& ItemInstance);

	/** Remove quantity from a specific slot index */
	UFUNCTION(BlueprintCallable, Category = "Aegis|Inventory")
	bool RemoveItemFromSlot(int32 SlotIndex, int32 Quantity = 1);

	/** Remove quantity of an item matching the definition across all slots */
	UFUNCTION(BlueprintCallable, Category = "Aegis|Inventory")
	bool RemoveItemByDef(UAegisItemDefinitionDataAsset* ItemDef, int32 Quantity = 1);

	/** Swap or merge slots */
	UFUNCTION(BlueprintCallable, Category = "Aegis|Inventory")
	bool MoveItem(int32 SourceSlot, int32 DestinationSlot);

	/** Split a portion of a stack into another slot */
	UFUNCTION(BlueprintCallable, Category = "Aegis|Inventory")
	bool SplitStack(int32 SourceSlot, int32 SplitAmount, int32 DestinationSlot);

	/** Use a consumable item in the given slot */
	UFUNCTION(BlueprintCallable, Category = "Aegis|Inventory")
	bool UseItem(int32 SlotIndex);

	// ==========================================
	// Queries
	// ==========================================

	UFUNCTION(BlueprintPure, Category = "Aegis|Inventory")
	int32 GetItemCount(UAegisItemDefinitionDataAsset* ItemDef) const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Inventory")
	bool HasItem(UAegisItemDefinitionDataAsset* ItemDef, int32 Quantity = 1) const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Inventory")
	FAegisItemInstance GetItemInSlot(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Inventory")
	const TArray<FAegisItemInstance>& GetAllSlots() const { return Slots; }

	UFUNCTION(BlueprintPure, Category = "Aegis|Inventory")
	int32 FindFirstEmptySlot() const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Inventory")
	bool IsFull() const { return FindFirstEmptySlot() == INDEX_NONE; }

	UFUNCTION(BlueprintCallable, Category = "Aegis|Inventory")
	void ClearInventory();

	// ==========================================
	// Delegates
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Aegis|Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Aegis|Inventory")
	FOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "Aegis|Inventory")
	FOnItemRemoved OnItemRemoved;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Inventory")
	TArray<FAegisItemInstance> Slots;

	void BroadcastUpdated();
};
