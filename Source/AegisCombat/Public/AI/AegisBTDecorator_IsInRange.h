#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "AegisBTDecorator_IsInRange.generated.h"

/**
 * UAegisBTDecorator_IsInRange
 * BT Decorator that checks if the AI pawn is within a specified distance of a target actor.
 */
UCLASS()
class AEGISCOMBAT_API UAegisBTDecorator_IsInRange : public UBTDecorator
{
	GENERATED_BODY()

public:
	UAegisBTDecorator_IsInRange();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

protected:
	/** Blackboard key for the target actor to measure distance to */
	UPROPERTY(EditAnywhere, Category = "Range")
	FBlackboardKeySelector TargetActorKey;

	/** Maximum acceptable range in cm */
	UPROPERTY(EditAnywhere, Category = "Range", meta = (ClampMin = "0.0"))
	float AcceptableRange = 200.0f;

	/** If true, inverts the check (returns true when NOT in range) */
	UPROPERTY(EditAnywhere, Category = "Range")
	bool bInvertCondition = false;
};
