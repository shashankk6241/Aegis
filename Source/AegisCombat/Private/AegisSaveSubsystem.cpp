#include "AegisSaveSubsystem.h"
#include "AegisCombatCharacter.h"
#include "AegisPlayerProgressionComponent.h"
#include "AegisInventoryComponent.h"
#include "AegisEquipmentComponent.h"
#include "AegisSkillTreeComponent.h"
#include "Kismet/GameplayStatics.h"

UAegisSaveSubsystem::UAegisSaveSubsystem()
{
}

bool UAegisSaveSubsystem::DoesSaveExist(const FString& SlotName, int32 UserIndex) const
{
	return UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
}

UAegisPlayerSaveGame* UAegisSaveSubsystem::GetSaveGameData(const FString& SlotName, int32 UserIndex) const
{
	if (!DoesSaveExist(SlotName, UserIndex)) return nullptr;
	return Cast<UAegisPlayerSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
}

bool UAegisSaveSubsystem::DeleteSave(const FString& SlotName, int32 UserIndex)
{
	return UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
}

bool UAegisSaveSubsystem::SaveGame(AAegisCombatCharacter* Character, const FString& SlotName, int32 UserIndex)
{
	if (!Character)
	{
		OnGameSaved.Broadcast(SlotName, false);
		return false;
	}

	UAegisPlayerSaveGame* SaveObject = Cast<UAegisPlayerSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UAegisPlayerSaveGame::StaticClass())
	);
	if (!SaveObject)
	{
		OnGameSaved.Broadcast(SlotName, false);
		return false;
	}

	SaveObject->SlotName = SlotName;
	SaveObject->UserIndex = UserIndex;
	SaveObject->PlayerData.Timestamp = FDateTime::Now();

	// 1. Save Transform
	SaveObject->PlayerData.SavedLocation = Character->GetActorLocation();
	SaveObject->PlayerData.SavedRotation = Character->GetActorRotation();

	// 2. Save Progression
	UAegisPlayerProgressionComponent* Progression = Character->FindComponentByClass<UAegisPlayerProgressionComponent>();
	if (Progression)
	{
		SaveObject->PlayerData.PlayerLevel = Progression->GetCurrentLevel();
		SaveObject->PlayerData.CurrentXP = Progression->GetCurrentXP();
		SaveObject->PlayerData.UnspentAttributePoints = Progression->GetAvailableAttributePoints();
		SaveObject->PlayerData.UnspentSkillPoints = Progression->GetAvailableSkillPoints();
		SaveObject->PlayerData.Vigor = Progression->GetAllocatedPoints(EAegisPlayerAttribute::Vigor);
		SaveObject->PlayerData.Endurance = Progression->GetAllocatedPoints(EAegisPlayerAttribute::Endurance);
		SaveObject->PlayerData.Strength = Progression->GetAllocatedPoints(EAegisPlayerAttribute::Strength);
		SaveObject->PlayerData.Resilience = Progression->GetAllocatedPoints(EAegisPlayerAttribute::Resilience);
	}

	// 3. Save Inventory
	UAegisInventoryComponent* Inventory = Character->FindComponentByClass<UAegisInventoryComponent>();
	if (Inventory)
	{
		SaveObject->PlayerData.InventoryItems = Inventory->GetAllSlots();
	}

	// 4. Save Equipment
	UAegisEquipmentComponent* Equipment = Character->FindComponentByClass<UAegisEquipmentComponent>();
	if (Equipment)
	{
		SaveObject->PlayerData.EquippedItems = Equipment->GetAllEquippedItems();
	}

	// 5. Save Skill Tree
	UAegisSkillTreeComponent* SkillTree = Character->FindComponentByClass<UAegisSkillTreeComponent>();
	if (SkillTree)
	{
		SaveObject->PlayerData.UnlockedSkills = SkillTree->GetUnlockedSkillsMap();
	}

	const bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveObject, SlotName, UserIndex);
	OnGameSaved.Broadcast(SlotName, bSuccess);
	return bSuccess;
}

bool UAegisSaveSubsystem::LoadGame(AAegisCombatCharacter* Character, const FString& SlotName, int32 UserIndex)
{
	if (!Character || !DoesSaveExist(SlotName, UserIndex))
	{
		OnGameLoaded.Broadcast(SlotName, false);
		return false;
	}

	UAegisPlayerSaveGame* SaveObject = Cast<UAegisPlayerSaveGame>(
		UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex)
	);
	if (!SaveObject)
	{
		OnGameLoaded.Broadcast(SlotName, false);
		return false;
	}

	const FAegisPlayerSaveData& Data = SaveObject->PlayerData;

	// 1. Restore Location if valid
	if (!Data.SavedLocation.IsZero())
	{
		Character->SetActorLocationAndRotation(Data.SavedLocation, Data.SavedRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}

	// 2. Restore Progression
	UAegisPlayerProgressionComponent* Progression = Character->FindComponentByClass<UAegisPlayerProgressionComponent>();
	if (Progression)
	{
		Progression->RestoreProgressionData(
			Data.PlayerLevel,
			Data.CurrentXP,
			Data.UnspentAttributePoints,
			Data.UnspentSkillPoints,
			Data.Vigor,
			Data.Endurance,
			Data.Strength,
			Data.Resilience
		);
	}

	// 3. Restore Inventory
	UAegisInventoryComponent* Inventory = Character->FindComponentByClass<UAegisInventoryComponent>();
	if (Inventory)
	{
		Inventory->RestoreInventorySlots(Data.InventoryItems);
	}

	// 4. Restore Equipment
	UAegisEquipmentComponent* Equipment = Character->FindComponentByClass<UAegisEquipmentComponent>();
	if (Equipment)
	{
		Equipment->ClearAllEquipment();
		for (const auto& Kvp : Data.EquippedItems)
		{
			if (Kvp.Value.IsValid())
			{
				Equipment->EquipItemInstance(Kvp.Value, Kvp.Key);
			}
		}
	}

	// 5. Restore Skill Tree
	UAegisSkillTreeComponent* SkillTree = Character->FindComponentByClass<UAegisSkillTreeComponent>();
	if (SkillTree)
	{
		SkillTree->RestoreUnlockedSkills(Data.UnlockedSkills);
	}

	OnGameLoaded.Broadcast(SlotName, true);
	return true;
}
