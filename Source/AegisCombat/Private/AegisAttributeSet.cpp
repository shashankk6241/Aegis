#include "AegisAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AegisGameplayTags.h"

UAegisAttributeSet::UAegisAttributeSet()
{
    InitHealth(100.0f);
    InitMaxHealth(100.0f);
    InitStamina(100.0f);
    InitMaxStamina(100.0f);
    InitPoise(50.0f);
    InitMaxPoise(50.0f);
    InitAttackPower(25.0f);
    InitDefense(10.0f);
}

void UAegisAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UAegisAttributeSet, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAegisAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAegisAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAegisAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAegisAttributeSet, Poise, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAegisAttributeSet, MaxPoise, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAegisAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAegisAttributeSet, Defense, COND_None, REPNOTIFY_Always);
}

void UAegisAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
    }
    else if (Attribute == GetStaminaAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
    }
    else if (Attribute == GetPoiseAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxPoise());
    }
}

void UAegisAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
    UAbilitySystemComponent* SourceASC = Context.GetOriginalInstigatorAbilitySystemComponent();
    AActor* TargetActor = Data.Target.GetOwnerActor();

    // 1. Process Physical Damage
    if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
    {
        const float LocalDamage = GetIncomingDamage();
        SetIncomingDamage(0.0f);

        if (LocalDamage > 0.0f)
        {
            const float OldHealth = GetHealth();
            const float NewHealth = FMath::Clamp(OldHealth - LocalDamage, 0.0f, GetMaxHealth());
            SetHealth(NewHealth);

            // Trigger hit reaction or death
            if (NewHealth <= 0.0f)
            {
                // Handle Death via Gameplay Tag / Event
            }
        }
    }

    // 2. Process Poise Stagger Damage
    if (Data.EvaluatedData.Attribute == GetIncomingPoiseDamageAttribute())
    {
        const float LocalPoiseDamage = GetIncomingPoiseDamage();
        SetIncomingPoiseDamage(0.0f);

        if (LocalPoiseDamage > 0.0f)
        {
            const float OldPoise = GetPoise();
            const float NewPoise = FMath::Clamp(OldPoise - LocalPoiseDamage, 0.0f, GetMaxPoise());
            SetPoise(NewPoise);

            // Check Poise Break (Stagger state)
            if (NewPoise <= 0.0f && OldPoise > 0.0f)
            {
                UAbilitySystemComponent* TargetASC = GetOwningAbilitySystemComponent();
                if (TargetASC)
                {
                    const FAegisGameplayTags& Tags = FAegisGameplayTags::Get();
                    TargetASC->AddLooseGameplayTag(Tags.State_Combat_PoiseBroken);
                    OnPoiseBroken.Broadcast(TargetActor);
                }
            }
        }
    }
}

void UAegisAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAegisAttributeSet, Health, OldHealth);
}

void UAegisAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAegisAttributeSet, MaxHealth, OldMaxHealth);
}

void UAegisAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAegisAttributeSet, Stamina, OldStamina);
}

void UAegisAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAegisAttributeSet, MaxStamina, OldMaxStamina);
}

void UAegisAttributeSet::OnRep_Poise(const FGameplayAttributeData& OldPoise)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAegisAttributeSet, Poise, OldPoise);
}

void UAegisAttributeSet::OnRep_MaxPoise(const FGameplayAttributeData& OldMaxPoise)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAegisAttributeSet, MaxPoise, OldMaxPoise);
}

void UAegisAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAegisAttributeSet, AttackPower, OldAttackPower);
}

void UAegisAttributeSet::OnRep_Defense(const FGameplayAttributeData& OldDefense)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAegisAttributeSet, Defense, OldDefense);
}
