#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AegisGameplayAbility_PlayerDodge.generated.h"

class UAnimMontage;

/**
 * UAegisGameplayAbility_PlayerDodge
 * Directional dodge roll granting invulnerability frames (I-frames).
 * Cancels current attacks and consumes stamina.
 */
UCLASS()
class AEGISCOMBAT_API UAegisGameplayAbility_PlayerDodge : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAegisGameplayAbility_PlayerDodge();

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
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	TObjectPtr<UAnimMontage> DodgeMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	float StaminaCost = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	float DodgeImpulse = 900.0f;

private:
	UPROPERTY()
	TObjectPtr<class UAbilityTask_PlayMontageAndWait> ActiveMontageTask;
};
