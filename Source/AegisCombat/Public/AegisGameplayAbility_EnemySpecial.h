#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AegisGameplayAbility_EnemySpecial.generated.h"

class UAnimMontage;
class UNiagaraSystem;

/**
 * UAegisGameplayAbility_EnemySpecial
 * Base class for elite/boss special abilities — supports AOE effects,
 * phase-transition invulnerability, and power-up montages.
 * Subclass in Blueprint for specific boss mechanics.
 */
UCLASS(Blueprintable)
class AEGISCOMBAT_API UAegisGameplayAbility_EnemySpecial : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAegisGameplayAbility_EnemySpecial();

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

protected:
	/** Special attack animation montage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Special")
	TObjectPtr<UAnimMontage> SpecialMontage;

	/** Gameplay Effect to apply in the AOE */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Special")
	TSubclassOf<class UGameplayEffect> AOEDamageEffectClass;

	/** AOE radius around the caster */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Special", meta = (ClampMin = "0.0"))
	float AOERadius = 400.0f;

	/** VFX system to spawn at the caster's location */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Special|VFX")
	TObjectPtr<UNiagaraSystem> SpecialVFX;

	/** If true, grants invulnerability during the ability (for phase transitions) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Special")
	bool bGrantInvulnerability = false;

	/** Blueprint-implementable event for custom special logic */
	UFUNCTION(BlueprintImplementableEvent, Category = "Aegis|Combat", meta = (DisplayName = "On Special Activated"))
	void K2_OnSpecialActivated();

	/** Blueprint-implementable event when the special montage finishes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Aegis|Combat", meta = (DisplayName = "On Special Completed"))
	void K2_OnSpecialCompleted();

private:
	UFUNCTION()
	void OnSpecialMontageCompleted();

	UFUNCTION()
	void OnSpecialMontageCancelled();

	void PerformAOEDamage();

	UPROPERTY()
	TObjectPtr<class UAbilityTask_PlayMontageAndWait> ActiveMontageTask;
};
