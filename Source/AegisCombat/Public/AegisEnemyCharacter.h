#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AegisEnemyArchetype.h"
#include "AegisEnemyCharacter.generated.h"

class UAegisAbilitySystemComponent;
class UAegisAttributeSet;
class UAegisEnemyDataAsset;
class UAegisAggroComponent;
class UAegisDeathHandlerComponent;
class UAegisEnemyWidgetComponent;
class UAnimMontage;
class UWidgetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyPhaseChanged, AAegisEnemyCharacter*, Enemy, int32, NewPhase);

/**
 * AAegisEnemyCharacter
 * Full RPG enemy character integrating data-driven configuration, GAS combat,
 * aggro management, death handling, health bar UI, and boss phase transitions.
 */
UCLASS()
class AEGISCOMBAT_API AAegisEnemyCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAegisEnemyCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintPure, Category = "Aegis|GAS")
	UAegisAttributeSet* GetAttributeSet() const { return AttributeSet; }

	UFUNCTION(BlueprintPure, Category = "Aegis|Enemy")
	UAegisAggroComponent* GetAggroComponent() const { return AggroComponent; }

	UFUNCTION(BlueprintPure, Category = "Aegis|Enemy")
	UAegisDeathHandlerComponent* GetDeathHandler() const { return DeathHandlerComponent; }

	// ==========================================
	// Identity & Configuration
	// ==========================================

	/** Get the enemy's display name from the data asset */
	UFUNCTION(BlueprintPure, Category = "Aegis|Enemy")
	FText GetEnemyName() const;

	/** Get the enemy's current level */
	UFUNCTION(BlueprintPure, Category = "Aegis|Enemy")
	int32 GetEnemyLevel() const { return EnemyLevel; }

	/** Get the enemy's archetype */
	UFUNCTION(BlueprintPure, Category = "Aegis|Enemy")
	EAegisEnemyArchetype GetArchetype() const;

	/** Get the data asset driving this enemy */
	UFUNCTION(BlueprintPure, Category = "Aegis|Enemy")
	const UAegisEnemyDataAsset* GetEnemyDataAsset() const { return EnemyDataAsset; }

	// ==========================================
	// Combat State
	// ==========================================

	/** Check if this enemy is dead */
	UFUNCTION(BlueprintPure, Category = "Aegis|Enemy")
	bool IsDead() const;

	/** Get the current boss phase (0 = normal / no phase, 1+ = phase index) */
	UFUNCTION(BlueprintPure, Category = "Aegis|Enemy")
	int32 GetCurrentPhase() const { return CurrentBossPhase; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnemyPhaseChanged OnPhaseChanged;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

	// ==========================================
	// Core GAS Components
	// ==========================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|GAS")
	TObjectPtr<UAegisAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|GAS")
	TObjectPtr<UAegisAttributeSet> AttributeSet;

	// ==========================================
	// RPG Components
	// ==========================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Enemy")
	TObjectPtr<UAegisAggroComponent> AggroComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Enemy")
	TObjectPtr<UAegisDeathHandlerComponent> DeathHandlerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|UI")
	TObjectPtr<UAegisEnemyWidgetComponent> HealthBarWidgetComponent;

	// ==========================================
	// Data-Driven Configuration
	// ==========================================

	/** The data asset defining this enemy type's stats, abilities, AI, and visuals */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Enemy")
	TObjectPtr<UAegisEnemyDataAsset> EnemyDataAsset;

	/** Override level (uses data asset default if 0) */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Aegis|Enemy", meta = (ClampMin = "0"))
	int32 EnemyLevelOverride = 0;

	// ==========================================
	// Hit Reaction Montages (fallback if no data asset)
	// ==========================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	TObjectPtr<UAnimMontage> HitReactLightMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	TObjectPtr<UAnimMontage> HitReactHeavyMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	TObjectPtr<UAnimMontage> PoiseBreakStaggerMontage;

	// ==========================================
	// Patrol Configuration
	// ==========================================

	/** Optional patrol waypoints (if empty, AI uses random nav points) */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Aegis|AI")
	TArray<FVector> PatrolWaypoints;

	// ==========================================
	// Initialization
	// ==========================================
	virtual void InitializeFromDataAsset();
	virtual void InitializeAttributes();
	virtual void InitializeAbilities();
	virtual void InitializeAI();
	virtual void InitializeHealthBar();

	// ==========================================
	// Event Handlers
	// ==========================================
	UFUNCTION()
	virtual void HandlePoiseBroken(AActor* TargetActor);

	UFUNCTION()
	virtual void OnHealthChanged(AActor* InstigatorActor, AActor* TargetActor, float OldValue, float NewValue);

	UFUNCTION()
	virtual void HandleDeath(AActor* DeadEnemy, AActor* Killer, int32 XPReward);

	// Boss phase transition
	virtual void CheckPhaseTransition(float HealthPercent);

private:
	int32 EnemyLevel = 1;
	int32 CurrentBossPhase = 0;
};
