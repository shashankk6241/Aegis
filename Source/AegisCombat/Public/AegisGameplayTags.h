#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * FAegisGameplayTags
 * Singleton struct managing native gameplay tags for type-safe GAS access.
 */
struct FAegisGameplayTags
{
public:
    static const FAegisGameplayTags& Get() { return GameplayTags; }
    static void InitializeNativeGameplayTags();

    // ==========================================
    // State Tags
    // ==========================================
    FGameplayTag State_Combat_Attacking;
    FGameplayTag State_Combat_CanCombo;
    FGameplayTag State_Combat_InputQueued;
    FGameplayTag State_Combat_PoiseBroken;
    FGameplayTag State_Combat_Dodging;
    FGameplayTag State_Combat_Blocking;

    // ==========================================
    // Ability / Input Tags
    // ==========================================
    FGameplayTag InputTag_Attack_Light;
    FGameplayTag InputTag_Attack_Heavy;
    FGameplayTag InputTag_Dodge;
    FGameplayTag InputTag_Block;

    // ==========================================
    // Damage & Effect Tags
    // ==========================================
    FGameplayTag Damage_Physical;
    FGameplayTag Damage_Poise;
    FGameplayTag Effect_HitReact_Light;
    FGameplayTag Effect_HitReact_Heavy;
    FGameplayTag Effect_HitReact_Knockdown;

    // ==========================================
    // Gameplay Cue Tags (Audio & Niagara VFX)
    // ==========================================
    FGameplayTag GameplayCue_Combat_HitImpact;
    FGameplayTag GameplayCue_Combat_PoiseBreak;
    FGameplayTag GameplayCue_Combat_WeaponTrail;

private:
    static FAegisGameplayTags GameplayTags;
};
