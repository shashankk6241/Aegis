#include "AegisAnimNotifyState_WeaponTrace.h"
#include "AegisGameplayAbility_Melee.h"
#include "AegisAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

UAegisAnimNotifyState_WeaponTrace::UAegisAnimNotifyState_WeaponTrace()
{
}

void UAegisAnimNotifyState_WeaponTrace::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
}

void UAegisAnimNotifyState_WeaponTrace::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	AActor* Owner = MeshComp->GetOwner();
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC) return;

	// Query active melee abilities to execute weapon trace
	for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.IsActive())
		{
			for (UGameplayAbility* Instance : Spec.GetAbilityInstances())
			{
				if (UAegisGameplayAbility_Melee* MeleeGA = Cast<UAegisGameplayAbility_Melee>(Instance))
				{
					MeleeGA->PerformWeaponTrace(StartSocketName, EndSocketName, TraceRadius);
				}
			}
		}
	}
}

void UAegisAnimNotifyState_WeaponTrace::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}
