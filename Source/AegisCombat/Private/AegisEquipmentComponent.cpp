#include "AegisEquipmentComponent.h"
#include "AegisItemDefinitionDataAsset.h"
#include "AegisInventoryComponent.h"
#include "AegisCombatCharacter.h"
#include "AegisWeapon.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"

UAegisEquipmentComponent::UAegisEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAegisEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
}

UAegisInventoryComponent* UAegisEquipmentComponent::GetInventoryComponent() const
{
	if (AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UAegisInventoryComponent>();
	}
	return nullptr;
}

bool UAegisEquipmentComponent::EquipFromInventory(int32 InventorySlotIndex, EAegisEquipmentSlot TargetSlot)
{
	UAegisInventoryComponent* Inventory = GetInventoryComponent();
	if (!Inventory) return false;

	FAegisItemInstance ItemToEquip = Inventory->GetItemInSlot(InventorySlotIndex);
	if (!ItemToEquip.IsValid()) return false;

	UAegisItemDefinitionDataAsset* Def = ItemToEquip.ItemDefinition;
	if (!Def) return false;

	// Determine destination slot
	EAegisEquipmentSlot FinalSlot = TargetSlot != EAegisEquipmentSlot::None ? TargetSlot : Def->TargetSlot;
	if (FinalSlot == EAegisEquipmentSlot::None) return false;

	// If slot already occupied, unequip first
	if (IsSlotEquipped(FinalSlot))
	{
		FAegisItemInstance OldItem = EquippedSlots[FinalSlot].ItemInstance;
		UnequipSlotToInventory(FinalSlot);
	}

	// Remove 1 from inventory slot
	Inventory->RemoveItemFromSlot(InventorySlotIndex, 1);

	// Equip the single item instance
	FAegisItemInstance SingleEquipInstance = ItemToEquip;
	SingleEquipInstance.Quantity = 1;

	return EquipItemInstance(SingleEquipInstance, FinalSlot);
}

bool UAegisEquipmentComponent::UnequipSlotToInventory(EAegisEquipmentSlot Slot)
{
	if (!IsSlotEquipped(Slot)) return false;

	UAegisInventoryComponent* Inventory = GetInventoryComponent();
	if (!Inventory || Inventory->IsFull()) return false;

	FAegisSlotRuntimeState& State = EquippedSlots[Slot];
	AAegisCombatCharacter* Character = Cast<AAegisCombatCharacter>(GetOwner());
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());

	// Remove grants
	RemoveItemGrants(Slot, State, Character, ASC);

	// Add item back to inventory
	FAegisItemInstance RecoveredItem = State.ItemInstance;
	EquippedSlots.Remove(Slot);

	Inventory->AddItemInstance(RecoveredItem);

	OnEquipmentSlotChanged.Broadcast(Slot, FAegisItemInstance());
	return true;
}

bool UAegisEquipmentComponent::EquipItemInstance(const FAegisItemInstance& ItemInstance, EAegisEquipmentSlot TargetSlot)
{
	if (!ItemInstance.IsValid()) return false;

	UAegisItemDefinitionDataAsset* Def = ItemInstance.ItemDefinition;
	if (!Def) return false;

	EAegisEquipmentSlot FinalSlot = TargetSlot != EAegisEquipmentSlot::None ? TargetSlot : Def->TargetSlot;
	if (FinalSlot == EAegisEquipmentSlot::None) return false;

	AAegisCombatCharacter* Character = Cast<AAegisCombatCharacter>(GetOwner());
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());

	// Clear existing slot if present
	if (EquippedSlots.Contains(FinalSlot))
	{
		RemoveItemGrants(FinalSlot, EquippedSlots[FinalSlot], Character, ASC);
		EquippedSlots.Remove(FinalSlot);
	}

	FAegisSlotRuntimeState NewState;
	NewState.ItemInstance = ItemInstance;

	ApplyItemGrants(FinalSlot, NewState, Character, ASC);
	EquippedSlots.Add(FinalSlot, NewState);

	OnEquipmentSlotChanged.Broadcast(FinalSlot, ItemInstance);
	return true;
}

