#include "AegisGameplayAbility_PlayerPotion.h"
#include "AegisGameplayTags.h"
#include "AegisAttributeSet.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

UAegisGameplayAbility_PlayerPotion::UAegisGameplayAbility_PlayerPotion()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	const FAegisGameplayTags& Tags = FAegisGameplayTags::Get();
	if (Tags.InputTag_Potion.IsValid())
	{
		SetAssetTags(FGameplayTagContainer(Tags.InputTag_Potion));
	}
	else
	{
		FGameplayTag FallbackTag = FGameplayTag::RequestGameplayTag(FName("InputTag.Potion"), false);
		if (FallbackTag.IsValid())
		{
			SetAssetTags(FGameplayTagContainer(FallbackTag));
		}
	}

	ActivationBlockedTags.AddTag(FAegisGameplayTags::Get().State_Combat_PoiseBroken);
	ActivationBlockedTags.AddTag(FAegisGameplayTags::Get().State_Player_Dead);
}

void UAegisGameplayAbility_PlayerPotion::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAegisAttributeSet* AttrSet = const_cast<UAegisAttributeSet*>(ASC->GetSet<UAegisAttributeSet>());
	if (!AttrSet)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const float MaxHP = AttrSet->GetMaxHealth();
	const float CurrentHP = AttrSet->GetHealth();

	// Restore health
	const float HealAmount = MaxHP * HealPercent;
	AttrSet->SetHealth(FMath::Min(CurrentHP + HealAmount, MaxHP));

	// Trigger GameplayCue
	FGameplayCueParameters CueParams;
	CueParams.Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	ASC->ExecuteGameplayCue(FAegisGameplayTags::Get().GameplayCue_Player_Potion, CueParams);

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character && DrinkMontage)
	{
		Character->PlayAnimMontage(DrinkMontage);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UAegisGameplayAbility_PlayerPotion::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
