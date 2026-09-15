#include "AegisEnemyCharacter.h"
#include "AegisAbilitySystemComponent.h"
#include "AegisAttributeSet.h"
#include "AegisGameplayTags.h"

AAegisEnemyCharacter::AAegisEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<UAegisAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UAegisAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AAegisEnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAegisEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	if (AttributeSet)
	{
		AttributeSet->OnPoiseBroken.AddDynamic(this, &AAegisEnemyCharacter::HandlePoiseBroken);
	}
}

void AAegisEnemyCharacter::HandlePoiseBroken(AActor* TargetActor)
{
	if (PoiseBreakStaggerMontage)
	{
		PlayAnimMontage(PoiseBreakStaggerMontage);
	}
}

void AAegisEnemyCharacter::OnHealthChanged(AActor* InstigatorActor, AActor* TargetActor, float OldValue, float NewValue)
{
	if (NewValue < OldValue && HitReactLightMontage)
	{
		PlayAnimMontage(HitReactLightMontage);
	}
}
