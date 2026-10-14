#include "AegisGameplayAbility_PlayerBlock.h"
#include "AbilitySystemComponent.h"
#include "AegisGameplayTags.h"
#include "GameFramework/Character.h"

UAegisGameplayAbility_PlayerBlock::UAegisGameplayAbility_PlayerBlock()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	const FAegisGameplayTags& Tags = FAegisGameplayTags::Get();
	if (Tags.InputTag_Block.IsValid())
	{
		SetAssetTags(FGameplayTagContainer(Tags.InputTag_Block));
	}
	else
	{
		FGameplayTag FallbackTag = FGameplayTag::RequestGameplayTag(FName("InputTag.Block"), false);
		if (FallbackTag.IsValid())
		{
			SetAssetTags(FGameplayTagContainer(FallbackTag));
		}
	}

	ActivationOwnedTags.AddTag(FAegisGameplayTags::Get().State_Combat_Blocking);
	ActivationBlockedTags.AddTag(FAegisGameplayTags::Get().State_Combat_PoiseBroken);
	ActivationBlockedTags.AddTag(FAegisGameplayTags::Get().State_Player_Dead);
}

void UAegisGameplayAbility_PlayerBlock::ActivateAbility(
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

	// 1. Activate initial Perfect Parry window
	bIsParrying = true;
	ASC->AddLooseGameplayTag(FAegisGameplayTags::Get().State_Combat_Parrying);

	// Schedule parry window expiration
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		Character->GetWorldTimerManager().SetTimer(
			ParryTimerHandle,
			this,
			&UAegisGameplayAbility_PlayerBlock::OnParryWindowExpired,
			ParryWindowDuration,
			false
		);

		if (BlockMontage)
		{
			Character->PlayAnimMontage(BlockMontage);
		}
	}
}

void UAegisGameplayAbility_PlayerBlock::OnParryWindowExpired()
{
	if (bIsParrying)
	{
		bIsParrying = false;
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			ASC->RemoveLooseGameplayTag(FAegisGameplayTags::Get().State_Combat_Parrying);
		}
	}
}

void UAegisGameplayAbility_PlayerBlock::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UAegisGameplayAbility_PlayerBlock::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	OnParryWindowExpired();

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		Character->GetWorldTimerManager().ClearTimer(ParryTimerHandle);
		if (BlockMontage)
		{
			Character->StopAnimMontage(BlockMontage);
		}
	}
}
