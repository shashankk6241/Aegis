#include "AI/AegisBTDecorator_CheckAttribute.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

UAegisBTDecorator_CheckAttribute::UAegisBTDecorator_CheckAttribute()
{
	NodeName = "Check Attribute";
}

bool UAegisBTDecorator_CheckAttribute::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn())
	{
		return false;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AIController->GetPawn());
	if (!ASC || !Attribute.IsValid())
	{
		return false;
	}

	bool bFound = false;
	const float CurrentValue = ASC->GetGameplayAttributeValue(Attribute, bFound);
	if (!bFound)
	{
		return false;
	}

	switch (ComparisonOp)
	{
	case EAegisAttributeComparison::GreaterThan:    return CurrentValue > ComparisonValue;
	case EAegisAttributeComparison::GreaterOrEqual: return CurrentValue >= ComparisonValue;
	case EAegisAttributeComparison::LessThan:       return CurrentValue < ComparisonValue;
	case EAegisAttributeComparison::LessOrEqual:    return CurrentValue <= ComparisonValue;
	case EAegisAttributeComparison::Equal:          return FMath::IsNearlyEqual(CurrentValue, ComparisonValue);
	default: return false;
	}
}

FString UAegisBTDecorator_CheckAttribute::GetStaticDescription() const
{
	static const TArray<FString> OpStrings = { TEXT(">"), TEXT(">="), TEXT("<"), TEXT("<="), TEXT("==") };
	const int32 OpIndex = static_cast<int32>(ComparisonOp);
	const FString OpStr = OpStrings.IsValidIndex(OpIndex) ? OpStrings[OpIndex] : TEXT("?");

	return FString::Printf(TEXT("%s %s %.1f"),
		Attribute.IsValid() ? *Attribute.GetName() : TEXT("None"),
		*OpStr,
		ComparisonValue);
}
