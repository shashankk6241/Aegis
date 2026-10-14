#include "AegisSkillTreeComponent.h"
#include "AegisPlayerProgressionComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

UAegisSkillTreeComponent::UAegisSkillTreeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAegisSkillTreeComponent::BeginPlay()
{
	Super::BeginPlay();
}

UAegisPlayerProgressionComponent* UAegisSkillTreeComponent::GetProgressionComponent() const
{
	if (AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UAegisPlayerProgressionComponent>();
	}
	return nullptr;
}

UAbilitySystemComponent* UAegisSkillTreeComponent::GetAbilitySystemComponent() const
{
	return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
}

int32 UAegisSkillTreeComponent::GetNodeRank(FName NodeId) const
{
	if (const int32* FoundRank = UnlockedNodeRanks.Find(NodeId))
	{
		return *FoundRank;
	}
	return 0;
}

bool UAegisSkillTreeComponent::IsNodeUnlocked(FName NodeId) const
{
	return GetNodeRank(NodeId) > 0;
}

bool UAegisSkillTreeComponent::CanUnlockNode(FName NodeId) const
{
	if (!SkillTreeData) return false;

	const FAegisSkillNodeDefinition* Node = SkillTreeData->FindNode(NodeId);
	if (!Node) return false;

	// Check rank
	int32 CurrentRank = GetNodeRank(NodeId);
	if (CurrentRank >= Node->MaxRank) return false;

	// Check progression requirements
	UAegisPlayerProgressionComponent* Progression = GetProgressionComponent();
	if (!Progression) return false;

	if (Progression->GetCurrentLevel() < Node->RequiredPlayerLevel) return false;
	if (Progression->GetAvailableSkillPoints() < Node->PointCost) return false;

	// Check prerequisites
	for (const FName& PreReqId : Node->PrerequisiteNodeIds)
	{
		if (!IsNodeUnlocked(PreReqId))
		{
			return false;
		}
	}

	return true;
}

bool UAegisSkillTreeComponent::UnlockSkillNode(FName NodeId)
{
	if (!CanUnlockNode(NodeId)) return false;

	const FAegisSkillNodeDefinition* Node = SkillTreeData->FindNode(NodeId);
	if (!Node) return false;

	UAegisPlayerProgressionComponent* Progression = GetProgressionComponent();
	if (!Progression || !Progression->SpendSkillPoints(Node->PointCost))
	{
		return false;
	}

	int32& Rank = UnlockedNodeRanks.FindOrAdd(NodeId, 0);
	Rank++;

	ApplyNodeGrants(*Node);

	OnSkillUnlocked.Broadcast(NodeId, Rank);
	return true;
}

void UAegisSkillTreeComponent::ApplyNodeGrants(const FAegisSkillNodeDefinition& Node)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC || !GetOwner()->HasAuthority()) return;

	// 1. Grant Active Ability
	if (Node.GrantedAbility)
	{
		// Remove existing handle if any
		if (FGameplayAbilitySpecHandle* ExistingHandle = GrantedAbilityHandles.Find(Node.NodeId))
		{
			ASC->ClearAbility(*ExistingHandle);
			GrantedAbilityHandles.Remove(Node.NodeId);
		}

		FGameplayAbilitySpec Spec(Node.GrantedAbility, 1);
		FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
		if (Handle.IsValid())
		{
			GrantedAbilityHandles.Add(Node.NodeId, Handle);
		}
	}

	// 2. Grant Passive Gameplay Effect
	if (Node.GrantedPassiveEffect)
	{
		// Remove existing handle if any
		if (FActiveGameplayEffectHandle* ExistingHandle = GrantedEffectHandles.Find(Node.NodeId))
		{
			ASC->RemoveActiveGameplayEffect(*ExistingHandle);
			GrantedEffectHandles.Remove(Node.NodeId);
		}

		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(GetOwner());
		FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectToSelf(
			Node.GrantedPassiveEffect->GetDefaultObject<UGameplayEffect>(),
			1.0f,
			Context
		);
		if (Handle.IsValid())
		{
			GrantedEffectHandles.Add(Node.NodeId, Handle);
		}
	}
}

void UAegisSkillTreeComponent::RemoveNodeGrants(FName NodeId)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC || !GetOwner()->HasAuthority()) return;

	if (FGameplayAbilitySpecHandle* AbilityHandle = GrantedAbilityHandles.Find(NodeId))
	{
		ASC->ClearAbility(*AbilityHandle);
		GrantedAbilityHandles.Remove(NodeId);
	}

	if (FActiveGameplayEffectHandle* EffectHandle = GrantedEffectHandles.Find(NodeId))
	{
		ASC->RemoveActiveGameplayEffect(*EffectHandle);
		GrantedEffectHandles.Remove(NodeId);
	}
}

bool UAegisSkillTreeComponent::RespecSkills()
{
	if (!SkillTreeData || UnlockedNodeRanks.Num() == 0) return false;

	UAegisPlayerProgressionComponent* Progression = GetProgressionComponent();
	if (!Progression) return false;

	int32 TotalRefund = 0;

	for (const auto& Kvp : UnlockedNodeRanks)
	{
		const FName NodeId = Kvp.Key;
		const int32 Rank = Kvp.Value;

		if (const FAegisSkillNodeDefinition* Node = SkillTreeData->FindNode(NodeId))
		{
			TotalRefund += (Node->PointCost * Rank);
		}

		RemoveNodeGrants(NodeId);
	}

	UnlockedNodeRanks.Empty();
	Progression->AddSkillPoints(TotalRefund);

	OnSkillsReset.Broadcast();
	return true;
}

void UAegisSkillTreeComponent::RestoreUnlockedSkills(const TMap<FName, int32>& InSkills)
{
	if (!SkillTreeData) return;

	// Clear current grants
	for (const auto& Kvp : UnlockedNodeRanks)
	{
		RemoveNodeGrants(Kvp.Key);
	}
	UnlockedNodeRanks.Empty();

	// Restore and re-grant
	for (const auto& Kvp : InSkills)
	{
		const FName NodeId = Kvp.Key;
		const int32 Rank = Kvp.Value;

		if (Rank > 0)
		{
			if (const FAegisSkillNodeDefinition* Node = SkillTreeData->FindNode(NodeId))
			{
				UnlockedNodeRanks.Add(NodeId, Rank);
				ApplyNodeGrants(*Node);
			}
		}
	}
}
