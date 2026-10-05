#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AegisLootDropInfo.h"
#include "AegisDeathHandlerComponent.generated.h"

class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEnemyDied, AActor*, DeadEnemy, AActor*, Killer, int32, XPReward);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLootDropped, AActor*, DeadEnemy, const TArray<FAegisLootDropInfo>&, DroppedLoot);

/**
 * UAegisDeathHandlerComponent
 * Manages the complete enemy death flow: death montage → ragdoll → loot drops → XP broadcast → cleanup.
 */
UCLASS(ClassGroup = (Aegis), meta = (BlueprintSpawnableComponent))
class AEGISCOMBAT_API UAegisDeathHandlerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAegisDeathHandlerComponent();

	/** Initiates the full death sequence */
	UFUNCTION(BlueprintCallable, Category = "Aegis|Death")
	void ExecuteDeathFlow(AActor* Killer);

	/** Check if death flow is currently in progress */
	UFUNCTION(BlueprintPure, Category = "Aegis|Death")
	bool IsDeathInProgress() const { return bDeathInProgress; }

	/** Configure from data asset values (called by enemy character on init) */
	void ConfigureFromData(UAnimMontage* InDeathMontage, int32 InXP, const TArray<FAegisLootDropInfo>& InLootTable);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnemyDied OnEnemyDied;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLootDropped OnLootDropped;

protected:
	/** Death animation montage */
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	TObjectPtr<UAnimMontage> DeathMontage;

	/** Time after death montage before enabling ragdoll */
	UPROPERTY(EditDefaultsOnly, Category = "Death", meta = (ClampMin = "0.0"))
	float RagdollDelay = 0.5f;

	/** Time after ragdoll before destroying the actor */
	UPROPERTY(EditDefaultsOnly, Category = "Death", meta = (ClampMin = "0.0"))
	float DestroyDelay = 5.0f;

	/** XP reward for killing this enemy */
	UPROPERTY(EditDefaultsOnly, Category = "Death|Rewards", meta = (ClampMin = "0"))
	int32 XPReward = 50;

	/** Loot table for death drops */
	UPROPERTY(EditDefaultsOnly, Category = "Death|Rewards")
	TArray<FAegisLootDropInfo> LootTable;

	/** Whether to enable ragdoll after death montage */
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	bool bEnableRagdoll = true;

private:
	bool bDeathInProgress = false;

	UFUNCTION()
	void OnDeathMontageEnded();

	void EnableRagdoll();
	void EvaluateLootDrops();
	void CleanupAndDestroy();

	FTimerHandle RagdollTimerHandle;
	FTimerHandle DestroyTimerHandle;

	TWeakObjectPtr<AActor> KillerActor;
};
