#include "AegisGameplayAbility_PlayerHeavyAttack.h"
#include "AbilitySystemComponent.h"
#include "AegisGameplayTags.h"
#include "AegisAttributeSet.h"
#include "GameFramework/Character.h"
#include "MotionWarpingComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UAegisGameplayAbility_PlayerHeavyAttack::UAegisGameplayAbility_PlayerHeavyAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	const FAegisGameplayTags& Tags = FAegisGameplayTags::Get();
	if (Tags.InputTag_Attack_Heavy.IsValid())
	{
		SetAssetTags(FGameplayTagContainer(Tags.InputTag_Attack_Heavy));
	}
	else
	{
		FGameplayTag FallbackTag = FGameplayTag::RequestGameplayTag(FName("InputTag.Attack.Heavy"), false);
		if (FallbackTag.IsValid())
		{
			SetAssetTags(FGameplayTagContainer(FallbackTag));
		}
	}

	ActivationOwnedTags.AddTag(FAegisGameplayTags::Get().State_Combat_Attacking);
	ActivationBlockedTags.AddTag(FAegisGameplayTags::Get().State_Combat_PoiseBroken);
	ActivationBlockedTags.AddTag(FAegisGameplayTags::Get().State_Player_Dead);
}

void UAegisGameplayAbility_PlayerHeavyAttack::ActivateAbility(
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

	// Deduct stamina
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
	UpdateMotionWarpingTarget();

	if (!HeavyAttackMontage)
	{
		// Fallback check on character or play directly
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		HeavyAttackMontage,
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

	ActiveMontageTask->OnCompleted.AddDynamic(this, &UAegisGameplayAbility_PlayerHeavyAttack::OnMontageCompleted);
	ActiveMontageTask->OnBlendOut.AddDynamic(this, &UAegisGameplayAbility_PlayerHeavyAttack::OnMontageCompleted);
	ActiveMontageTask->OnInterrupted.AddDynamic(this, &UAegisGameplayAbility_PlayerHeavyAttack::OnMontageCancelled);
	ActiveMontageTask->OnCancelled.AddDynamic(this, &UAegisGameplayAbility_PlayerHeavyAttack::OnMontageCancelled);
	ActiveMontageTask->ReadyForActivation();
}

void UAegisGameplayAbility_PlayerHeavyAttack::UpdateMotionWarpingTarget()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character) return;

	UMotionWarpingComponent* WarpingComp = Character->FindComponentByClass<UMotionWarpingComponent>();
	if (!WarpingComp) return;

	const FVector Start = Character->GetActorLocation();
	const FVector Forward = Character->GetActorForwardVector();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Character);

	TArray<FHitResult> Hits;
	const bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetWorld(),
		Start,
		Start + Forward * 600.0f,
		140.0f,
		ObjectTypes,
		false,
		IgnoreActors,
		EDrawDebugTrace::None,
		Hits,
		true
	);

	if (bHit && Hits.Num() > 0)
	{
		AActor* BestTarget = nullptr;
		float ClosestDistSq = TNumericLimits<float>::Max();

		for (const FHitResult& Hit : Hits)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && HitActor != Character)
			{
				const float DistSq = FVector::DistSquared(Start, HitActor->GetActorLocation());
				if (DistSq < ClosestDistSq)
				{
					ClosestDistSq = DistSq;
					BestTarget = HitActor;
				}
			}
		}

		if (BestTarget)
		{
			FVector TargetLoc = BestTarget->GetActorLocation() - (Forward * 90.0f);
			TargetLoc.Z = Character->GetActorLocation().Z;

			FRotator TargetRot = (BestTarget->GetActorLocation() - Start).Rotation();
			TargetRot.Pitch = 0.0f;
			TargetRot.Roll = 0.0f;

			WarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
				FName("CombatTarget"),
				TargetLoc,
				TargetRot
			);
		}
	}
}

void UAegisGameplayAbility_PlayerHeavyAttack::PerformWeaponTrace(FName StartSocket, FName EndSocket, float Radius, bool bShowDebug)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return;

	ACharacter* Character = Cast<ACharacter>(Avatar);
	if (!Character || !Character->GetMesh()) return;

	const FVector StartLoc = Character->GetMesh()->GetSocketLocation(StartSocket);
	const FVector EndLoc = Character->GetMesh()->GetSocketLocation(EndSocket);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Avatar);

	TArray<FHitResult> HitResults;
	const EDrawDebugTrace::Type DebugType = bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	const bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetWorld(),
		StartLoc,
		EndLoc,
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
				// Direct damage execution fallback
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

void UAegisGameplayAbility_PlayerHeavyAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAegisGameplayAbility_PlayerHeavyAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UAegisGameplayAbility_PlayerHeavyAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	AlreadyHitActors.Empty();
}
