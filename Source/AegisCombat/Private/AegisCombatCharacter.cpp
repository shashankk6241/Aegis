#include "AegisCombatCharacter.h"
#include "AegisAbilitySystemComponent.h"
#include "AegisAttributeSet.h"
#include "AegisPlayerDataAsset.h"
#include "AegisPlayerProgressionComponent.h"
#include "AegisTargetLockComponent.h"
#include "AegisPlayerDeathComponent.h"
#include "AegisWeapon.h"
#include "AegisGameplayTags.h"
#include "MotionWarpingComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"

AAegisCombatCharacter::AAegisCombatCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 1. Create Core GAS Components
	AbilitySystemComponent = CreateDefaultSubobject<UAegisAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UAegisAttributeSet>(TEXT("AttributeSet"));

	// 2. Create Motion Warping Component
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	// 3. Create RPG Player Components
	ProgressionComponent = CreateDefaultSubobject<UAegisPlayerProgressionComponent>(TEXT("ProgressionComponent"));
	TargetLockComponent = CreateDefaultSubobject<UAegisTargetLockComponent>(TEXT("TargetLockComponent"));
	DeathComponent = CreateDefaultSubobject<UAegisPlayerDeathComponent>(TEXT("DeathComponent"));
}

UAbilitySystemComponent* AAegisCombatCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// ==========================================
// Attribute Pure Getters for UI
// ==========================================

float AAegisCombatCharacter::GetHealth() const
{
	return AttributeSet ? AttributeSet->GetHealth() : 0.0f;
}

float AAegisCombatCharacter::GetMaxHealth() const
{
	return AttributeSet ? AttributeSet->GetMaxHealth() : 100.0f;
}

float AAegisCombatCharacter::GetHealthPercent() const
{
	const float MaxH = GetMaxHealth();
	return MaxH > 0.0f ? FMath::Clamp(GetHealth() / MaxH, 0.0f, 1.0f) : 0.0f;
}

float AAegisCombatCharacter::GetStamina() const
{
	return AttributeSet ? AttributeSet->GetStamina() : 0.0f;
}

float AAegisCombatCharacter::GetMaxStamina() const
{
	return AttributeSet ? AttributeSet->GetMaxStamina() : 100.0f;
}

float AAegisCombatCharacter::GetStaminaPercent() const
{
	const float MaxS = GetMaxStamina();
	return MaxS > 0.0f ? FMath::Clamp(GetStamina() / MaxS, 0.0f, 1.0f) : 0.0f;
}

float AAegisCombatCharacter::GetPoise() const
{
	return AttributeSet ? AttributeSet->GetPoise() : 0.0f;
}

float AAegisCombatCharacter::GetMaxPoise() const
{
	return AttributeSet ? AttributeSet->GetMaxPoise() : 100.0f;
}

float AAegisCombatCharacter::GetPoisePercent() const
{
	const float MaxP = GetMaxPoise();
	return MaxP > 0.0f ? FMath::Clamp(GetPoise() / MaxP, 0.0f, 1.0f) : 0.0f;
}

float AAegisCombatCharacter::GetAttackPower() const
{
	return AttributeSet ? AttributeSet->GetAttackPower() : 0.0f;
}

float AAegisCombatCharacter::GetDefense() const
{
	return AttributeSet ? AttributeSet->GetDefense() : 0.0f;
}

// ==========================================
// Progression Getters
// ==========================================

int32 AAegisCombatCharacter::GetPlayerLevel() const
{
	return ProgressionComponent ? ProgressionComponent->GetCurrentLevel() : 1;
}

int32 AAegisCombatCharacter::GetCurrentXP() const
{
	return ProgressionComponent ? ProgressionComponent->GetCurrentXP() : 0;
}

int32 AAegisCombatCharacter::GetXPForNextLevel() const
{
	return ProgressionComponent ? ProgressionComponent->GetXPForNextLevel() : 100;
}

float AAegisCombatCharacter::GetXPPercent() const
{
	return ProgressionComponent ? ProgressionComponent->GetXPPercent() : 0.0f;
}

int32 AAegisCombatCharacter::GetMaxPotions() const
{
	return PlayerDataAsset ? PlayerDataAsset->MaxPotionCharges : 3;
}

bool AAegisCombatCharacter::IsTargetLocked() const
{
	return TargetLockComponent ? TargetLockComponent->HasTarget() : false;
}

AActor* AAegisCombatCharacter::GetLockedTarget() const
{
	return TargetLockComponent ? TargetLockComponent->GetTargetActor() : nullptr;
}

bool AAegisCombatCharacter::IsDead() const
{
	return DeathComponent ? DeathComponent->IsDead() : false;
}

// ==========================================
// Initialization Pipelines
// ==========================================

void AAegisCombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitAbilityActorInfo();
	InitializeFromDataAsset();
	InitializeDefaultAttributes();
	SpawnAndEquipDefaultWeapon();

	if (AttributeSet)
	{
		AttributeSet->OnPoiseBroken.AddDynamic(this, &AAegisCombatCharacter::HandlePoiseBroken);
	}

	// Register stimuli source for AI Perception
	UAIPerceptionSystem::RegisterPerceptionStimuliSource(this, UAISense_Sight::StaticClass(), this);
	UAIPerceptionSystem::RegisterPerceptionStimuliSource(this, UAISense_Hearing::StaticClass(), this);
}

void AAegisCombatCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilityActorInfo();
	InitializeFromDataAsset();
	InitializeDefaultAttributes();
	SpawnAndEquipDefaultWeapon();
}

void AAegisCombatCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilityActorInfo();
}

void AAegisCombatCharacter::InitAbilityActorInfo()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void AAegisCombatCharacter::InitializeFromDataAsset()
{
	if (!PlayerDataAsset) return;

	// 1. Configure Progression Component
	if (ProgressionComponent)
	{
		ProgressionComponent->ConfigureFromData(PlayerDataAsset);
	}

	// 2. Configure Death Component
	if (DeathComponent)
	{
		DeathComponent->ConfigureDeath(PlayerDataAsset->DeathMontage, PlayerDataAsset->RespawnMontage);
	}

	// 3. Fallback montages from data asset
	if (!HitReactLightMontage && PlayerDataAsset->HitReactLightMontage)
	{
		HitReactLightMontage = PlayerDataAsset->HitReactLightMontage;
	}
	if (!HitReactHeavyMontage && PlayerDataAsset->HitReactHeavyMontage)
	{
		HitReactHeavyMontage = PlayerDataAsset->HitReactHeavyMontage;
	}
	if (!PoiseBreakStaggerMontage && PlayerDataAsset->StunMontage)
	{
		PoiseBreakStaggerMontage = PlayerDataAsset->StunMontage;
	}

	// 4. Default weapon from data asset
	if (!DefaultWeaponClass && PlayerDataAsset->DefaultWeaponClass)
	{
		DefaultWeaponClass = PlayerDataAsset->DefaultWeaponClass;
		WeaponAttachSocket = PlayerDataAsset->WeaponAttachSocket;
	}

	// 5. Consumables
	RemainingPotions = PlayerDataAsset->MaxPotionCharges;

	// 6. Grant Abilities from Data Asset
	if (AbilitySystemComponent && HasAuthority())
	{
		TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant;

		if (PlayerDataAsset->LightAttackAbility)
		{
			MeleeAbilityClass = PlayerDataAsset->LightAttackAbility;
			AbilitiesToGrant.AddUnique(PlayerDataAsset->LightAttackAbility);
		}
		if (PlayerDataAsset->HeavyAttackAbility)
		{
			HeavyAttackAbilityClass = PlayerDataAsset->HeavyAttackAbility;
			AbilitiesToGrant.AddUnique(PlayerDataAsset->HeavyAttackAbility);
		}
		if (PlayerDataAsset->DodgeAbility)
		{
			DodgeAbilityClass = PlayerDataAsset->DodgeAbility;
			AbilitiesToGrant.AddUnique(PlayerDataAsset->DodgeAbility);
		}
		if (PlayerDataAsset->BlockAbility)
		{
			BlockAbilityClass = PlayerDataAsset->BlockAbility;
			AbilitiesToGrant.AddUnique(PlayerDataAsset->BlockAbility);
		}
		if (PlayerDataAsset->SpecialAbility)
		{
			SpecialAbilityClass = PlayerDataAsset->SpecialAbility;
			AbilitiesToGrant.AddUnique(PlayerDataAsset->SpecialAbility);
		}
		if (PlayerDataAsset->PotionAbility)
		{
			PotionAbilityClass = PlayerDataAsset->PotionAbility;
			AbilitiesToGrant.AddUnique(PlayerDataAsset->PotionAbility);
		}

		for (const TSubclassOf<UGameplayAbility>& ExtraAbility : PlayerDataAsset->AdditionalAbilities)
		{
			if (ExtraAbility)
			{
				AbilitiesToGrant.AddUnique(ExtraAbility);
			}
		}

		AbilitySystemComponent->AddCharacterAbilities(AbilitiesToGrant);
	}
}

void AAegisCombatCharacter::InitializeDefaultAttributes()
{
	// Grant standalone default abilities if configured directly on the blueprint
	if (AbilitySystemComponent && HasAuthority())
	{
		TArray<TSubclassOf<UGameplayAbility>> AllAbilities = DefaultAbilities;
		if (MeleeAbilityClass) AllAbilities.AddUnique(MeleeAbilityClass);
		if (HeavyAttackAbilityClass) AllAbilities.AddUnique(HeavyAttackAbilityClass);
		if (DodgeAbilityClass) AllAbilities.AddUnique(DodgeAbilityClass);
		if (BlockAbilityClass) AllAbilities.AddUnique(BlockAbilityClass);
		if (SpecialAbilityClass) AllAbilities.AddUnique(SpecialAbilityClass);
		if (PotionAbilityClass) AllAbilities.AddUnique(PotionAbilityClass);

		AbilitySystemComponent->AddCharacterAbilities(AllAbilities);
	}

	// Apply default gameplay effect
	if (HasAuthority() && AbilitySystemComponent && DefaultAttributesEffect)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		Context.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributesEffect, 1.0f, Context);
		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void AAegisCombatCharacter::SpawnAndEquipDefaultWeapon()
{
	if (EquippedWeapon || !DefaultWeaponClass || !HasAuthority()) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	EquippedWeapon = GetWorld()->SpawnActor<AAegisWeapon>(DefaultWeaponClass, GetActorTransform(), SpawnParams);
	if (EquippedWeapon)
	{
		EquippedWeapon->EquipToCharacter(this, WeaponAttachSocket);
	}
}

