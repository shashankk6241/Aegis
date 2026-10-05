#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AegisEnemyArchetype.h"
#include "AegisLootDropInfo.h"
#include "AegisEnemyDataAsset.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UAnimMontage;
class UBehaviorTree;
class UCurveFloat;
class USkeletalMesh;
class UAnimInstance;

/**
 * UAegisEnemyDataAsset
 * Designer-authored data asset defining an enemy type's full configuration:
 * stats, abilities, AI behavior, visuals, animations, and loot tables.
 */
UCLASS(BlueprintType)
class AEGISCOMBAT_API UAegisEnemyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ==========================================
	// Identity
	// ==========================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	EAegisEnemyArchetype Archetype = EAegisEnemyArchetype::Melee_Grunt;

	// ==========================================
	// Level & Base Stats
	// ==========================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1"))
	int32 DefaultLevel = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float BaseHealth = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float BaseStamina = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float BasePoise = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float BaseAttackPower = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float BaseDefense = 10.0f;

	// ==========================================
	// Stat Scaling Configuration
	// ==========================================

	/** Per-level growth rate for Health */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Scaling")
	float HealthGrowthRate = 0.12f;

	/** Per-level growth rate for Stamina */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Scaling")
	float StaminaGrowthRate = 0.05f;

	/** Per-level growth rate for Poise */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Scaling")
	float PoiseGrowthRate = 0.05f;

	/** Per-level growth rate for AttackPower */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Scaling")
	float AttackPowerGrowthRate = 0.08f;

	/** Per-level growth rate for Defense */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Scaling")
	float DefenseGrowthRate = 0.06f;

	/** Optional curve for non-linear stat scaling. X = Level, Y = Multiplier. Overrides linear growth rates if set. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Scaling")
	TObjectPtr<UCurveFloat> StatScalingCurve;

	// ==========================================
	// GAS Configuration
	// ==========================================

	/** Abilities granted to this enemy on spawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

	/** Gameplay Effect used to initialize default attribute values */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayEffect> DefaultAttributesEffect;

	// ==========================================
	// AI Configuration
	// ==========================================

	/** Behavior Tree asset driving this enemy's AI logic */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	/** Sight detection radius in cm */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float DetectionRadius = 1200.0f;

	/** Melee attack engagement range in cm */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float AttackRange = 200.0f;

	/** Range at which enemy disengages and returns to patrol */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float LoseAggroRange = 2000.0f;

	/** Movement speed while patrolling */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float PatrolSpeed = 200.0f;

	/** Movement speed while chasing a target */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float ChaseSpeed = 450.0f;

	/** AI hearing perception radius in cm */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float HearingRange = 800.0f;

	// ==========================================
	// Animation Montages
	// ==========================================

	/** Attack montages used by combat abilities (can be multiple for variety) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TArray<TObjectPtr<UAnimMontage>> AttackMontages;

	/** Light flinch reaction montage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HitReactLightMontage;

	/** Heavy stagger reaction montage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HitReactHeavyMontage;

	/** Poise break full stagger montage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> StaggerMontage;

	/** Death montage (played before ragdoll/dissolve) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

	// ==========================================
	// Visual Configuration
	// ==========================================

	/** Skeletal mesh override (if not using a Blueprint subclass) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	/** Animation Blueprint class override */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	TSubclassOf<UAnimInstance> AnimBlueprintClass;

	// ==========================================
	// Rewards
	// ==========================================

	/** Base XP awarded on kill (scaled by level difference) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewards", meta = (ClampMin = "0"))
	int32 XPReward = 50;

	/** Loot table entries evaluated on death */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rewards")
	TArray<FAegisLootDropInfo> LootTable;

	// ==========================================
	// Boss-Specific Configuration
	// ==========================================

	/** Health percentage thresholds that trigger phase transitions (e.g., 0.75, 0.50, 0.25) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss", meta = (EditCondition = "Archetype == EAegisEnemyArchetype::Boss"))
	TArray<float> PhaseThresholds;

	/** Abilities granted at each phase transition (indexed to match PhaseThresholds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss", meta = (EditCondition = "Archetype == EAegisEnemyArchetype::Boss"))
	TArray<TSubclassOf<UGameplayAbility>> PhaseAbilities;

	// ==========================================
	// Utility Functions
	// ==========================================

	/** Calculate a single stat value scaled to a given level using this asset's configuration */
	UFUNCTION(BlueprintPure, Category = "Aegis|Enemy")
	float GetScaledStat(float BaseValue, float GrowthRate, int32 Level) const;

	/** Returns the PrimaryAssetId for asset manager lookups */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
