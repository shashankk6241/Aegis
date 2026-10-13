#include "AI/AegisBTDecorator_IsInRange.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UAegisBTDecorator_IsInRange::UAegisBTDecorator_IsInRange()
{
	NodeName = "Is In Range";
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UAegisBTDecorator_IsInRange, TargetActorKey), AActor::StaticClass());
}

bool UAegisBTDecorator_IsInRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	const AAIController* AIController = OwnerComp.GetAIOwner();
	if (!BB || !AIController || !AIController->GetPawn())
	{
		return false;
	}

	const FName KeyName = TargetActorKey.SelectedKeyName.IsNone() ? FName("TargetActor") : TargetActorKey.SelectedKeyName;
	const AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(KeyName));
	if (!TargetActor)
	{
		return false;
	}

	const float Distance = FVector::Dist(AIController->GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());
	const bool bInRange = Distance <= AcceptableRange;

	return bInvertCondition ? !bInRange : bInRange;
}

FString UAegisBTDecorator_IsInRange::GetStaticDescription() const
{
	return FString::Printf(TEXT("%sIn Range (%.0f cm) of %s"),
		bInvertCondition ? TEXT("NOT ") : TEXT(""),
		AcceptableRange,
		*TargetActorKey.SelectedKeyName.ToString());
}
