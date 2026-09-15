#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AegisEnemyCharacter.generated.h"

class UAegisAbilitySystemComponent;
class UAegisAttributeSet;
class UAnimMontage;

/**
 * AAegisEnemyCharacter
 * AI/Dummy combat entity demonstrating GAS reactions, poise stagger, and hit montages.
 */
UCLASS()
class AEGISCOMBAT_API AAegisEnemyCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAegisEnemyCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAegisAttributeSet* GetAttributeSet() const { return AttributeSet; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|GAS")
	TObjectPtr<UAegisAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|GAS")
	TObjectPtr<UAegisAttributeSet> AttributeSet;

	// Hit Reaction Montages
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	TObjectPtr<UAnimMontage> HitReactLightMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	TObjectPtr<UAnimMontage> HitReactHeavyMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	TObjectPtr<UAnimMontage> PoiseBreakStaggerMontage;

	// Event Handlers
	UFUNCTION()
	virtual void HandlePoiseBroken(AActor* TargetActor);

	UFUNCTION()
	virtual void OnHealthChanged(AActor* InstigatorActor, AActor* TargetActor, float OldValue, float NewValue);
};
