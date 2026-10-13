#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "AttributeSet.h"
#include "AegisBTDecorator_CheckAttribute.generated.h"

/** Comparison operators for attribute threshold checks */
UENUM(BlueprintType)
enum class EAegisAttributeComparison : uint8
{
	GreaterThan     UMETA(DisplayName = ">"),
	GreaterOrEqual  UMETA(DisplayName = ">="),
	LessThan        UMETA(DisplayName = "<"),
	LessOrEqual     UMETA(DisplayName = "<="),
	Equal           UMETA(DisplayName = "==")
};

/**
 * UAegisBTDecorator_CheckAttribute
 * BT Decorator that checks if a GAS attribute on the AI pawn meets a threshold condition.
 * Useful for health checks, stamina gates, etc.
 */
UCLASS()
class AEGISCOMBAT_API UAegisBTDecorator_CheckAttribute : public UBTDecorator
{
	GENERATED_BODY()

public:
	UAegisBTDecorator_CheckAttribute();

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;

protected:
	/** The gameplay attribute to check */
	UPROPERTY(EditAnywhere, Category = "Attribute")
	FGameplayAttribute Attribute;

	/** Comparison operator */
	UPROPERTY(EditAnywhere, Category = "Attribute")
	EAegisAttributeComparison ComparisonOp = EAegisAttributeComparison::GreaterThan;

	/** Value to compare against */
	UPROPERTY(EditAnywhere, Category = "Attribute")
	float ComparisonValue = 0.0f;
};
