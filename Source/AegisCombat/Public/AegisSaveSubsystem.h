#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AegisPlayerSaveGame.h"
#include "AegisSaveSubsystem.generated.h"

class AAegisCombatCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveGameCompleted, const FString&, SlotName, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoadGameCompleted, const FString&, SlotName, bool, bSuccess);

/**
 * UAegisSaveSubsystem
 * Game-instance subsystem coordinating saving and loading of the player character's
 * RPG stats, level, inventory items, equipped gear, and unlocked skill tree talents.
 */
UCLASS()
class AEGISCOMBAT_API UAegisSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UAegisSaveSubsystem();

	// ==========================================
	// Save / Load Operations
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Aegis|SaveSystem")
	bool SaveGame(AAegisCombatCharacter* Character, const FString& SlotName = TEXT("AegisPlayerSave_0"), int32 UserIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "Aegis|SaveSystem")
	bool LoadGame(AAegisCombatCharacter* Character, const FString& SlotName = TEXT("AegisPlayerSave_0"), int32 UserIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "Aegis|SaveSystem")
	bool DeleteSave(const FString& SlotName = TEXT("AegisPlayerSave_0"), int32 UserIndex = 0);

	UFUNCTION(BlueprintPure, Category = "Aegis|SaveSystem")
	bool DoesSaveExist(const FString& SlotName = TEXT("AegisPlayerSave_0"), int32 UserIndex = 0) const;

	UFUNCTION(BlueprintPure, Category = "Aegis|SaveSystem")
	UAegisPlayerSaveGame* GetSaveGameData(const FString& SlotName = TEXT("AegisPlayerSave_0"), int32 UserIndex = 0) const;

	// ==========================================
	// Delegates
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Aegis|SaveSystem")
	FOnSaveGameCompleted OnGameSaved;

	UPROPERTY(BlueprintAssignable, Category = "Aegis|SaveSystem")
	FOnLoadGameCompleted OnGameLoaded;
};
