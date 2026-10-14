#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "AegisPlayerDataAsset.h"
#include "AegisCombatCharacter.generated.h"

class UAegisAbilitySystemComponent;
class UAegisAttributeSet;
class UMotionWarpingComponent;
class UAegisPlayerDataAsset;
class UAegisPlayerProgressionComponent;
class UAegisTargetLockComponent;
class UAegisPlayerDeathComponent;
class UAegisInventoryComponent;
class UAegisEquipmentComponent;
class AAegisWeapon;
class UGameplayAbility;
class UAnimMontage;

/**
 * AAegisCombatCharacter
 * Advanced RPG player character integrating data-driven configuration,
 * full GAS combat suite (Light/Heavy/Dodge/Block/Parry/Special/Potion),
 * progression & leveling, Souls-like target locking, and death/respawn flow.
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

	UFUNCTION(BlueprintPure, Category = "Aegis|Components")
	UAegisPlayerProgressionComponent* GetProgressionComponent() const { return ProgressionComponent; }

	UFUNCTION(BlueprintPure, Category = "Aegis|Components")
	UAegisTargetLockComponent* GetTargetLockComponent() const { return TargetLockComponent; }

	UFUNCTION(BlueprintPure, Category = "Aegis|Components")
	UAegisPlayerDeathComponent* GetDeathComponent() const { return DeathComponent; }

	UFUNCTION(BlueprintPure, Category = "Aegis|Weapon")
	AAegisWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	// ==========================================
	// Attribute Pure Getters for UI / HUD
	// ==========================================
	UFUNCTION(BlueprintPure, Category = "Aegis|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Attributes")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Attributes")
	float GetStamina() const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Attributes")
	float GetMaxStamina() const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Attributes")
	float GetStaminaPercent() const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Attributes")
	float GetPoise() const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Attributes")
	float GetMaxPoise() const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Attributes")
	float GetPoisePercent() const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Attributes")
	float GetAttackPower() const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Attributes")
	float GetDefense() const;

	// ==========================================
	// Progression Getters
	// ==========================================
	UFUNCTION(BlueprintPure, Category = "Aegis|Progression")
	int32 GetPlayerLevel() const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Progression")
	int32 GetCurrentXP() const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Progression")
	int32 GetXPForNextLevel() const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Progression")
	float GetXPPercent() const;

	// ==========================================
	// Combat Actions
	// ==========================================
	UFUNCTION(BlueprintCallable, Category = "Aegis|Combat")
	void PerformMeleeAttack(); // Light combo attack

	UFUNCTION(BlueprintCallable, Category = "Aegis|Combat")
	void PerformHeavyAttack(); // Heavy strike

	UFUNCTION(BlueprintCallable, Category = "Aegis|Combat")
	void PerformDodge(); // Dodge roll / dash with I-frames

	UFUNCTION(BlueprintCallable, Category = "Aegis|Combat")
	void StartBlock(); // Block & parry window

	UFUNCTION(BlueprintCallable, Category = "Aegis|Combat")
	void EndBlock();

	UFUNCTION(BlueprintCallable, Category = "Aegis|Combat")
	void PerformSpecialAttack(); // Whirlwind AoE

	UFUNCTION(BlueprintCallable, Category = "Aegis|Combat")
	void UsePotion(); // Healing flask

	UFUNCTION(BlueprintCallable, Category = "Aegis|Combat")
	void ToggleTargetLock(); // Souls-like Lock-on

	UFUNCTION(BlueprintCallable, Category = "Aegis|Combat")
	void SwitchTargetLock(float Direction);

	// Status queries
	UFUNCTION(BlueprintPure, Category = "Aegis|Combat")
	bool IsTargetLocked() const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Combat")
	AActor* GetLockedTarget() const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Combat")
	bool IsDead() const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Combat")
	int32 GetRemainingPotions() const { return RemainingPotions; }

	UFUNCTION(BlueprintPure, Category = "Aegis|Combat")
	int32 GetMaxPotions() const;

	// Inventory & Equipment Accessors
	UFUNCTION(BlueprintPure, Category = "Aegis|Inventory")
	UAegisInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	UFUNCTION(BlueprintPure, Category = "Aegis|Equipment")
	UAegisEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

	UFUNCTION(BlueprintCallable, Category = "Aegis|Weapon")
	void SetEquippedWeapon(AAegisWeapon* InWeapon) { EquippedWeapon = InWeapon; }

	UFUNCTION(BlueprintCallable, Category = "Aegis|Abilities")
	void SetMeleeAbilityClass(TSubclassOf<UGameplayAbility> InClass) { MeleeAbilityClass = InClass; }

	UFUNCTION(BlueprintCallable, Category = "Aegis|Abilities")
	void SetHeavyAttackAbilityClass(TSubclassOf<UGameplayAbility> InClass) { HeavyAttackAbilityClass = InClass; }

	UFUNCTION(BlueprintCallable, Category = "Aegis|Abilities")
	void SetSpecialAbilityClass(TSubclassOf<UGameplayAbility> InClass) { SpecialAbilityClass = InClass; }

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	// ==========================================
	// Core GAS Components
	// ==========================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|GAS")
	TObjectPtr<UAegisAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|GAS")
	TObjectPtr<UAegisAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Combat")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	// ==========================================
	// RPG Player Components
	// ==========================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Progression")
	TObjectPtr<UAegisPlayerProgressionComponent> ProgressionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Combat")
	TObjectPtr<UAegisTargetLockComponent> TargetLockComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Combat")
	TObjectPtr<UAegisPlayerDeathComponent> DeathComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Inventory")
	TObjectPtr<UAegisInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Equipment")
	TObjectPtr<UAegisEquipmentComponent> EquipmentComponent;

	// ==========================================
	// Data-Driven Configuration
	// ==========================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Data")
	TObjectPtr<UAegisPlayerDataAsset> PlayerDataAsset;

	// ==========================================
	// Startup Abilities (Overrides if no data asset)
	// ==========================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Abilities")
	TSubclassOf<UGameplayAbility> MeleeAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Abilities")
	TSubclassOf<UGameplayAbility> HeavyAttackAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Abilities")
	TSubclassOf<UGameplayAbility> DodgeAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Abilities")
	TSubclassOf<UGameplayAbility> BlockAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Abilities")
	TSubclassOf<UGameplayAbility> SpecialAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Abilities")
	TSubclassOf<UGameplayAbility> PotionAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Abilities")
	TSubclassOf<class UGameplayEffect> DefaultAttributesEffect;

	// Equipment
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Weapon")
	TObjectPtr<AAegisWeapon> EquippedWeapon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Weapon")
	TSubclassOf<AAegisWeapon> DefaultWeaponClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Weapon")
	FName WeaponAttachSocket = FName("hand_r_socket");

	// Hit Reaction Montages
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	TObjectPtr<UAnimMontage> HitReactLightMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	TObjectPtr<UAnimMontage> HitReactHeavyMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Combat")
	TObjectPtr<UAnimMontage> PoiseBreakStaggerMontage;

	// Initialization Pipelines
	virtual void InitAbilityActorInfo();
	virtual void InitializeFromDataAsset();
	virtual void InitializeDefaultAttributes();
	virtual void SpawnAndEquipDefaultWeapon();

	// Event Handlers
	UFUNCTION()
	virtual void HandlePoiseBroken(AActor* TargetActor);

	int32 RemainingPotions = 3;
};
