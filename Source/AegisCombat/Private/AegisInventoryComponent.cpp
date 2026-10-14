#include "AegisInventoryComponent.h"
#include "AegisItemDefinitionDataAsset.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

UAegisInventoryComponent::UAegisInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	MaxSlots = 24;
}

void UAegisInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Slots.Num() != MaxSlots)
	{
		Slots.SetNum(MaxSlots);
	}
}

bool UAegisInventoryComponent::AddItem(UAegisItemDefinitionDataAsset* ItemDef, int32 Quantity)
{
	if (!ItemDef || Quantity <= 0) return false;

	if (Slots.Num() != MaxSlots)
	{
		Slots.SetNum(MaxSlots);
	}

	int32 Remainder = Quantity;

	// 1. First pass: Stack onto existing compatible items
	for (int32 i = 0; i < Slots.Num() && Remainder > 0; ++i)
	{
		if (Slots[i].IsValid() && Slots[i].ItemDefinition == ItemDef)
		{
			int32 SpaceInStack = ItemDef->MaxStackSize - Slots[i].Quantity;
			if (SpaceInStack > 0)
			{
				int32 ToAdd = FMath::Min(Remainder, SpaceInStack);
				Slots[i].Quantity += ToAdd;
				Remainder -= ToAdd;
			}
		}
	}

	// 2. Second pass: Fill empty slots
	for (int32 i = 0; i < Slots.Num() && Remainder > 0; ++i)
	{
		if (!Slots[i].IsValid())
		{
			int32 ToAdd = FMath::Min(Remainder, ItemDef->MaxStackSize);
			Slots[i] = FAegisItemInstance(ItemDef, ToAdd);
			Remainder -= ToAdd;
		}
	}

	int32 AddedCount = Quantity - Remainder;
	if (AddedCount > 0)
	{
		OnItemAdded.Broadcast(ItemDef, AddedCount);
		BroadcastUpdated();
	}

	return Remainder == 0;
}

bool UAegisInventoryComponent::AddItemInstance(const FAegisItemInstance& ItemInstance)
{
	if (!ItemInstance.IsValid()) return false;

	if (Slots.Num() != MaxSlots)
	{
		Slots.SetNum(MaxSlots);
	}

	// Try stack if possible
	if (ItemInstance.ItemDefinition->MaxStackSize > 1)
	{
		for (int32 i = 0; i < Slots.Num(); ++i)
		{
			if (Slots[i].CanStackWith(ItemInstance))
			{
				int32 Space = Slots[i].ItemDefinition->MaxStackSize - Slots[i].Quantity;
				if (Space >= ItemInstance.Quantity)
				{
					Slots[i].Quantity += ItemInstance.Quantity;
					OnItemAdded.Broadcast(ItemInstance.ItemDefinition, ItemInstance.Quantity);
					BroadcastUpdated();
					return true;
				}
			}
		}
	}

	// Otherwise find first empty slot
	int32 EmptyIndex = FindFirstEmptySlot();
	if (EmptyIndex != INDEX_NONE)
	{
		Slots[EmptyIndex] = ItemInstance;
		OnItemAdded.Broadcast(ItemInstance.ItemDefinition, ItemInstance.Quantity);
		BroadcastUpdated();
		return true;
	}

	return false;
}

bool UAegisInventoryComponent::RemoveItemFromSlot(int32 SlotIndex, int32 Quantity)
{
	if (!Slots.IsValidIndex(SlotIndex) || !Slots[SlotIndex].IsValid() || Quantity <= 0)
	{
		return false;
	}

	UAegisItemDefinitionDataAsset* Def = Slots[SlotIndex].ItemDefinition;
	int32 ActualRemove = FMath::Min(Quantity, Slots[SlotIndex].Quantity);

	Slots[SlotIndex].Quantity -= ActualRemove;
	if (Slots[SlotIndex].Quantity <= 0)
	{
		Slots[SlotIndex] = FAegisItemInstance();
	}

	OnItemRemoved.Broadcast(Def, ActualRemove);
	BroadcastUpdated();
	return true;
}

bool UAegisInventoryComponent::RemoveItemByDef(UAegisItemDefinitionDataAsset* ItemDef, int32 Quantity)
{
	if (!ItemDef || Quantity <= 0 || !HasItem(ItemDef, Quantity))
	{
		return false;
	}

	int32 RemainingToRemove = Quantity;
	for (int32 i = Slots.Num() - 1; i >= 0 && RemainingToRemove > 0; --i)
	{
		if (Slots[i].IsValid() && Slots[i].ItemDefinition == ItemDef)
		{
			int32 ToRemove = FMath::Min(RemainingToRemove, Slots[i].Quantity);
			Slots[i].Quantity -= ToRemove;
			RemainingToRemove -= ToRemove;

			if (Slots[i].Quantity <= 0)
			{
				Slots[i] = FAegisItemInstance();
			}
		}
	}

	OnItemRemoved.Broadcast(ItemDef, Quantity);
	BroadcastUpdated();
	return true;
}

