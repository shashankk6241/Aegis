#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "AegisDamageExecutionCalc.generated.h"

/**
 * UAegisDamageExecutionCalc
 * Custom execution calculation capturing AttackPower, Defense, and Poise to calculate mitigated damage.
 */
UCLASS()
class AEGISCOMBAT_API UAegisDamageExecutionCalc : public UGameplayEffectExecutionCalculation
{
    GENERATED_BODY()

public:
    UAegisDamageExecutionCalc();

    virtual void Execute_Implementation(
        const FGameplayEffectCustomExecutionParameters& ExecutionParams,
        FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
