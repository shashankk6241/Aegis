#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AegisPlayerDeathComponent.generated.h"

class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerDied, AActor*, DeadPlayer, AActor*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerRespawned, AActor*, RespawnedPlayer);

/**
 * UAegisPlayerDeathComponent
 * Manages player death flow, death animations, input disabling,
 * and seamless respawn / checkpoint restoration.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AEGISCOMBAT_API UAegisPlayerDeathComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAegisPlayerDeathComponent();

	UFUNCTION(BlueprintCallable, Category = "Aegis|PlayerDeath")
	void ConfigureDeath(UAnimMontage* InDeathMontage, UAnimMontage* InRespawnMontage, bool bInEnableRespawn = true, float InRespawnDelay = 5.0f);

	UFUNCTION(BlueprintCallable, Category = "Aegis|PlayerDeath")
	void ExecuteDeathFlow(AActor* Killer);

	UFUNCTION(BlueprintCallable, Category = "Aegis|PlayerDeath")
	void RespawnPlayer();

	UFUNCTION(BlueprintPure, Category = "Aegis|PlayerDeath")
	bool IsDead() const { return bIsDead; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDied OnPlayerDied;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerRespawned OnPlayerRespawned;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor, AActor* TargetActor, float OldValue, float NewValue);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|PlayerDeath")
	TObjectPtr<UAnimMontage> DeathMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|PlayerDeath")
	TObjectPtr<UAnimMontage> RespawnMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|PlayerDeath")
	bool bEnableRespawn = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|PlayerDeath")
	float RespawnDelay = 5.0f;

	FTransform InitialSpawnTransform;
	bool bIsDead = false;

	FTimerHandle RespawnTimerHandle;
};
