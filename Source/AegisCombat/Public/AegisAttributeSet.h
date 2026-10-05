#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AegisAttributeSet.generated.h"

// Macro defining standard getter/setter accessors for Gameplay Attributes
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnAttributeChangedDelegate, AActor*, Instigator, AActor*, Target, float, OldValue, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPoiseBrokenDelegate, AActor*, TargetActor);

/**
 * UAegisAttributeSet
 * Core attribute container managing Health, Stamina, Poise, and Combat Statistics.
 */
UCLASS()
class AEGISCOMBAT_API UAegisAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    UAegisAttributeSet();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

    // ==========================================
    // Health Attributes
    // ==========================================
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health", ReplicatedUsing = OnRep_Health)
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(UAegisAttributeSet, Health);

    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health", ReplicatedUsing = OnRep_MaxHealth)
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(UAegisAttributeSet, MaxHealth);

    // ==========================================
    // Stamina Attributes
    // ==========================================
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stamina", ReplicatedUsing = OnRep_Stamina)
    FGameplayAttributeData Stamina;
    ATTRIBUTE_ACCESSORS(UAegisAttributeSet, Stamina);

    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stamina", ReplicatedUsing = OnRep_MaxStamina)
    FGameplayAttributeData MaxStamina;
    ATTRIBUTE_ACCESSORS(UAegisAttributeSet, MaxStamina);

    // ==========================================
    // Poise Attributes (Stagger & Break)
    // ==========================================
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Poise", ReplicatedUsing = OnRep_Poise)
    FGameplayAttributeData Poise;
    ATTRIBUTE_ACCESSORS(UAegisAttributeSet, Poise);

    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Poise", ReplicatedUsing = OnRep_MaxPoise)
    FGameplayAttributeData MaxPoise;
    ATTRIBUTE_ACCESSORS(UAegisAttributeSet, MaxPoise);

    // ==========================================
    // Combat Scaling
    // ==========================================
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_AttackPower)
    FGameplayAttributeData AttackPower;
    ATTRIBUTE_ACCESSORS(UAegisAttributeSet, AttackPower);

    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_Defense)
    FGameplayAttributeData Defense;
    ATTRIBUTE_ACCESSORS(UAegisAttributeSet, Defense);

    // ==========================================
    // Meta Attributes (Server-side calculations)
    // ==========================================
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
    FGameplayAttributeData IncomingDamage;
    ATTRIBUTE_ACCESSORS(UAegisAttributeSet, IncomingDamage);

    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
    FGameplayAttributeData IncomingPoiseDamage;
    ATTRIBUTE_ACCESSORS(UAegisAttributeSet, IncomingPoiseDamage);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPoiseBrokenDelegate OnPoiseBroken;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAttributeChangedDelegate OnHealthChanged;

protected:
    UFUNCTION()
    virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

    UFUNCTION()
    virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

    UFUNCTION()
    virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

    UFUNCTION()
    virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);

    UFUNCTION()
    virtual void OnRep_Poise(const FGameplayAttributeData& OldPoise);

    UFUNCTION()
    virtual void OnRep_MaxPoise(const FGameplayAttributeData& OldMaxPoise);

    UFUNCTION()
    virtual void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);

    UFUNCTION()
    virtual void OnRep_Defense(const FGameplayAttributeData& OldDefense);
};
