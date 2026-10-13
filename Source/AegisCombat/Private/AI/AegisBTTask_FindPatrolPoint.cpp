#include "AI/AegisBTTask_FindPatrolPoint.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UAegisBTTask_FindPatrolPoint::UAegisBTTask_FindPatrolPoint()
{
	NodeName = "Find Patrol Point";

	PatrolLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UAegisBTTask_FindPatrolPoint, PatrolLocationKey));
	HomeLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UAegisBTTask_FindPatrolPoint, HomeLocationKey));
}

EBTNodeResult::Type UAegisBTTask_FindPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn())
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return EBTNodeResult::Failed;
	}

	FVector OriginLocation = FVector::ZeroVector;
	if (!HomeLocationKey.SelectedKeyName.IsNone())
	{
		OriginLocation = BB->GetValueAsVector(HomeLocationKey.SelectedKeyName);
	}

	// Fallback to pawn current location if HomeLocation was not set or is at origin (0,0,0)
	if (OriginLocation.IsNearlyZero())
	{
		OriginLocation = AIController->GetPawn()->GetActorLocation();
		const FName HomeKey = HomeLocationKey.SelectedKeyName.IsNone() ? FName("HomeLocation") : HomeLocationKey.SelectedKeyName;
		BB->SetValueAsVector(HomeKey, OriginLocation);
	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AegisAI] FindPatrolPoint: No NavigationSystem found! Did you add a NavMeshBoundsVolume to the level?"));
		return EBTNodeResult::Failed;
	}

	FNavLocation ResultLocation;
	const bool bFound = NavSys->GetRandomReachablePointInRadius(OriginLocation, PatrolRadius, ResultLocation);

	if (bFound)
	{
		const FName PatrolKey = PatrolLocationKey.SelectedKeyName.IsNone() ? FName("PatrolLocation") : PatrolLocationKey.SelectedKeyName;
		BB->SetValueAsVector(PatrolKey, ResultLocation.Location);
		return EBTNodeResult::Succeeded;
	}

	UE_LOG(LogTemp, Warning, TEXT("[AegisAI] FindPatrolPoint: Could not find reachable point within radius %.0f around (%.0f, %.0f, %.0f). Check that NavMesh covers this area (press P)."),
		PatrolRadius, OriginLocation.X, OriginLocation.Y, OriginLocation.Z);
	return EBTNodeResult::Failed;
}

FString UAegisBTTask_FindPatrolPoint::GetStaticDescription() const
{
	return FString::Printf(TEXT("Find random patrol point within %.0f cm of home"), PatrolRadius);
}
