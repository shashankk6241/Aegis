#include "AI/AegisBTService_UpdateCombat.h"
#include "AIController.h"
#include "AegisAggroComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UAegisBTService_UpdateCombat::UAegisBTService_UpdateCombat()
{
	NodeName = "Update Combat Data";
	Interval = 0.25f;
	RandomDeviation = 0.05f;

	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UAegisBTService_UpdateCombat, TargetActorKey), AActor::StaticClass());
	DistanceKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UAegisBTService_UpdateCombat, DistanceKey));
	HasLOSKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UAegisBTService_UpdateCombat, HasLOSKey));
	HasTargetKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UAegisBTService_UpdateCombat, HasTargetKey));
}

void UAegisBTService_UpdateCombat::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn())
	{
		return;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	UAegisAggroComponent* AggroComp = ControlledPawn->FindComponentByClass<UAegisAggroComponent>();

	AActor* Target = AggroComp ? AggroComp->GetHighestThreatTarget() : nullptr;
	const bool bHasTarget = Target != nullptr;

	const FName TargetKey = TargetActorKey.SelectedKeyName.IsNone() ? FName("TargetActor") : TargetActorKey.SelectedKeyName;
	const FName DistKey = DistanceKey.SelectedKeyName.IsNone() ? FName("DistanceToTarget") : DistanceKey.SelectedKeyName;
	const FName LOSKey = HasLOSKey.SelectedKeyName.IsNone() ? FName("HasLineOfSight") : HasLOSKey.SelectedKeyName;
	const FName HasAggroKey = HasTargetKey.SelectedKeyName.IsNone() ? FName("HasAggroTarget") : HasTargetKey.SelectedKeyName;

	BB->SetValueAsBool(HasAggroKey, bHasTarget);

	if (bHasTarget)
	{
		BB->SetValueAsObject(TargetKey, Target);

		const float Distance = FVector::Dist(ControlledPawn->GetActorLocation(), Target->GetActorLocation());
		BB->SetValueAsFloat(DistKey, Distance);
		BB->SetValueAsVector(FName("TargetLocation"), Target->GetActorLocation());

		// Refresh threat while actively targeting to prevent aggro from decaying during combat
		if (AggroComp)
		{
			AggroComp->AddThreat(Target, 2.0f);
		}

		// Line of sight check
		FVector EyeLocation;
		FRotator EyeRotation;
		ControlledPawn->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(ControlledPawn);

		const bool bHasLOS = !ControlledPawn->GetWorld()->LineTraceSingleByChannel(
			HitResult, EyeLocation, Target->GetActorLocation(),
			ECC_Visibility, QueryParams
		);
		BB->SetValueAsBool(LOSKey, bHasLOS);
	}
	else
	{
		BB->ClearValue(TargetKey);
		BB->SetValueAsFloat(DistKey, 0.0f);
		BB->SetValueAsBool(LOSKey, false);
	}
}

FString UAegisBTService_UpdateCombat::GetStaticDescription() const
{
	return TEXT("Updates blackboard with aggro target, distance, and LOS data");
}
