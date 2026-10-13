#include "AegisGameplayAbility_EnemyMelee.h"
#include "AegisGameplayTags.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Kismet/KismetSystemLibrary.h"

UAegisGameplayAbility_EnemyMelee::UAegisGameplayAbility_EnemyMelee()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	const FAegisGameplayTags& Tags = FAegisGameplayTags::Get();
	SetAssetTags(FGameplayTagContainer(Tags.Ability_Enemy_Melee));
	ActivationOwnedTags.AddTag(Tags.State_Combat_Attacking);

	// Block activation during stagger or death
	ActivationBlockedTags.AddTag(Tags.State_Combat_PoiseBroken);
	ActivationBlockedTags.AddTag(Tags.State_AI_Dead);
}

bool UAegisGameplayAbility_EnemyMelee::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return AttackMontages.Num() > 0;
}

void UAegisGameplayAbility_EnemyMelee::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (AttackMontages.IsEmpty())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Pick a random attack montage
	const int32 MontageIndex = FMath::RandRange(0, AttackMontages.Num() - 1);
	UAnimMontage* ChosenMontage = AttackMontages[MontageIndex];
	if (!ChosenMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AlreadyHitActors.Empty();

	// Clean up any previous task
	if (ActiveMontageTask)
	{
		ActiveMontageTask->EndTask();
		ActiveMontageTask = nullptr;
	}

	// Play the attack montage
	ActiveMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		ChosenMontage,
		1.0f,
		NAME_None,
		true,
		1.0f,
		0.0f
	);

	if (!ActiveMontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveMontageTask->OnCompleted.AddDynamic(this, &UAegisGameplayAbility_EnemyMelee::OnMontageCompleted);
	ActiveMontageTask->OnBlendOut.AddDynamic(this, &UAegisGameplayAbility_EnemyMelee::OnMontageCompleted);
	ActiveMontageTask->OnInterrupted.AddDynamic(this, &UAegisGameplayAbility_EnemyMelee::OnMontageCancelled);
	ActiveMontageTask->OnCancelled.AddDynamic(this, &UAegisGameplayAbility_EnemyMelee::OnMontageCancelled);
	ActiveMontageTask->ReadyForActivation();
}

void UAegisGameplayAbility_EnemyMelee::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAegisGameplayAbility_EnemyMelee::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UAegisGameplayAbility_EnemyMelee::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	AlreadyHitActors.Empty();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
