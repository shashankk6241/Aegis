#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AegisGameplayAbility_EnemyRanged.generated.h"

class UAnimMontage;

/**
 * UAegisGameplayAbility_EnemyRanged
 * Enemy ranged attack ability — spawns a projectile actor toward the target.
 * Plays a cast montage, spawns projectile at a socket, and applies damage on hit.
 */
UCLASS()
class AEGISCOMBAT_API UAegisGameplayAbility_EnemyRanged : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAegisGameplayAbility_EnemyRanged();

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
	/** Cast animation montage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAnimMontage> CastMontage;

	/** Projectile actor class to spawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<AActor> ProjectileClass;

	/** Socket on the mesh to spawn the projectile from */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FName ProjectileSpawnSocket = FName("Muzzle_Socket");

	/** Damage Gameplay Effect applied by the projectile */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;

	/** Projectile speed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float ProjectileSpeed = 2000.0f;

	/** Minimum range — AI won't use this ability if closer than this */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float MinRange = 300.0f;

	/** Maximum range for this ability */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float MaxRange = 1500.0f;

private:
	UFUNCTION()
	void OnCastMontageCompleted();

	UFUNCTION()
	void OnCastMontageCancelled();

	void SpawnProjectile();

	UPROPERTY()
	TObjectPtr<class UAbilityTask_PlayMontageAndWait> ActiveMontageTask;
};
