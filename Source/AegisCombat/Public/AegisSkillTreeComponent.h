#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "ActiveGameplayEffectHandle.h"
#include "AegisSkillTreeDataAsset.h"
#include "AegisSkillTreeComponent.generated.h"

class UAegisPlayerProgressionComponent;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillUnlocked, FName, NodeId, int32, NewRank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillsReset);

/**
 * UAegisSkillTreeComponent
 * Evaluates skill node prerequisites, level gates, and costs,
 * grants unlocked abilities/passive effects to the player's ASC,
 * and handles respec refunds.
 */
UCLASS(ClassGroup = (Aegis), meta = (BlueprintSpawnableComponent))
class AEGISCOMBAT_API UAegisSkillTreeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAegisSkillTreeComponent();

	virtual void BeginPlay() override;

	// ==========================================
	// Configuration
	// ==========================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|SkillTree")
	TObjectPtr<UAegisSkillTreeDataAsset> SkillTreeData;

	// ==========================================
	// Operations
	// ==========================================

	UFUNCTION(BlueprintPure, Category = "Aegis|SkillTree")
	bool CanUnlockNode(FName NodeId) const;

	UFUNCTION(BlueprintCallable, Category = "Aegis|SkillTree")
	bool UnlockSkillNode(FName NodeId);

	UFUNCTION(BlueprintCallable, Category = "Aegis|SkillTree")
	bool RespecSkills();

	// ==========================================
	// Queries
	// ==========================================

	UFUNCTION(BlueprintPure, Category = "Aegis|SkillTree")
	int32 GetNodeRank(FName NodeId) const;

	UFUNCTION(BlueprintPure, Category = "Aegis|SkillTree")
	bool IsNodeUnlocked(FName NodeId) const;

	UFUNCTION(BlueprintPure, Category = "Aegis|SkillTree")
	const TMap<FName, int32>& GetUnlockedSkillsMap() const { return UnlockedNodeRanks; }

	UFUNCTION(BlueprintCallable, Category = "Aegis|SkillTree")
	void RestoreUnlockedSkills(const TMap<FName, int32>& InSkills);

	// ==========================================
	// Delegates
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Aegis|SkillTree")
	FOnSkillUnlocked OnSkillUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Aegis|SkillTree")
	FOnSkillsReset OnSkillsReset;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|SkillTree")
	TMap<FName, int32> UnlockedNodeRanks;

	TMap<FName, FGameplayAbilitySpecHandle> GrantedAbilityHandles;
	TMap<FName, FActiveGameplayEffectHandle> GrantedEffectHandles;

	UAegisPlayerProgressionComponent* GetProgressionComponent() const;
	UAbilitySystemComponent* GetAbilitySystemComponent() const;
	void ApplyNodeGrants(const FAegisSkillNodeDefinition& Node);
	void RemoveNodeGrants(FName NodeId);
};
