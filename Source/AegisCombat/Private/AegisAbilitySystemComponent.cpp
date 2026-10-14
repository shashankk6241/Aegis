#include "AegisAbilitySystemComponent.h"
#include "AegisGameplayTags.h"
#include "AegisGameplayAbility_Melee.h"
#include "Abilities/GameplayAbility.h"

UAegisAbilitySystemComponent::UAegisAbilitySystemComponent()
{
    SetIsReplicated(true);
}

void UAegisAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
    if (!InputTag.IsValid()) return;

    const FAegisGameplayTags& Tags = FAegisGameplayTags::Get();

    // If currently attacking and combo window is open or already attacking, buffer the input!
    if (HasMatchingGameplayTag(Tags.State_Combat_CanCombo) || HasMatchingGameplayTag(Tags.State_Combat_Attacking))
    {
        if (!HasMatchingGameplayTag(Tags.State_Combat_InputQueued))
        {
            AddLooseGameplayTag(Tags.State_Combat_InputQueued);
        }
        TryBufferAbility(InputTag);
        return;
    }

    // Standard immediate ability activation
    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        const bool bMatchesSpecTag = AbilitySpec.GetDynamicSpecSourceTags().HasTag(InputTag);
        const bool bMatchesAssetTag = AbilitySpec.Ability && AbilitySpec.Ability->GetAssetTags().HasTag(InputTag);
        const bool bIsMeleeFallback = (InputTag.MatchesTag(Tags.InputTag_Attack_Light) || InputTag.MatchesTag(Tags.InputTag_Attack_Heavy)) &&
            AbilitySpec.Ability && AbilitySpec.Ability->GetClass()->IsChildOf(UAegisGameplayAbility_Melee::StaticClass());

        if (bMatchesSpecTag || bMatchesAssetTag || bIsMeleeFallback)
        {
            AbilitySpecInputPressed(AbilitySpec);
            if (!AbilitySpec.IsActive())
            {
                TryActivateAbility(AbilitySpec.Handle);
            }
        }
    }
}

void UAegisAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
    if (!InputTag.IsValid()) return;

    const FAegisGameplayTags& Tags = FAegisGameplayTags::Get();

    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        const bool bMatchesSpecTag = AbilitySpec.GetDynamicSpecSourceTags().HasTag(InputTag);
        const bool bMatchesAssetTag = AbilitySpec.Ability && AbilitySpec.Ability->GetAssetTags().HasTag(InputTag);
        const bool bIsMeleeFallback = (InputTag.MatchesTag(Tags.InputTag_Attack_Light) || InputTag.MatchesTag(Tags.InputTag_Attack_Heavy)) &&
            AbilitySpec.Ability && AbilitySpec.Ability->GetClass()->IsChildOf(UAegisGameplayAbility_Melee::StaticClass());

        if (bMatchesSpecTag || bMatchesAssetTag || bIsMeleeFallback)
        {
            AbilitySpecInputReleased(AbilitySpec);
        }
    }
}

void UAegisAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
    if (BufferTimer > 0.0f)
    {
        BufferTimer -= DeltaTime;
        if (BufferTimer <= 0.0f)
        {
            ClearBufferedInput();
        }
    }
}

bool UAegisAbilitySystemComponent::TryBufferAbility(const FGameplayTag& InputTag)
{
    BufferedInputTag = InputTag;
    BufferTimer = MaxBufferWindow;
    return true;
}

void UAegisAbilitySystemComponent::ClearBufferedInput()
{
    BufferedInputTag = FGameplayTag::EmptyTag;
    BufferTimer = 0.0f;
    RemoveLooseGameplayTag(FAegisGameplayTags::Get().State_Combat_InputQueued);
}

void UAegisAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
    if (!GetOwner()->HasAuthority()) return;

    const FAegisGameplayTags& Tags = FAegisGameplayTags::Get();

    for (const TSubclassOf<UGameplayAbility>& AbilityClass : StartupAbilities)
    {
        if (AbilityClass)
        {
            bool bAlreadyHas = false;
            for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
            {
                if (Spec.Ability && Spec.Ability->GetClass() == AbilityClass)
                {
                    bAlreadyHas = true;
                    break;
                }
            }
            if (bAlreadyHas) continue;

            FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
            if (const UGameplayAbility* CDO = AbilityClass->GetDefaultObject<UGameplayAbility>())
            {
                AbilitySpec.GetDynamicSpecSourceTags().AppendTags(CDO->GetAssetTags());
            }

            // Ensure Melee abilities have the light attack input tag associated
            if (AbilityClass->IsChildOf(UAegisGameplayAbility_Melee::StaticClass()))
            {
                if (Tags.InputTag_Attack_Light.IsValid())
                {
                    AbilitySpec.GetDynamicSpecSourceTags().AddTag(Tags.InputTag_Attack_Light);
                }
            }

            GiveAbility(AbilitySpec);
        }
    }
}
