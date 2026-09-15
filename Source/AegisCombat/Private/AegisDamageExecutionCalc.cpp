#include "AegisDamageExecutionCalc.h"
#include "AegisAttributeSet.h"
#include "AbilitySystemComponent.h"

// Struct declaring captured attributes
struct FAegisDamageStatics
{
    DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
    DECLARE_ATTRIBUTE_CAPTUREDEF(Defense);
    DECLARE_ATTRIBUTE_CAPTUREDEF(Poise);

    FAegisDamageStatics()
    {
        // Capture Source Attack Power
        DEFINE_ATTRIBUTE_CAPTUREDEF(UAegisAttributeSet, AttackPower, Source, false);

        // Capture Target Defense and Poise
        DEFINE_ATTRIBUTE_CAPTUREDEF(UAegisAttributeSet, Defense, Target, false);
        DEFINE_ATTRIBUTE_CAPTUREDEF(UAegisAttributeSet, Poise, Target, false);
    }
};

static const FAegisDamageStatics& DamageStatics()
{
    static FAegisDamageStatics DStatics;
    return DStatics;
}

UAegisDamageExecutionCalc::UAegisDamageExecutionCalc()
{
    RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
    RelevantAttributesToCapture.Add(DamageStatics().DefenseDef);
    RelevantAttributesToCapture.Add(DamageStatics().PoiseDef);
}

void UAegisDamageExecutionCalc::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams,
    FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

    const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = SourceTags;
    EvaluationParameters.TargetTags = TargetTags;

    // 1. Capture Source Attack Power
    float AttackPower = 0.0f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef, EvaluationParameters, AttackPower);
    AttackPower = FMath::Max(AttackPower, 0.0f);

    // 2. Capture Target Defense
    float Defense = 0.0f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefenseDef, EvaluationParameters, Defense);
    Defense = FMath::Max(Defense, 0.0f);

    // 3. Armor Mitigation Formula: Damage = Attack * (100 / (100 + Defense))
    const float MitigatedDamage = AttackPower * (100.0f / (100.0f + Defense));

    // 4. Calculate Poise Damage (Base 35% of attack power)
    const float PoiseDamage = AttackPower * 0.35f;

    // 5. Output values to Meta Attributes
    OutExecutionOutput.AddOutputModifier(
        FGameplayModifierEvaluatedData(
            UAegisAttributeSet::GetIncomingDamageAttribute(),
            EGameplayModOp::Additive,
            MitigatedDamage
        )
    );

    OutExecutionOutput.AddOutputModifier(
        FGameplayModifierEvaluatedData(
            UAegisAttributeSet::GetIncomingPoiseDamageAttribute(),
            EGameplayModOp::Additive,
            PoiseDamage
        )
    );
}
