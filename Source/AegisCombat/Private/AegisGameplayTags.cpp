#include "AegisGameplayTags.h"
#include "GameplayTagsManager.h"

FAegisGameplayTags FAegisGameplayTags::GameplayTags;

void FAegisGameplayTags::InitializeNativeGameplayTags()
{
    UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

    // Combat States
    GameplayTags.State_Combat_Attacking = Manager.AddNativeGameplayTag(
        FName("State.Combat.Attacking"),
        FString("Entity is currently in an active attack animation window.")
    );

    GameplayTags.State_Combat_CanCombo = Manager.AddNativeGameplayTag(
        FName("State.Combat.CanCombo"),
        FString("Combo window open: Next attack input can be queued.")
    );

    GameplayTags.State_Combat_InputQueued = Manager.AddNativeGameplayTag(
        FName("State.Combat.InputQueued"),
        FString("Next attack input has been buffered.")
    );

    GameplayTags.State_Combat_PoiseBroken = Manager.AddNativeGameplayTag(
        FName("State.Combat.PoiseBroken"),
        FString("Target poise has been broken and is vulnerable to critical execution.")
    );

    GameplayTags.State_Combat_Dodging = Manager.AddNativeGameplayTag(
        FName("State.Combat.Dodging"),
        FString("Entity is dodging (granting invulnerability frames).")
    );

    GameplayTags.State_Combat_Blocking = Manager.AddNativeGameplayTag(
        FName("State.Combat.Blocking"),
        FString("Entity is actively blocking incoming strikes.")
    );

    // Inputs
    GameplayTags.InputTag_Attack_Light = Manager.AddNativeGameplayTag(
        FName("InputTag.Attack.Light"),
        FString("Light attack primary input.")
    );

    GameplayTags.InputTag_Attack_Heavy = Manager.AddNativeGameplayTag(
        FName("InputTag.Attack.Heavy"),
        FString("Heavy attack charged input.")
    );

    GameplayTags.InputTag_Dodge = Manager.AddNativeGameplayTag(
        FName("InputTag.Dodge"),
        FString("Dodge roll input.")
    );

    GameplayTags.InputTag_Block = Manager.AddNativeGameplayTag(
        FName("InputTag.Block"),
        FString("Block / parry input.")
    );

    // Damage & Effects
    GameplayTags.Damage_Physical = Manager.AddNativeGameplayTag(
        FName("Damage.Physical"),
        FString("Standard physical health damage.")
    );

    GameplayTags.Damage_Poise = Manager.AddNativeGameplayTag(
        FName("Damage.Poise"),
        FString("Stagger / poise reduction damage.")
    );

    GameplayTags.Effect_HitReact_Light = Manager.AddNativeGameplayTag(
        FName("Effect.HitReact.Light"),
        FString("Light flinch reaction.")
    );

    GameplayTags.Effect_HitReact_Heavy = Manager.AddNativeGameplayTag(
        FName("Effect.HitReact.Heavy"),
        FString("Heavy stagger reaction.")
    );

    GameplayTags.Effect_HitReact_Knockdown = Manager.AddNativeGameplayTag(
        FName("Effect.HitReact.Knockdown"),
        FString("Full knockdown ragdoll effect.")
    );

    // Cues
    GameplayTags.GameplayCue_Combat_HitImpact = Manager.AddNativeGameplayTag(
        FName("GameplayCue.Combat.HitImpact"),
        FString("Spawns directional sparks, flesh hit sound, and frame hit-stop.")
    );

    GameplayTags.GameplayCue_Combat_PoiseBreak = Manager.AddNativeGameplayTag(
        FName("GameplayCue.Combat.PoiseBreak"),
        FString("Shatter sound and visual aura when enemy poise breaks.")
    );

    GameplayTags.GameplayCue_Combat_WeaponTrail = Manager.AddNativeGameplayTag(
        FName("GameplayCue.Combat.WeaponTrail"),
        FString("Ribbon trail following weapon edge.")
    );
}
