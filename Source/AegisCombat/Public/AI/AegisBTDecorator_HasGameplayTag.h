#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "GameplayTagContainer.h"
#include "AegisBTDecorator_HasGameplayTag.generated.h"

/**
 * UAegisBTDecorator_HasGameplayTag
 * BT Decorator that checks if the AI pawn's AbilitySystemComponent has a specific gameplay tag.
 */
UCLASS()
class AEGISCOMBAT_API UAegisBTDecorator_HasGameplayTag : public UBTDecorator
{
	GENERATED_BODY()

public:
	UAegisBTDecorator_HasGameplayTag();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

protected:
	/** The gameplay tag to check for on the pawn's ASC */
	UPROPERTY(EditAnywhere, Category = "GameplayTag")
	FGameplayTag TagToCheck;

	/** If true, condition passes when the tag is NOT present */
	UPROPERTY(EditAnywhere, Category = "GameplayTag")
	bool bInvertCondition = false;
};