void UAegisEquipmentComponent::ApplyItemGrants(EAegisEquipmentSlot Slot, FAegisSlotRuntimeState& State, AAegisCombatCharacter* Character, UAbilitySystemComponent* ASC)
{
	UAegisItemDefinitionDataAsset* Def = State.ItemInstance.ItemDefinition;
	if (!Def || !Character) return;

	// 1. Weapon spawning and mesh attachment
	if (Def->WeaponClass && GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AAegisWeapon* SpawnedWeapon = GetWorld()->SpawnActor<AAegisWeapon>(Def->WeaponClass, Character->GetActorTransform(), SpawnParams);
		if (SpawnedWeapon)
		{
			SpawnedWeapon->EquipToCharacter(Character, Def->AttachSocket);
			State.SpawnedWeapon = SpawnedWeapon;

			// If Main Hand weapon, update Character's active weapon pointer
			if (Slot == EAegisEquipmentSlot::MainHand)
			{
				Character->SetEquippedWeapon(SpawnedWeapon);
			}
		}
	}

	// 2. Armor skeletal mesh attachment
	if (Def->ArmorMesh && Character->GetMesh())
	{
		USkeletalMeshComponent* ArmorComp = NewObject<USkeletalMeshComponent>(Character);
		if (ArmorComp)
		{
			ArmorComp->SetSkeletalMesh(Def->ArmorMesh);
			ArmorComp->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale);
			ArmorComp->SetLeaderPoseComponent(Character->GetMesh());
			ArmorComp->RegisterComponent();
			State.SpawnedArmorMesh = ArmorComp;
		}
	}

	// 3. Grant GAS Abilities
	if (ASC && Character->HasAuthority())
	{
		// Weapon-specific attacks
		if (Def->GrantedMeleeAbility)
		{
			FGameplayAbilitySpec Spec(Def->GrantedMeleeAbility, 1);
			State.GrantedAbilityHandles.Add(ASC->GiveAbility(Spec));
			if (Slot == EAegisEquipmentSlot::MainHand)
			{
				Character->SetMeleeAbilityClass(Def->GrantedMeleeAbility);
			}
		}
		if (Def->GrantedHeavyAbility)
		{
			FGameplayAbilitySpec Spec(Def->GrantedHeavyAbility, 1);
			State.GrantedAbilityHandles.Add(ASC->GiveAbility(Spec));
			if (Slot == EAegisEquipmentSlot::MainHand)
			{
				Character->SetHeavyAttackAbilityClass(Def->GrantedHeavyAbility);
			}
		}
		if (Def->GrantedSpecialAbility)
		{
			FGameplayAbilitySpec Spec(Def->GrantedSpecialAbility, 1);
			State.GrantedAbilityHandles.Add(ASC->GiveAbility(Spec));
			if (Slot == EAegisEquipmentSlot::MainHand)
			{
				Character->SetSpecialAbilityClass(Def->GrantedSpecialAbility);
			}
		}

		// Passive Abilities
		for (const TSubclassOf<UGameplayAbility>& PassiveClass : Def->GrantedPassiveAbilities)
		{
			if (PassiveClass)
			{
				FGameplayAbilitySpec Spec(PassiveClass, 1);
				State.GrantedAbilityHandles.Add(ASC->GiveAbility(Spec));
			}
		}

		// 4. Apply Gameplay Effects (Stat bonuses)
		for (const TSubclassOf<UGameplayEffect>& EffectClass : Def->GrantedEquipEffects)
		{
			if (EffectClass)
			{
				FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
				Context.AddSourceObject(Character);
				FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectToSelf(EffectClass->GetDefaultObject<UGameplayEffect>(), 1.0f, Context);
				if (Handle.IsValid())
				{
					State.ActiveEffectHandles.Add(Handle);
				}
			}
		}
	}
}

void UAegisEquipmentComponent::RemoveItemGrants(EAegisEquipmentSlot Slot, FAegisSlotRuntimeState& State, AAegisCombatCharacter* Character, UAbilitySystemComponent* ASC)
{
	// 1. Destroy spawned weapon
	if (State.SpawnedWeapon)
	{
		if (Character && Character->GetEquippedWeapon() == State.SpawnedWeapon)
		{
			Character->SetEquippedWeapon(nullptr);
		}
		State.SpawnedWeapon->Destroy();
		State.SpawnedWeapon = nullptr;
	}

	// 2. Destroy armor mesh
	if (State.SpawnedArmorMesh)
	{
		State.SpawnedArmorMesh->DestroyComponent();
		State.SpawnedArmorMesh = nullptr;
	}

	// 3. Clear GAS abilities
	if (ASC && Character && Character->HasAuthority())
	{
		for (const FGameplayAbilitySpecHandle& Handle : State.GrantedAbilityHandles)
		{
			ASC->ClearAbility(Handle);
		}
		State.GrantedAbilityHandles.Empty();

		// 4. Remove active gameplay effects
		for (const FActiveGameplayEffectHandle& Handle : State.ActiveEffectHandles)
		{
			ASC->RemoveActiveGameplayEffect(Handle);
		}
		State.ActiveEffectHandles.Empty();
	}
}

FAegisItemInstance UAegisEquipmentComponent::GetEquippedItem(EAegisEquipmentSlot Slot) const
{
	if (const FAegisSlotRuntimeState* State = EquippedSlots.Find(Slot))
	{
		return State->ItemInstance;
	}
	return FAegisItemInstance();
}

bool UAegisEquipmentComponent::IsSlotEquipped(EAegisEquipmentSlot Slot) const
{
	return EquippedSlots.Contains(Slot) && EquippedSlots[Slot].ItemInstance.IsValid();
}

AAegisWeapon* UAegisEquipmentComponent::GetEquippedWeaponActor(EAegisEquipmentSlot Slot) const
{
	if (const FAegisSlotRuntimeState* State = EquippedSlots.Find(Slot))
	{
		return State->SpawnedWeapon;
	}
	return nullptr;
}
