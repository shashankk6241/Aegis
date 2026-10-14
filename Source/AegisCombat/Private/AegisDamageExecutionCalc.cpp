#include "AegisDamageExecutionCalc.h"
#include "AegisAttributeSet.h"
#include "AegisGameplayTags.h"
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

    const FAegisGameplayTags& Tags = FAegisGameplayTags::Get();

    // 0. Check Invulnerability & Dodging (I-Frames)
    if (TargetTags)
    {
        if (TargetTags->HasTagExact(Tags.State_Combat_Dodging) || 
            TargetTags->HasTagExact(Tags.State_Enemy_Invulnerable))
        {
            // Fully evaded / invulnerable — 0 damage and 0 poise damage
            return;
        }

        // Check Perfect Parry Window
        if (TargetTags->HasTagExact(Tags.State_Combat_Parrying))
        {
            // Negate all damage to the defender
            // Inflict heavy counter-poise damage to the attacker
            UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
            if (SourceASC)
            {
                SourceASC->AddLooseGameplayTag(Tags.State_Combat_PoiseBroken);
            }
            return;
        }
    }

    // 1. Capture Source Attack Power
    float AttackPower = 0.0f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef, EvaluationParameters, AttackPower);
    AttackPower = FMath::Max(AttackPower, 0.0f);

    // 2. Capture Target Defense
    float Defense = 0.0f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefenseDef, EvaluationParameters, Defense);
    Defense = FMath::Max(Defense, 0.0f);

    // 3. Armor Mitigation Formula: Damage = Attack * (100 / (100 + Defense))
    float MitigatedDamage = AttackPower * (100.0f / (100.0f + Defense));

    // 4. Calculate Poise Damage (Base 35% of attack power)
    float PoiseDamage = AttackPower * 0.35f;

    // 5. Check Blocking Mitigation (80% damage reduction)
    if (TargetTags && TargetTags->HasTagExact(Tags.State_Combat_Blocking))
    {
        MitigatedDamage *= 0.2f; // Block 80% of damage
        PoiseDamage *= 0.3f;    // Block 70% of poise impact
    }

    // 6. Output values to Meta Attributes
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
