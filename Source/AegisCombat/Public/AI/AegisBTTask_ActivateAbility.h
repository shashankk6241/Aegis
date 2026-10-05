#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AegisBTTask_ActivateAbility.generated.h"

class UGameplayAbility;

/**
 * UAegisBTTask_ActivateAbility
 * BT Task node that activates a GAS ability on the AI pawn.
 * Optionally waits for the ability to finish before returning success.
 */
UCLASS()
class AEGISCOMBAT_API UAegisBTTask_ActivateAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAegisBTTask_ActivateAbility();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual FString GetStaticDescription() const override;

protected:
	/** The ability class to activate */
	UPROPERTY(EditAnywhere, Category = "Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;

	/** If true, waits for the ability to end before returning success */
	UPROPERTY(EditAnywhere, Category = "Ability")
	bool bWaitForAbilityEnd = true;

	/** Maximum time to wait for ability completion before failing */
	UPROPERTY(EditAnywhere, Category = "Ability", meta = (EditCondition = "bWaitForAbilityEnd", ClampMin = "0.1"))
	float MaxWaitTime = 5.0f;

private:
	float ElapsedTime = 0.0f;
};
