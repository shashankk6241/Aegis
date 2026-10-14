#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AegisGameplayAbility_PlayerBlock.generated.h"

class UAnimMontage;

/**
 * UAegisGameplayAbility_PlayerBlock
 * Defensive stance providing damage mitigation while held.
 * Grants a perfect parry window during the first 0.25 seconds.
 */
UCLASS()
class AEGISCOMBAT_API UAegisGameplayAbility_PlayerBlock : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAegisGameplayAbility_PlayerBlock();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void InputReleased(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	void OnParryWindowExpired();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	TObjectPtr<UAnimMontage> BlockMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	float ParryWindowDuration = 0.25f;

private:
	FTimerHandle ParryTimerHandle;
	bool bIsParrying = false;
};
