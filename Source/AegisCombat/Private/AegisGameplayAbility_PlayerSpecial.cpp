#include "AegisGameplayAbility_PlayerSpecial.h"
#include "AbilitySystemComponent.h"
#include "AegisGameplayTags.h"
#include "AegisAttributeSet.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UAegisGameplayAbility_PlayerSpecial::UAegisGameplayAbility_PlayerSpecial()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	const FAegisGameplayTags& Tags = FAegisGameplayTags::Get();
	if (Tags.InputTag_Attack_Special.IsValid())
	{
		SetAssetTags(FGameplayTagContainer(Tags.InputTag_Attack_Special));
	}
	else
	{
		FGameplayTag FallbackTag = FGameplayTag::RequestGameplayTag(FName("InputTag.Attack.Special"), false);
		if (FallbackTag.IsValid())
		{
			SetAssetTags(FGameplayTagContainer(FallbackTag));
		}
	}

	ActivationOwnedTags.AddTag(FAegisGameplayTags::Get().State_Combat_Attacking);
	ActivationBlockedTags.AddTag(FAegisGameplayTags::Get().State_Combat_PoiseBroken);
	ActivationBlockedTags.AddTag(FAegisGameplayTags::Get().State_Player_Dead);
}

void UAegisGameplayAbility_PlayerSpecial::ActivateAbility(
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

	AlreadyHitActors.Empty();

	if (!SpecialMontage)
	{
		// Fallback: Perform radial damage immediately and end
		PerformRadialDamage();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	ActiveMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		SpecialMontage,
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

	ActiveMontageTask->OnCompleted.AddDynamic(this, &UAegisGameplayAbility_PlayerSpecial::OnMontageCompleted);
	ActiveMontageTask->OnBlendOut.AddDynamic(this, &UAegisGameplayAbility_PlayerSpecial::OnMontageCompleted);
	ActiveMontageTask->OnInterrupted.AddDynamic(this, &UAegisGameplayAbility_PlayerSpecial::OnMontageCancelled);
	ActiveMontageTask->OnCancelled.AddDynamic(this, &UAegisGameplayAbility_PlayerSpecial::OnMontageCancelled);
	ActiveMontageTask->ReadyForActivation();

	// Schedule radial damage trigger during the apex of the spin
	PerformRadialDamage();
}

void UAegisGameplayAbility_PlayerSpecial::PerformRadialDamage(float Radius, bool bShowDebug)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return;

	const FVector Origin = Avatar->GetActorLocation();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Avatar);

	TArray<FHitResult> HitResults;
	const EDrawDebugTrace::Type DebugType = bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	const bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetWorld(),
		Origin,
		Origin + FVector(0, 0, 10.0f),
		Radius,
		ObjectTypes,
		false,
		IgnoreActors,
		DebugType,
		HitResults,
		true
	);

	if (!bHit) return;

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC) return;

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || AlreadyHitActors.Contains(HitActor)) continue;

		AlreadyHitActors.Add(HitActor);

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (TargetASC)
		{
			if (DamageEffectClass)
			{
				FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
				Context.AddHitResult(Hit);

				FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, Context);
				if (SpecHandle.IsValid())
				{
					SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
				}
			}
			else
			{
				UAegisAttributeSet* SourceAttr = const_cast<UAegisAttributeSet*>(SourceASC->GetSet<UAegisAttributeSet>());
				UAegisAttributeSet* TargetAttr = const_cast<UAegisAttributeSet*>(TargetASC->GetSet<UAegisAttributeSet>());
				if (SourceAttr && TargetAttr)
				{
					const float AttackPower = SourceAttr->GetAttackPower() * DamageMultiplier;
					const float Defense = TargetAttr->GetDefense();
					const float MitigatedDamage = AttackPower * (100.0f / (100.0f + Defense));
					const float PoiseDamage = AttackPower * 0.35f * PoiseDamageMultiplier;

					TargetAttr->SetIncomingDamage(TargetAttr->GetIncomingDamage() + MitigatedDamage);
					TargetAttr->SetIncomingPoiseDamage(TargetAttr->GetIncomingPoiseDamage() + PoiseDamage);
				}
			}
		}
	}
}

void UAegisGameplayAbility_PlayerSpecial::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAegisGameplayAbility_PlayerSpecial::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UAegisGameplayAbility_PlayerSpecial::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	AlreadyHitActors.Empty();
}