void AAegisCombatCharacter::HandlePoiseBroken(AActor* TargetActor)
{
	if (PoiseBreakStaggerMontage)
	{
		PlayAnimMontage(PoiseBreakStaggerMontage);
	}
}

// ==========================================
// Combat Actions
// ==========================================

void AAegisCombatCharacter::PerformMeleeAttack()
{
	if (!AbilitySystemComponent) return;

	const FAegisGameplayTags& AegisTags = FAegisGameplayTags::Get();

	TSubclassOf<UGameplayAbility> AbilityToActivate = MeleeAbilityClass;
	if (!AbilityToActivate)
	{
		for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
		{
			if (Spec.Ability)
			{
				AbilityToActivate = Spec.Ability->GetClass();
				break;
			}
		}
	}

	if (!AbilityToActivate) return;

	// Ensure ability is granted
	bool bHasAbility = false;
	for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->GetClass() == AbilityToActivate)
		{
			bHasAbility = true;
			break;
		}
	}
	if (!bHasAbility && HasAuthority())
	{
		FGameplayAbilitySpec Spec(AbilityToActivate, 1);
		Spec.GetDynamicSpecSourceTags().AddTag(AegisTags.InputTag_Attack_Light);
		AbilitySystemComponent->GiveAbility(Spec);
	}

	// Buffer if already attacking
	if (AbilitySystemComponent->HasMatchingGameplayTag(AegisTags.State_Combat_CanCombo) ||
		AbilitySystemComponent->HasMatchingGameplayTag(AegisTags.State_Combat_Attacking))
	{
		if (!AbilitySystemComponent->HasMatchingGameplayTag(AegisTags.State_Combat_InputQueued))
		{
			AbilitySystemComponent->AddLooseGameplayTag(AegisTags.State_Combat_InputQueued);
		}
		return;
	}

	AbilitySystemComponent->TryActivateAbilityByClass(AbilityToActivate);
}

void AAegisCombatCharacter::PerformHeavyAttack()
{
	if (!AbilitySystemComponent || !HeavyAttackAbilityClass) return;

	const FAegisGameplayTags& AegisTags = FAegisGameplayTags::Get();
	if (AbilitySystemComponent->HasMatchingGameplayTag(AegisTags.State_Combat_Attacking))
	{
		return;
	}

	AbilitySystemComponent->TryActivateAbilityByClass(HeavyAttackAbilityClass);
}

void AAegisCombatCharacter::PerformDodge()
{
	if (!AbilitySystemComponent || !DodgeAbilityClass) return;

	// Cancel current attack cleanly to allow responsive dodging
	const FAegisGameplayTags& AegisTags = FAegisGameplayTags::Get();
	FGameplayTagContainer CancelTags(AegisTags.State_Combat_Attacking);
	AbilitySystemComponent->CancelAbilities(&CancelTags);

	AbilitySystemComponent->TryActivateAbilityByClass(DodgeAbilityClass);
}

void AAegisCombatCharacter::StartBlock()
{
	if (!AbilitySystemComponent || !BlockAbilityClass) return;

	AbilitySystemComponent->TryActivateAbilityByClass(BlockAbilityClass);
}

void AAegisCombatCharacter::EndBlock()
{
	if (!AbilitySystemComponent) return;

	const FAegisGameplayTags& AegisTags = FAegisGameplayTags::Get();
	AbilitySystemComponent->AbilityInputTagReleased(AegisTags.InputTag_Block);
}

void AAegisCombatCharacter::PerformSpecialAttack()
{
	if (!AbilitySystemComponent || !SpecialAbilityClass) return;

	AbilitySystemComponent->TryActivateAbilityByClass(SpecialAbilityClass);
}

void AAegisCombatCharacter::UsePotion()
{
	if (!AbilitySystemComponent || !PotionAbilityClass || RemainingPotions <= 0) return;

	if (AbilitySystemComponent->TryActivateAbilityByClass(PotionAbilityClass))
	{
		RemainingPotions = FMath::Max(RemainingPotions - 1, 0);
	}
}

void AAegisCombatCharacter::ToggleTargetLock()
{
	if (TargetLockComponent)
	{
		TargetLockComponent->ToggleTargetLock();
	}
}

void AAegisCombatCharacter::SwitchTargetLock(float Direction)
{
	if (TargetLockComponent)
	{
		TargetLockComponent->SwitchTarget(Direction);
	}
}

void AAegisCombatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
