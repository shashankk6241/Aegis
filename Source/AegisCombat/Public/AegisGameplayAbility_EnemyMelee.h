#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AegisGameplayAbility_EnemyMelee.generated.h"

class UAnimMontage;

/**
 * UAegisGameplayAbility_EnemyMelee
 * Enemy melee attack ability — simpler than the player combo system.
 * Selects a random attack montage, performs weapon trace, and applies damage.
 * Designed for BT-driven activation via UAegisBTTask_ActivateAbility.
 */
UCLASS()
class AEGISCOMBAT_API UAegisGameplayAbility_EnemyMelee : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAegisGameplayAbility_EnemyMelee();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	/** Available attack montages (one is randomly chosen per activation) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TArray<TObjectPtr<UAnimMontage>> AttackMontages;

	/** Damage Gameplay Effect applied on hit */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

	/** Cooldown between attacks in seconds */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float AttackCooldown = 1.5f;

	/** Weapon trace sphere radius */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float TraceRadius = 30.0f;

	/** Socket names for weapon trace */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FName TraceStartSocket = FName("WeaponStart");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FName TraceEndSocket = FName("WeaponEnd");

private:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UPROPERTY()
	TObjectPtr<class UAbilityTask_PlayMontageAndWait> ActiveMontageTask;

	TSet<TWeakObjectPtr<AActor>> AlreadyHitActors;
};
