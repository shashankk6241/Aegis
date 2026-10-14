#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AegisGameplayAbility_PlayerHeavyAttack.generated.h"

class UAnimMontage;

/**
 * UAegisGameplayAbility_PlayerHeavyAttack
 * High-damage charged / heavy strike. Breaks guard, inflicts heavy poise damage,
 * and warps to combat target.
 */
UCLASS()
class AEGISCOMBAT_API UAegisGameplayAbility_PlayerHeavyAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAegisGameplayAbility_PlayerHeavyAttack();

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

	UFUNCTION(BlueprintCallable, Category = "Aegis|Combat")
	void PerformWeaponTrace(FName StartSocket, FName EndSocket, float Radius, bool bShowDebug = false);

protected:
	void UpdateMotionWarpingTarget();

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	TObjectPtr<UAnimMontage> HeavyAttackMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	float DamageMultiplier = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	float PoiseDamageMultiplier = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	float StaminaCost = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

private:
	TSet<TWeakObjectPtr<AActor>> AlreadyHitActors;

	UPROPERTY()
	TObjectPtr<class UAbilityTask_PlayMontageAndWait> ActiveMontageTask;
};
