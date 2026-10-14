#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AegisPlayerDataAsset.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UAnimMontage;
class AAegisWeapon;
class UAegisItemDefinitionDataAsset;

UENUM(BlueprintType)
enum class EAegisPlayerAttribute : uint8
{
	Vigor       UMETA(DisplayName = "Vigor (Health)"),
	Endurance   UMETA(DisplayName = "Endurance (Stamina)"),
	Strength    UMETA(DisplayName = "Strength (Attack Power)"),
	Resilience  UMETA(DisplayName = "Resilience (Defense & Poise)")
};

/**
 * UAegisPlayerDataAsset
 * Complete data-driven configuration for RPG player characters:
 * Base stats, stat growth formulas, XP requirements, starting abilities, weapon, and animations.
 */
UCLASS(BlueprintType)
class AEGISCOMBAT_API UAegisPlayerDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ==========================================
	// Identity
	// ==========================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText CharacterClassName = FText::FromString(TEXT("Champion"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText Description = FText::FromString(TEXT("A balanced combatant specializing in swordplay, parrying, and high-impact strikes."));

	// ==========================================
	// Base Stats (Level 1)
	// ==========================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Base")
	float BaseHealth = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Base")
	float BaseStamina = 150.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Base")
	float BasePoise = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Base")
	float BaseAttackPower = 40.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Base")
	float BaseDefense = 20.0f;

	// ==========================================
	// Stat Point Scaling (Points allocated by player)
	// ==========================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Growth")
	float HealthPerVigor = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Growth")
	float StaminaPerEndurance = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Growth")
	float AttackPowerPerStrength = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Growth")
	float DefensePerResilience = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats|Growth")
	float PoisePerResilience = 5.0f;

	// ==========================================
	// Level Progression
	// ==========================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Progression", meta = (ClampMin = "10"))
	int32 BaseXPRequired = 100;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Progression", meta = (ClampMin = "1.0"))
	float XPExponent = 1.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Progression", meta = (ClampMin = "1"))
	int32 AttributePointsPerLevel = 2;

	// ==========================================
	// Consumables / Potions
	// ==========================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumables", meta = (ClampMin = "1"))
	int32 MaxPotionCharges = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumables", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float PotionHealPercent = 0.45f;

	// ==========================================
	// Equipment & Starting Inventory
	// ==========================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TSubclassOf<AAegisWeapon> DefaultWeaponClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	FName WeaponAttachSocket = FName("hand_r_socket");

	/** Starting weapon item definition (overrides DefaultWeaponClass if set) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UAegisItemDefinitionDataAsset> StartingWeaponItem;

	/** Starting inventory items (consumables, materials, spare equipment) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TArray<TObjectPtr<UAegisItemDefinitionDataAsset>> StartingItems;

	// ==========================================
	// GAS Abilities
	// ==========================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> LightAttackAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> HeavyAttackAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> DodgeAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> BlockAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> SpecialAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> PotionAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> AdditionalAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayEffect> DefaultAttributesEffect;

	// ==========================================
	// Combat Montages
	// ==========================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Combat")
	TObjectPtr<UAnimMontage> HeavyAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Combat")
	TObjectPtr<UAnimMontage> DodgeMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Combat")
	TObjectPtr<UAnimMontage> BlockMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Combat")
	TObjectPtr<UAnimMontage> SpecialAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Combat")
	TObjectPtr<UAnimMontage> HitReactLightMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Combat")
	TObjectPtr<UAnimMontage> HitReactHeavyMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Combat")
	TObjectPtr<UAnimMontage> StunMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Combat")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations|Combat")
	TObjectPtr<UAnimMontage> RespawnMontage;

	// Helper to calculate required XP for a given level
	UFUNCTION(BlueprintPure, Category = "Aegis|Progression")
	int32 CalculateXPForLevel(int32 Level) const
	{
		if (Level <= 1) return BaseXPRequired;
		return FMath::RoundToInt(static_cast<float>(BaseXPRequired) * FMath::Pow(static_cast<float>(Level), XPExponent));
	}
};
