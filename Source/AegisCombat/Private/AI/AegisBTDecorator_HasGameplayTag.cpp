#include "AI/AegisBTDecorator_HasGameplayTag.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

UAegisBTDecorator_HasGameplayTag::UAegisBTDecorator_HasGameplayTag()
{
	NodeName = "Has Gameplay Tag";
}

bool UAegisBTDecorator_HasGameplayTag::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn())
	{
		return false;
	}

	const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AIController->GetPawn());
	if (!ASC || !TagToCheck.IsValid())
	{
		return false;
	}

	const bool bHasTag = ASC->HasMatchingGameplayTag(TagToCheck);
	return bInvertCondition ? !bHasTag : bHasTag;
}

FString UAegisBTDecorator_HasGameplayTag::GetStaticDescription() const
{
	return FString::Printf(TEXT("%sHas Tag: %s"),
		bInvertCondition ? TEXT("NOT ") : TEXT(""),
		*TagToCheck.ToString());
}
