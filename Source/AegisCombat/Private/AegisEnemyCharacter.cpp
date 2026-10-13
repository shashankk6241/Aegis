#include "AegisEnemyCharacter.h"
#include "AegisAbilitySystemComponent.h"
#include "AegisAttributeSet.h"
#include "AegisEnemyDataAsset.h"
#include "AegisAggroComponent.h"
#include "AegisDeathHandlerComponent.h"
#include "AegisEnemyAIController.h"
#include "AegisEnemyScalingFunctionLibrary.h"
#include "AegisGameplayTags.h"
#include "UI/AegisEnemyWidgetComponent.h"
#include "UI/AegisEnemyHealthBarWidget.h"

AAegisEnemyCharacter::AAegisEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// Override AI Controller class
	AIControllerClass = AAegisEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// GAS Components
	AbilitySystemComponent = CreateDefaultSubobject<UAegisAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UAegisAttributeSet>(TEXT("AttributeSet"));

	// RPG Components
	AggroComponent = CreateDefaultSubobject<UAegisAggroComponent>(TEXT("AggroComponent"));
	DeathHandlerComponent = CreateDefaultSubobject<UAegisDeathHandlerComponent>(TEXT("DeathHandlerComponent"));

	// Health Bar Widget
	HealthBarWidgetComponent = CreateDefaultSubobject<UAegisEnemyWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidgetComponent->SetupAttachment(RootComponent);
	HealthBarWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
}

UAbilitySystemComponent* AAegisEnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

FText AAegisEnemyCharacter::GetEnemyName() const
{
	if (EnemyDataAsset)
	{
		return EnemyDataAsset->DisplayName;
	}
	return FText::FromString(GetName());
}

EAegisEnemyArchetype AAegisEnemyCharacter::GetArchetype() const
{
	if (EnemyDataAsset)
	{
		return EnemyDataAsset->Archetype;
	}
	return EAegisEnemyArchetype::Melee_Grunt;
}

bool AAegisEnemyCharacter::IsDead() const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->HasMatchingGameplayTag(FAegisGameplayTags::Get().State_AI_Dead);
	}
	return false;
}

void AAegisEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize GAS
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	// Determine enemy level
	EnemyLevel = EnemyLevelOverride > 0 ? EnemyLevelOverride : (EnemyDataAsset ? EnemyDataAsset->DefaultLevel : 1);

	// Initialize from data asset
	InitializeFromDataAsset();

	// Bind attribute events
	if (AttributeSet)
	{
		AttributeSet->OnPoiseBroken.AddDynamic(this, &AAegisEnemyCharacter::HandlePoiseBroken);
		AttributeSet->OnHealthChanged.AddDynamic(this, &AAegisEnemyCharacter::OnHealthChanged);
	}

	// Bind death event
	if (DeathHandlerComponent)
	{
		DeathHandlerComponent->OnEnemyDied.AddDynamic(this, &AAegisEnemyCharacter::HandleDeath);
	}
}

void AAegisEnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitializeAI();
}

void AAegisEnemyCharacter::InitializeFromDataAsset()
{
	if (!EnemyDataAsset) return;

	// Apply data asset montages as fallbacks
	if (!HitReactLightMontage && EnemyDataAsset->HitReactLightMontage)
	{
		HitReactLightMontage = EnemyDataAsset->HitReactLightMontage;
	}
	if (!HitReactHeavyMontage && EnemyDataAsset->HitReactHeavyMontage)
	{
		HitReactHeavyMontage = EnemyDataAsset->HitReactHeavyMontage;
	}
	if (!PoiseBreakStaggerMontage && EnemyDataAsset->StaggerMontage)
	{
		PoiseBreakStaggerMontage = EnemyDataAsset->StaggerMontage;
	}

	// Configure death handler from data asset
	if (DeathHandlerComponent)
	{
		DeathHandlerComponent->ConfigureFromData(
			EnemyDataAsset->DeathMontage,
			EnemyDataAsset->XPReward,
			EnemyDataAsset->LootTable
		);
	}

	// Configure aggro ranges
	if (AggroComponent)
	{
		// MaxAggroDistance comes from the data asset's LoseAggroRange
		// This is also configured on the perception component via the AI controller
	}

	InitializeAttributes();
	InitializeAbilities();
	InitializeAI();
	InitializeHealthBar();
}

void AAegisEnemyCharacter::InitializeAttributes()
{
	if (!AbilitySystemComponent || !EnemyDataAsset) return;

	// Apply the default attributes GE if available
	if (EnemyDataAsset->DefaultAttributesEffect && HasAuthority())
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		Context.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
			EnemyDataAsset->DefaultAttributesEffect, static_cast<float>(EnemyLevel), Context);
		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	// Override with level-scaled values from data asset
	if (AttributeSet)
	{
		float ScaledHealth, ScaledStamina, ScaledPoise, ScaledAttack, ScaledDefense;
		UAegisEnemyScalingFunctionLibrary::GetScaledStatsFromDataAsset(
			EnemyDataAsset, EnemyLevel,
			ScaledHealth, ScaledStamina, ScaledPoise, ScaledAttack, ScaledDefense
		);

		AttributeSet->InitHealth(ScaledHealth);
		AttributeSet->InitMaxHealth(ScaledHealth);
		AttributeSet->InitStamina(ScaledStamina);
		AttributeSet->InitMaxStamina(ScaledStamina);
		AttributeSet->InitPoise(ScaledPoise);
		AttributeSet->InitMaxPoise(ScaledPoise);
		AttributeSet->InitAttackPower(ScaledAttack);
		AttributeSet->InitDefense(ScaledDefense);
	}
}

