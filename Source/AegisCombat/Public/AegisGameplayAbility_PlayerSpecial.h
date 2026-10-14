#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AegisGameplayAbility_PlayerSpecial.generated.h"

class UAnimMontage;

/**
 * UAegisGameplayAbility_PlayerSpecial
 * Whirlwind / Ultimate AoE strike hitting all surrounding enemies.
 */
UCLASS()
class AEGISCOMBAT_API UAegisGameplayAbility_PlayerSpecial : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAegisGameplayAbility_PlayerSpecial();

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
	void PerformRadialDamage(float Radius = 350.0f, bool bShowDebug = false);

protected:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	TObjectPtr<UAnimMontage> SpecialMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	float DamageMultiplier = 3.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	float PoiseDamageMultiplier = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	float StaminaCost = 35.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

private:
	TSet<TWeakObjectPtr<AActor>> AlreadyHitActors;

	UPROPERTY()
	TObjectPtr<class UAbilityTask_PlayMontageAndWait> ActiveMontageTask;
};
