#include "AegisAbilitySystemComponent.h"
#include "AegisGameplayTags.h"
#include "Abilities/GameplayAbility.h"

UAegisAbilitySystemComponent::UAegisAbilitySystemComponent()
{
    SetIsReplicated(true);
}

void UAegisAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
    if (!InputTag.IsValid()) return;

    const FAegisGameplayTags& Tags = FAegisGameplayTags::Get();

    // If currently attacking and combo window is open, buffer the input!
    if (HasMatchingGameplayTag(Tags.State_Combat_CanCombo))
    {
        AddLooseGameplayTag(Tags.State_Combat_InputQueued);
        TryBufferAbility(InputTag);
        return;
    }

    // Standard immediate ability activation
    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        const bool bMatchesSpecTag = AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag);
        const bool bMatchesAssetTag = AbilitySpec.Ability && AbilitySpec.Ability->GetAssetTags().HasTagExact(InputTag);

        if (bMatchesSpecTag || bMatchesAssetTag)
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

    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        const bool bMatchesSpecTag = AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag);
        const bool bMatchesAssetTag = AbilitySpec.Ability && AbilitySpec.Ability->GetAssetTags().HasTagExact(InputTag);

        if (bMatchesSpecTag || bMatchesAssetTag)
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
            GiveAbility(AbilitySpec);
        }
    }
}
