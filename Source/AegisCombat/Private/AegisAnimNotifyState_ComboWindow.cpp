#include "AegisAnimNotifyState_ComboWindow.h"
#include "AegisAbilitySystemComponent.h"
#include "AegisGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"

UAegisAnimNotifyState_ComboWindow::UAegisAnimNotifyState_ComboWindow()
{
}

void UAegisAnimNotifyState_ComboWindow::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());
	if (ASC)
	{
		ASC->AddLooseGameplayTag(FAegisGameplayTags::Get().State_Combat_CanCombo);
	}
}

void UAegisAnimNotifyState_ComboWindow::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());
	if (ASC)
	{
		ASC->RemoveLooseGameplayTag(FAegisGameplayTags::Get().State_Combat_CanCombo);
		
		FGameplayEventData Payload;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), FAegisGameplayTags::Get().State_Combat_CanCombo, Payload);
	}
}
