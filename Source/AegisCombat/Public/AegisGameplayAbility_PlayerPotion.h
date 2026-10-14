#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AegisGameplayAbility_PlayerPotion.generated.h"

class UAnimMontage;

/**
 * UAegisGameplayAbility_PlayerPotion
 * Consumable healing flask restoring health and triggering restorative visual cues.
 */
UCLASS()
class AEGISCOMBAT_API UAegisGameplayAbility_PlayerPotion : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAegisGameplayAbility_PlayerPotion();

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Potion")
	float HealPercent = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Potion")
	TObjectPtr<UAnimMontage> DrinkMontage = nullptr;
};
