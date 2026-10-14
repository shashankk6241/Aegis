#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AegisPlayerDataAsset.h"
#include "AegisPlayerProgressionComponent.generated.h"

class UAegisPlayerDataAsset;
class UAegisAttributeSet;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerXPChanged, int32, CurrentXP, int32, RequiredXP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLevelUp, int32, NewLevel, int32, AttributePointsGained);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributePointsChanged, int32, AvailablePoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillPointsChanged, int32, AvailablePoints);

/**
 * UAegisPlayerProgressionComponent
 * Manages player character RPG leveling, experience points, stat point allocation,
 * and dynamic attribute recalculation.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AEGISCOMBAT_API UAegisPlayerProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAegisPlayerProgressionComponent();

	// Initialization
	UFUNCTION(BlueprintCallable, Category = "Aegis|Progression")
	void ConfigureFromData(const UAegisPlayerDataAsset* InDataAsset);

	// Experience & Leveling
	UFUNCTION(BlueprintCallable, Category = "Aegis|Progression")
	void AddXP(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Aegis|Progression")
	int32 GetCurrentLevel() const { return CurrentLevel; }

	UFUNCTION(BlueprintPure, Category = "Aegis|Progression")
	int32 GetCurrentXP() const { return CurrentXP; }

	UFUNCTION(BlueprintPure, Category = "Aegis|Progression")
	int32 GetXPForNextLevel() const { return XPForNextLevel; }

	UFUNCTION(BlueprintPure, Category = "Aegis|Progression")
	float GetXPPercent() const;

	UFUNCTION(BlueprintPure, Category = "Aegis|Progression")
	int32 GetAvailableAttributePoints() const { return AvailableAttributePoints; }

	UFUNCTION(BlueprintPure, Category = "Aegis|Progression")
	int32 GetAvailableSkillPoints() const { return AvailableSkillPoints; }

	UFUNCTION(BlueprintPure, Category = "Aegis|Progression")
	int32 GetAllocatedPoints(EAegisPlayerAttribute Attribute) const;

	// Stat Allocation
	UFUNCTION(BlueprintCallable, Category = "Aegis|Progression")
	bool AllocateAttributePoint(EAegisPlayerAttribute Attribute);

	// Skill Points
	UFUNCTION(BlueprintCallable, Category = "Aegis|Progression")
	bool SpendSkillPoints(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Aegis|Progression")
	void AddSkillPoints(int32 Amount);

	// Save/Load Restoration
	UFUNCTION(BlueprintCallable, Category = "Aegis|Progression")
	void RestoreProgressionData(
		int32 InLevel,
		int32 InXP,
		int32 InUnspentAttrPoints,
		int32 InUnspentSkillPoints,
		int32 InVigor,
		int32 InEndurance,
		int32 InStrength,
		int32 InResilience
	);

	// Automatic Enemy Death listener
	UFUNCTION()
	void HandleEnemyKilled(AActor* DeadEnemy, AActor* Killer, int32 XPReward);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerXPChanged OnXPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerLevelUp OnLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAttributePointsChanged OnAttributePointsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSkillPointsChanged OnSkillPointsChanged;

protected:
	virtual void BeginPlay() override;

	void UpdateRequiredXP();
	void RecalculateAttributes();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aegis|Progression")
	int32 CurrentLevel = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Progression")
	int32 CurrentXP = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Progression")
	int32 XPForNextLevel = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Progression")
	int32 AvailableAttributePoints = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Progression")
	int32 AvailableSkillPoints = 0;

	// Allocated Points
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Progression")
	int32 VigorPoints = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Progression")
	int32 EndurancePoints = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Progression")
	int32 StrengthPoints = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|Progression")
	int32 ResiliencePoints = 0;

	UPROPERTY()
	TObjectPtr<const UAegisPlayerDataAsset> PlayerDataAsset;
};