void AAegisEnemyCharacter::InitializeAbilities()
{
	if (!AbilitySystemComponent || !EnemyDataAsset || !HasAuthority()) return;

	AbilitySystemComponent->AddCharacterAbilities(EnemyDataAsset->GrantedAbilities);
}

void AAegisEnemyCharacter::InitializeAI()
{
	if (!EnemyDataAsset) return;

	// Start the behavior tree via the AI controller
	AAegisEnemyAIController* AIController = Cast<AAegisEnemyAIController>(GetController());
	if (AIController)
	{
		AIController->StartBehaviorTreeFromDataAsset(EnemyDataAsset);
	}
}

void AAegisEnemyCharacter::InitializeHealthBar()
{
	if (!HealthBarWidgetComponent) return;

	UAegisEnemyHealthBarWidget* HealthBar = HealthBarWidgetComponent->GetHealthBarWidget();
	if (HealthBar)
	{
		HealthBar->SetEnemyName(GetEnemyName());
		HealthBar->SetEnemyLevel(EnemyLevel);
		HealthBar->SetHealthPercent(1.0f);
		HealthBar->SetPoisePercent(1.0f);

		if (EnemyDataAsset)
		{
			HealthBar->SetArchetypeStyle(EnemyDataAsset->Archetype == EAegisEnemyArchetype::Boss);
		}
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
	// Update health bar
	if (HealthBarWidgetComponent && AttributeSet)
	{
		const float MaxHealth = AttributeSet->GetMaxHealth();
		const float HealthPercent = MaxHealth > 0.0f ? NewValue / MaxHealth : 0.0f;

		UAegisEnemyHealthBarWidget* HealthBar = HealthBarWidgetComponent->GetHealthBarWidget();
		if (HealthBar)
		{
			HealthBar->SetHealthPercent(HealthPercent);
			HealthBar->SetPoisePercent(
				AttributeSet->GetMaxPoise() > 0.0f
				? AttributeSet->GetPoise() / AttributeSet->GetMaxPoise()
				: 0.0f
			);
		}

		HealthBarWidgetComponent->ShowHealthBar();
	}

	// Add aggro for damage dealers
	if (AggroComponent && InstigatorActor)
	{
		const float DamageDealt = OldValue - NewValue;
		if (DamageDealt > 0.0f)
		{
			AggroComponent->AddThreat(InstigatorActor, DamageDealt * 2.0f);
		}
	}

	// Play hit react
	if (NewValue < OldValue && NewValue > 0.0f)
	{
		const float DamageRatio = (OldValue - NewValue) / FMath::Max(AttributeSet->GetMaxHealth(), 1.0f);
		if (DamageRatio > 0.15f && HitReactHeavyMontage)
		{
			PlayAnimMontage(HitReactHeavyMontage);
		}
		else if (HitReactLightMontage)
		{
			PlayAnimMontage(HitReactLightMontage);
		}
	}

	// Check for death
	if (NewValue <= 0.0f && DeathHandlerComponent && !DeathHandlerComponent->IsDeathInProgress())
	{
		DeathHandlerComponent->ExecuteDeathFlow(InstigatorActor);
	}

	// Check boss phase transitions
	if (EnemyDataAsset && EnemyDataAsset->Archetype == EAegisEnemyArchetype::Boss)
	{
		const float MaxHealth = AttributeSet ? AttributeSet->GetMaxHealth() : 1.0f;
		const float HealthPercent = MaxHealth > 0.0f ? NewValue / MaxHealth : 0.0f;
		CheckPhaseTransition(HealthPercent);
	}
}

void AAegisEnemyCharacter::HandleDeath(AActor* DeadEnemy, AActor* Killer, int32 XPReward)
{
	// Clear aggro on death
	if (AggroComponent)
	{
		AggroComponent->ClearAllAggro();
	}

	// Hide health bar
	if (HealthBarWidgetComponent)
	{
		HealthBarWidgetComponent->HideHealthBar();
	}
}

void AAegisEnemyCharacter::CheckPhaseTransition(float HealthPercent)
{
	if (!EnemyDataAsset || !AbilitySystemComponent) return;

	const TArray<float>& Thresholds = EnemyDataAsset->PhaseThresholds;
	const TArray<TSubclassOf<UGameplayAbility>>& PhaseAbilities = EnemyDataAsset->PhaseAbilities;

	for (int32 i = CurrentBossPhase; i < Thresholds.Num(); ++i)
	{
		if (HealthPercent <= Thresholds[i])
		{
			CurrentBossPhase = i + 1;

			// Grant the phase ability if available
			if (PhaseAbilities.IsValidIndex(i) && PhaseAbilities[i] && HasAuthority())
			{
				FGameplayAbilitySpec AbilitySpec(PhaseAbilities[i], 1);
				AbilitySystemComponent->GiveAbility(AbilitySpec);
				AbilitySystemComponent->TryActivateAbilityByClass(PhaseAbilities[i]);
			}

			OnPhaseChanged.Broadcast(this, CurrentBossPhase);
			break; // Only transition one phase at a time
		}
	}
}
