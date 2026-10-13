#include "AegisGameplayAbility_EnemySpecial.h"
#include "AegisGameplayTags.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Kismet/KismetSystemLibrary.h"

UAegisGameplayAbility_EnemySpecial::UAegisGameplayAbility_EnemySpecial()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	const FAegisGameplayTags& Tags = FAegisGameplayTags::Get();
	SetAssetTags(FGameplayTagContainer(Tags.Ability_Enemy_Special));
	ActivationOwnedTags.AddTag(Tags.State_Combat_Attacking);

	ActivationBlockedTags.AddTag(Tags.State_AI_Dead);
}

void UAegisGameplayAbility_EnemySpecial::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Grant invulnerability if configured (e.g., boss phase transitions)
	if (bGrantInvulnerability)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			ASC->AddLooseGameplayTag(FAegisGameplayTags::Get().State_Enemy_Invulnerable);
		}
	}

	// Spawn VFX at caster location
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (SpecialVFX && Avatar)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			SpecialVFX,
			Avatar->GetActorLocation(),
			Avatar->GetActorRotation()
		);
	}

	// Notify Blueprint
	K2_OnSpecialActivated();

	// Play special montage
	if (SpecialMontage)
	{
		if (ActiveMontageTask)
		{
			ActiveMontageTask->EndTask();
			ActiveMontageTask = nullptr;
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

		if (ActiveMontageTask)
		{
			ActiveMontageTask->OnCompleted.AddDynamic(this, &UAegisGameplayAbility_EnemySpecial::OnSpecialMontageCompleted);
			ActiveMontageTask->OnBlendOut.AddDynamic(this, &UAegisGameplayAbility_EnemySpecial::OnSpecialMontageCompleted);
			ActiveMontageTask->OnInterrupted.AddDynamic(this, &UAegisGameplayAbility_EnemySpecial::OnSpecialMontageCancelled);
			ActiveMontageTask->OnCancelled.AddDynamic(this, &UAegisGameplayAbility_EnemySpecial::OnSpecialMontageCancelled);
			ActiveMontageTask->ReadyForActivation();
			return;
		}
	}

	// If no montage, immediately perform AOE and end
	PerformAOEDamage();
	K2_OnSpecialCompleted();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UAegisGameplayAbility_EnemySpecial::OnSpecialMontageCompleted()
{
	PerformAOEDamage();
	K2_OnSpecialCompleted();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAegisGameplayAbility_EnemySpecial::OnSpecialMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UAegisGameplayAbility_EnemySpecial::PerformAOEDamage()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar || !AOEDamageEffectClass)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC)
	{
		return;
	}

	// Sphere overlap to find targets in AOE
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Avatar);

	TArray<AActor*> OverlappedActors;
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Avatar->GetActorLocation(),
		AOERadius,
		ObjectTypes,
		nullptr,
		IgnoreActors,
		OverlappedActors
	);

	// Apply damage GE to each target
	for (AActor* HitActor : OverlappedActors)
	{
		if (!HitActor)
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (TargetASC)
		{
			FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
			Context.AddSourceObject(Avatar);

			FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(AOEDamageEffectClass, 1.0f, Context);
			if (SpecHandle.IsValid())
			{
				SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}
		}
	}
}

void UAegisGameplayAbility_EnemySpecial::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Remove invulnerability
	if (bGrantInvulnerability)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			ASC->RemoveLooseGameplayTag(FAegisGameplayTags::Get().State_Enemy_Invulnerable);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