bool UAegisInventoryComponent::MoveItem(int32 SourceSlot, int32 DestinationSlot)
{
	if (!Slots.IsValidIndex(SourceSlot) || !Slots.IsValidIndex(DestinationSlot) || SourceSlot == DestinationSlot)
	{
		return false;
	}

	if (!Slots[SourceSlot].IsValid()) return false;

	// Check if stackable
	if (Slots[DestinationSlot].IsValid() && Slots[DestinationSlot].CanStackWith(Slots[SourceSlot]))
	{
		int32 Space = Slots[DestinationSlot].ItemDefinition->MaxStackSize - Slots[DestinationSlot].Quantity;
		int32 Transfer = FMath::Min(Slots[SourceSlot].Quantity, Space);

		Slots[DestinationSlot].Quantity += Transfer;
		Slots[SourceSlot].Quantity -= Transfer;

		if (Slots[SourceSlot].Quantity <= 0)
		{
			Slots[SourceSlot] = FAegisItemInstance();
		}

		BroadcastUpdated();
		return true;
	}

	// Normal swap
	Slots.Swap(SourceSlot, DestinationSlot);
	BroadcastUpdated();
	return true;
}

bool UAegisInventoryComponent::SplitStack(int32 SourceSlot, int32 SplitAmount, int32 DestinationSlot)
{
	if (!Slots.IsValidIndex(SourceSlot) || !Slots.IsValidIndex(DestinationSlot) || SourceSlot == DestinationSlot)
	{
		return false;
	}

	if (!Slots[SourceSlot].IsValid() || SplitAmount <= 0 || Slots[SourceSlot].Quantity <= SplitAmount)
	{
		return false;
	}

	if (Slots[DestinationSlot].IsValid())
	{
		return false; // Destination must be empty
	}

	FAegisItemInstance NewInstance = Slots[SourceSlot];
	NewInstance.InstanceId = FGuid::NewGuid();
	NewInstance.Quantity = SplitAmount;

	Slots[SourceSlot].Quantity -= SplitAmount;
	Slots[DestinationSlot] = NewInstance;

	BroadcastUpdated();
	return true;
}

bool UAegisInventoryComponent::UseItem(int32 SlotIndex)
{
	if (!Slots.IsValidIndex(SlotIndex) || !Slots[SlotIndex].IsValid())
	{
		return false;
	}

	UAegisItemDefinitionDataAsset* Def = Slots[SlotIndex].ItemDefinition;
	if (!Def || Def->ItemType != EAegisItemType::Consumable)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (ASC)
	{
		// Apply Consume Effect if present
		if (Def->ConsumeEffect)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddSourceObject(GetOwner());
			ASC->ApplyGameplayEffectToSelf(Def->ConsumeEffect->GetDefaultObject<UGameplayEffect>(), 1.0f, Context);
		}

		// Activate Consume Ability if present
		if (Def->ConsumeAbility)
		{
			ASC->TryActivateAbilityByClass(Def->ConsumeAbility);
		}
	}

	if (Def->bConsumeOnUse)
	{
		RemoveItemFromSlot(SlotIndex, 1);
	}

	return true;
}

int32 UAegisInventoryComponent::GetItemCount(UAegisItemDefinitionDataAsset* ItemDef) const
{
	if (!ItemDef) return 0;

	int32 Total = 0;
	for (const FAegisItemInstance& Item : Slots)
	{
		if (Item.IsValid() && Item.ItemDefinition == ItemDef)
		{
			Total += Item.Quantity;
		}
	}
	return Total;
}

bool UAegisInventoryComponent::HasItem(UAegisItemDefinitionDataAsset* ItemDef, int32 Quantity) const
{
	return GetItemCount(ItemDef) >= Quantity;
}

FAegisItemInstance UAegisInventoryComponent::GetItemInSlot(int32 SlotIndex) const
{
	if (Slots.IsValidIndex(SlotIndex))
	{
		return Slots[SlotIndex];
	}
	return FAegisItemInstance();
}

int32 UAegisInventoryComponent::FindFirstEmptySlot() const
{
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (!Slots[i].IsValid())
		{
			return i;
		}
	}
	return INDEX_NONE;
}

void UAegisInventoryComponent::RestoreInventorySlots(const TArray<FAegisItemInstance>& InSlots)
{
	Slots = InSlots;
	if (Slots.Num() != MaxSlots)
	{
		Slots.SetNum(MaxSlots);
	}
	BroadcastUpdated();
}

void UAegisInventoryComponent::ClearInventory()
{
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		Slots[i] = FAegisItemInstance();
	}
	BroadcastUpdated();
}

void UAegisInventoryComponent::BroadcastUpdated()
{
	OnInventoryUpdated.Broadcast(Slots);
}
