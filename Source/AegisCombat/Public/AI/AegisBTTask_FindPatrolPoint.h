#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "AegisBTTask_FindPatrolPoint.generated.h"

/**
 * UAegisBTTask_FindPatrolPoint
 * BT Task that finds a random navigable point within patrol radius of the enemy's home location.
 */
UCLASS()
class AEGISCOMBAT_API UAegisBTTask_FindPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAegisBTTask_FindPatrolPoint();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Blackboard key to write the patrol destination */
	UPROPERTY(EditAnywhere, Category = "Patrol")
	FBlackboardKeySelector PatrolLocationKey;

	/** Blackboard key for the enemy's home/spawn location */
	UPROPERTY(EditAnywhere, Category = "Patrol")
	FBlackboardKeySelector HomeLocationKey;

	/** Maximum patrol radius from home location (in cm) */
	UPROPERTY(EditAnywhere, Category = "Patrol", meta = (ClampMin = "100.0"))
	float PatrolRadius = 800.0f;
};
