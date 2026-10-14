#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AegisSkillTreeDataAsset.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UTexture2D;

/**
 * Single node definition inside an Aegis Skill Tree.
 */
USTRUCT(BlueprintType)
struct AEGISCOMBAT_API FAegisSkillNodeDefinition
{
	GENERATED_BODY()

	/** Unique stable identifier for this node (e.g., Skill.HeavySweep, Skill.IronSkin) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FName NodeId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UTexture2D> Icon;

	/** Player level required before this node can be unlocked */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill", meta = (ClampMin = "1"))
	int32 RequiredPlayerLevel = 1;

	/** Skill points needed per rank */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill", meta = (ClampMin = "1"))
	int32 PointCost = 1;

	/** Maximum rank for this skill node */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill", meta = (ClampMin = "1"))
	int32 MaxRank = 1;

	/** Prerequisite node IDs that must be unlocked prior to unlocking this node */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TArray<FName> PrerequisiteNodeIds;

	/** Active ability granted to the player's ASC when unlocked */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Grants")
	TSubclassOf<UGameplayAbility> GrantedAbility;

	/** Passive gameplay effect applied while unlocked */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Grants")
	TSubclassOf<UGameplayEffect> GrantedPassiveEffect;
};

/**
 * UAegisSkillTreeDataAsset
 * Designer-authored skill tree containing nodes, prerequisites, level gates,
 * and ability/effect grants.
 */
UCLASS(BlueprintType)
class AEGISCOMBAT_API UAegisSkillTreeDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tree")
	FName TreeId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tree")
	FText TreeName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tree", meta = (MultiLine = "true"))
	FText TreeDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tree")
	TArray<FAegisSkillNodeDefinition> Nodes;

	const FAegisSkillNodeDefinition* FindNode(FName NodeId) const
	{
		for (const FAegisSkillNodeDefinition& Node : Nodes)
		{
			if (Node.NodeId == NodeId)
			{
				return &Node;
			}
		}
		return nullptr;
	}
};
