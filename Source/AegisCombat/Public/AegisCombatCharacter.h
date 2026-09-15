#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "AegisCombatCharacter.generated.h"

class UAegisAbilitySystemComponent;
class UAegisAttributeSet;
class UMotionWarpingComponent;
class UGameplayAbility;

/**
 * AAegisCombatCharacter
 * Base combat character implementing GAS, Motion Warping, and Enhanced Input integration.
 */
UCLASS()
class AEGISCOMBAT_API AAegisCombatCharacter : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AAegisCombatCharacter();

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    UFUNCTION(BlueprintPure, Category = "Aegis|GAS")
    UAegisAttributeSet* GetAttributeSet() const { return AttributeSet; }

    UFUNCTION(BlueprintPure, Category = "Aegis|Attributes")
    float GetHealth() const;

    UFUNCTION(BlueprintPure, Category = "Aegis|Attributes")
    float GetMaxHealth() const;

    UFUNCTION(BlueprintPure, Category = "Aegis|Attributes")
    float GetPoise() const;

    UFUNCTION(BlueprintPure, Category = "Aegis|Attributes")
    float GetMaxPoise() const;

    UFUNCTION(BlueprintCallable, Category = "Aegis|Combat")
    void PerformMeleeAttack();

    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
    virtual void BeginPlay() override;

    // Core GAS Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|GAS")
    TObjectPtr<UAegisAbilitySystemComponent> AbilitySystemComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|GAS")
    TObjectPtr<UAegisAttributeSet> AttributeSet;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Combat")
    TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

    // Startup Abilities
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Abilities")
    TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Abilities")
    TSubclassOf<UGameplayAbility> MeleeAbilityClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Abilities")
    TSubclassOf<class UGameplayEffect> DefaultAttributesEffect;

    // Initialization
    virtual void InitAbilityActorInfo();
    virtual void InitializeDefaultAttributes();

    // Poise Break Event Callback
    UFUNCTION()
    virtual void HandlePoiseBroken(AActor* TargetActor);
};
