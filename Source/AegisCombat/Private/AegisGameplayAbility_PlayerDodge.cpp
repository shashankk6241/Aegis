#include "AegisGameplayAbility_PlayerDodge.h"
#include "AbilitySystemComponent.h"
#include "AegisGameplayTags.h"
#include "AegisAttributeSet.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UAegisGameplayAbility_PlayerDodge::UAegisGameplayAbility_PlayerDodge()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	const FAegisGameplayTags& Tags = FAegisGameplayTags::Get();
	if (Tags.InputTag_Dodge.IsValid())
	{
		SetAssetTags(FGameplayTagContainer(Tags.InputTag_Dodge));
	}
	else
	{
		FGameplayTag FallbackTag = FGameplayTag::RequestGameplayTag(FName("InputTag.Dodge"), false);
		if (FallbackTag.IsValid())
		{
			SetAssetTags(FGameplayTagContainer(FallbackTag));
		}
	}

	ActivationOwnedTags.AddTag(FAegisGameplayTags::Get().State_Combat_Dodging);
	ActivationBlockedTags.AddTag(FAegisGameplayTags::Get().State_Combat_PoiseBroken);
	ActivationBlockedTags.AddTag(FAegisGameplayTags::Get().State_Player_Dead);
}

void UAegisGameplayAbility_PlayerDodge::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. Deduct stamina
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		UAegisAttributeSet* AttrSet = const_cast<UAegisAttributeSet*>(ASC->GetSet<UAegisAttributeSet>());
		if (AttrSet)
		{
			const float CurrentStamina = AttrSet->GetStamina();
			if (CurrentStamina < StaminaCost)
			{
				EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
				return;
			}
			AttrSet->SetStamina(FMath::Max(CurrentStamina - StaminaCost, 0.0f));
		}
	}

	// 2. Determine dodge direction (based on movement acceleration or facing backward)
	FVector DodgeDirection = Character->GetCharacterMovement()->GetCurrentAcceleration().GetSafeNormal2D();
	if (DodgeDirection.IsNearlyZero())
	{
		DodgeDirection = -Character->GetActorForwardVector();
	}

	// Rotate character toward dodge direction
	Character->SetActorRotation(DodgeDirection.Rotation());

	// Apply quick directional impulse if not using full root motion montage
	if (!DodgeMontage)
	{
		Character->LaunchCharacter(DodgeDirection * DodgeImpulse + FVector(0, 0, 150.0f), true, false);

		// End ability after short duration (0.4s I-frame window)
		FTimerHandle TimerHandle;
		Character->GetWorldTimerManager().SetTimer(
			TimerHandle,
			[this]()
			{
				EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			},
			0.45f,
			false
		);
		return;
	}

	// Play dodge montage
	ActiveMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		DodgeMontage,
		1.2f,
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

	ActiveMontageTask->OnCompleted.AddDynamic(this, &UAegisGameplayAbility_PlayerDodge::OnMontageCompleted);
	ActiveMontageTask->OnBlendOut.AddDynamic(this, &UAegisGameplayAbility_PlayerDodge::OnMontageCompleted);
	ActiveMontageTask->OnInterrupted.AddDynamic(this, &UAegisGameplayAbility_PlayerDodge::OnMontageCancelled);
	ActiveMontageTask->OnCancelled.AddDynamic(this, &UAegisGameplayAbility_PlayerDodge::OnMontageCancelled);
	ActiveMontageTask->ReadyForActivation();
}

void UAegisGameplayAbility_PlayerDodge::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAegisGameplayAbility_PlayerDodge::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UAegisGameplayAbility_PlayerDodge::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
