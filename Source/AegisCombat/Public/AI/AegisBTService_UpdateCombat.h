#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "AegisBTService_UpdateCombat.generated.h"

/**
 * UAegisBTService_UpdateCombat
 * BT Service that periodically updates blackboard keys with aggro target,
 * distance, and line-of-sight data from the AegisAggroComponent.
 */
UCLASS()
class AEGISCOMBAT_API UAegisBTService_UpdateCombat : public UBTService
{
	GENERATED_BODY()

public:
	UAegisBTService_UpdateCombat();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;

	/** Key for the current aggro target actor */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	/** Key for distance to target (float) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DistanceKey;

	/** Key for whether we have LOS to target (bool) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasLOSKey;

	/** Key for whether we have any aggro target (bool) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasTargetKey;
};
