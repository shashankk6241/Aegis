#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AegisGameplayAbility_Melee.generated.h"

class UAnimMontage;
class UMotionWarpingComponent;

USTRUCT(BlueprintType)
struct FAegisAttackMontageInfo
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<UAnimMontage> Montage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float DamageMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float PoiseDamageMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float WarpDistance = 250.0f;
};

/**
 * UAegisGameplayAbility_Melee
 * Core combo melee attack ability integrating Motion Warping, Root Motion, and Hit Traces.
 */
UCLASS()
class AEGISCOMBAT_API UAegisGameplayAbility_Melee : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UAegisGameplayAbility_Melee();

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

    // Hit-box Sphere Sweep
    UFUNCTION(BlueprintCallable, Category = "Aegis|Combat")
    void PerformWeaponTrace(FName StartSocket, FName EndSocket, float Radius, bool bShowDebug = false);

protected:
    // Combo Configuration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
    TArray<FAegisAttackMontageInfo> ComboChain;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
    TSubclassOf<class UGameplayEffect> DamageEffectClass;

    UPROPERTY(BlueprintReadOnly, Category = "Aegis|Combat")
    int32 CurrentComboIndex = 0;

    // Motion Warping
    void UpdateMotionWarpingTarget();
    void PlayCurrentComboAttack();

    // Anim Montage Callbacks
    UFUNCTION()
    void OnMontageCompleted();

    UFUNCTION()
    void OnMontageCancelled();

    UFUNCTION()
    void OnComboWindowClosed(FGameplayEventData Payload);

    bool TryAdvanceCombo();

private:
    TSet<TWeakObjectPtr<AActor>> AlreadyHitActors;
    
    UPROPERTY()
    TObjectPtr<class UAbilityTask_PlayMontageAndWait> ActiveMontageTask;

    UPROPERTY()
    TObjectPtr<class UAbilityTask_WaitGameplayEvent> ActiveEventTask;
};
