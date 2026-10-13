#include "AegisCombatCharacter.h"
#include "AegisAbilitySystemComponent.h"
#include "AegisAttributeSet.h"
#include "MotionWarpingComponent.h"
#include "AegisGameplayTags.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"

AAegisCombatCharacter::AAegisCombatCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create Ability System Component
    AbilitySystemComponent = CreateDefaultSubobject<UAegisAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    // Create Attribute Set
    AttributeSet = CreateDefaultSubobject<UAegisAttributeSet>(TEXT("AttributeSet"));

    // Create Motion Warping Component
    MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
}

UAbilitySystemComponent* AAegisCombatCharacter::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

float AAegisCombatCharacter::GetHealth() const
{
    return AttributeSet ? AttributeSet->GetHealth() : 0.0f;
}

float AAegisCombatCharacter::GetMaxHealth() const
{
    return AttributeSet ? AttributeSet->GetMaxHealth() : 100.0f;
}

float AAegisCombatCharacter::GetPoise() const
{
    return AttributeSet ? AttributeSet->GetPoise() : 0.0f;
}

float AAegisCombatCharacter::GetMaxPoise() const
{
    return AttributeSet ? AttributeSet->GetMaxPoise() : 100.0f;
}

void AAegisCombatCharacter::PerformMeleeAttack()
{
    if (!AbilitySystemComponent || !MeleeAbilityClass) return;

    const FAegisGameplayTags& AegisTags = FAegisGameplayTags::Get();

    // If we are definitely attacking, buffer the input
    if (AbilitySystemComponent->HasMatchingGameplayTag(AegisTags.State_Combat_Attacking))
    {
        if (!AbilitySystemComponent->HasMatchingGameplayTag(AegisTags.State_Combat_InputQueued))
        {
            AbilitySystemComponent->AddLooseGameplayTag(AegisTags.State_Combat_InputQueued);
        }
        return;
    }

    // Otherwise, try to start the first attack
    if (!AbilitySystemComponent->TryActivateAbilityByClass(MeleeAbilityClass))
    {
        // If activation failed, the ability is likely already running! Buffer the input as a fallback.
        if (!AbilitySystemComponent->HasMatchingGameplayTag(AegisTags.State_Combat_InputQueued))
        {
            AbilitySystemComponent->AddLooseGameplayTag(AegisTags.State_Combat_InputQueued);
        }
    }
}

void AAegisCombatCharacter::BeginPlay()
{
    Super::BeginPlay();

    InitAbilityActorInfo();
    InitializeDefaultAttributes();

    if (AbilitySystemComponent && HasAuthority())
    {
        AbilitySystemComponent->AddCharacterAbilities(DefaultAbilities);
    }

    if (AttributeSet)
    {
        AttributeSet->OnPoiseBroken.AddDynamic(this, &AAegisCombatCharacter::HandlePoiseBroken);
    }

    // Register with AI Perception system so enemies can see and hear the player
    UAIPerceptionSystem::RegisterPerceptionStimuliSource(this, UAISense_Sight::StaticClass(), this);
    UAIPerceptionSystem::RegisterPerceptionStimuliSource(this, UAISense_Hearing::StaticClass(), this);
}

void AAegisCombatCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // Server-side initialization of GAS actor info
    InitAbilityActorInfo();
    InitializeDefaultAttributes();

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->AddCharacterAbilities(DefaultAbilities);
    }
}

void AAegisCombatCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    // Client-side initialization of GAS actor info
    InitAbilityActorInfo();
}

void AAegisCombatCharacter::InitAbilityActorInfo()
{
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(this, this);
    }
}

void AAegisCombatCharacter::InitializeDefaultAttributes()
{
    if (!HasAuthority() || !AbilitySystemComponent || !DefaultAttributesEffect) return;

    FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
    Context.AddSourceObject(this);

    FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributesEffect, 1.0f, Context);
    if (SpecHandle.IsValid())
    {
        AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}

void AAegisCombatCharacter::HandlePoiseBroken(AActor* TargetActor)
{
    // Handle stagger animation montage or vulnerability cues
}

void AAegisCombatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    // Bind Enhanced Input actions to AbilitySystemComponent->AbilityInputTagPressed / Released
}
