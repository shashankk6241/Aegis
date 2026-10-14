#include "AegisPlayerProgressionComponent.h"
#include "AegisPlayerDataAsset.h"
#include "AegisAttributeSet.h"
#include "AegisGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

UAegisPlayerProgressionComponent::UAegisPlayerProgressionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAegisPlayerProgressionComponent::BeginPlay()
{
	Super::BeginPlay();
	UpdateRequiredXP();
}

void UAegisPlayerProgressionComponent::ConfigureFromData(const UAegisPlayerDataAsset* InDataAsset)
{
	PlayerDataAsset = InDataAsset;
	UpdateRequiredXP();
	RecalculateAttributes();
}

void UAegisPlayerProgressionComponent::UpdateRequiredXP()
{
	if (PlayerDataAsset)
	{
		XPForNextLevel = PlayerDataAsset->CalculateXPForLevel(CurrentLevel);
	}
	else
	{
		XPForNextLevel = FMath::RoundToInt(100.0f * FMath::Pow(static_cast<float>(CurrentLevel), 1.4f));
	}
}

float UAegisPlayerProgressionComponent::GetXPPercent() const
{
	if (XPForNextLevel <= 0) return 1.0f;
	return FMath::Clamp(static_cast<float>(CurrentXP) / static_cast<float>(XPForNextLevel), 0.0f, 1.0f);
}

int32 UAegisPlayerProgressionComponent::GetAllocatedPoints(EAegisPlayerAttribute Attribute) const
{
	switch (Attribute)
	{
	case EAegisPlayerAttribute::Vigor: return VigorPoints;
	case EAegisPlayerAttribute::Endurance: return EndurancePoints;
	case EAegisPlayerAttribute::Strength: return StrengthPoints;
	case EAegisPlayerAttribute::Resilience: return ResiliencePoints;
	default: return 0;
	}
}

void UAegisPlayerProgressionComponent::AddXP(int32 Amount)
{
	if (Amount <= 0) return;

	CurrentXP += Amount;

	bool bLeveledUp = false;
	int32 TotalPointsGained = 0;

	const int32 PointsPerLevel = PlayerDataAsset ? PlayerDataAsset->AttributePointsPerLevel : 2;

	while (CurrentXP >= XPForNextLevel)
	{
		CurrentXP -= XPForNextLevel;
		CurrentLevel++;
		AvailableAttributePoints += PointsPerLevel;
		TotalPointsGained += PointsPerLevel;
		bLeveledUp = true;
		UpdateRequiredXP();
	}

	OnXPChanged.Broadcast(CurrentXP, XPForNextLevel);

	if (bLeveledUp)
	{
		OnLevelUp.Broadcast(CurrentLevel, TotalPointsGained);
		OnAttributePointsChanged.Broadcast(AvailableAttributePoints);

		// Trigger Level Up GameplayCue
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
		if (ASC)
		{
			FGameplayCueParameters CueParams;
			CueParams.Location = GetOwner()->GetActorLocation();
			ASC->ExecuteGameplayCue(FAegisGameplayTags::Get().GameplayCue_Player_LevelUp, CueParams);
		}

		RecalculateAttributes();
	}
}

bool UAegisPlayerProgressionComponent::AllocateAttributePoint(EAegisPlayerAttribute Attribute)
{
	if (AvailableAttributePoints <= 0) return false;

	AvailableAttributePoints--;

	switch (Attribute)
	{
	case EAegisPlayerAttribute::Vigor:
		VigorPoints++;
		break;
	case EAegisPlayerAttribute::Endurance:
		EndurancePoints++;
		break;
	case EAegisPlayerAttribute::Strength:
		StrengthPoints++;
		break;
	case EAegisPlayerAttribute::Resilience:
		ResiliencePoints++;
		break;
	}

	OnAttributePointsChanged.Broadcast(AvailableAttributePoints);
	RecalculateAttributes();
	return true;
}

void UAegisPlayerProgressionComponent::RecalculateAttributes()
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority()) return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC) return;

	UAegisAttributeSet* AttrSet = const_cast<UAegisAttributeSet*>(ASC->GetSet<UAegisAttributeSet>());
	if (!AttrSet) return;

	// Calculate target stats based on base + level scaling + allocated points
	float BaseHP = PlayerDataAsset ? PlayerDataAsset->BaseHealth : 500.0f;
	float BaseStam = PlayerDataAsset ? PlayerDataAsset->BaseStamina : 150.0f;
	float BasePoise = PlayerDataAsset ? PlayerDataAsset->BasePoise : 100.0f;
	float BaseAP = PlayerDataAsset ? PlayerDataAsset->BaseAttackPower : 40.0f;
	float BaseDef = PlayerDataAsset ? PlayerDataAsset->BaseDefense : 20.0f;

	float HPPerVigor = PlayerDataAsset ? PlayerDataAsset->HealthPerVigor : 30.0f;
	float StamPerEnd = PlayerDataAsset ? PlayerDataAsset->StaminaPerEndurance : 10.0f;
	float APPerStr = PlayerDataAsset ? PlayerDataAsset->AttackPowerPerStrength : 4.0f;
	float DefPerRes = PlayerDataAsset ? PlayerDataAsset->DefensePerResilience : 2.5f;
	float PoisePerRes = PlayerDataAsset ? PlayerDataAsset->PoisePerResilience : 5.0f;

	// Natural level scaling (3% per level past level 1)
	const float LevelMultiplier = 1.0f + static_cast<float>(CurrentLevel - 1) * 0.03f;

	const float FinalMaxHealth = (BaseHP * LevelMultiplier) + (static_cast<float>(VigorPoints) * HPPerVigor);
	const float FinalMaxStamina = (BaseStam * LevelMultiplier) + (static_cast<float>(EndurancePoints) * StamPerEnd);
	const float FinalMaxPoise = (BasePoise * LevelMultiplier) + (static_cast<float>(ResiliencePoints) * PoisePerRes);
	const float FinalAttackPower = (BaseAP * LevelMultiplier) + (static_cast<float>(StrengthPoints) * APPerStr);
	const float FinalDefense = (BaseDef * LevelMultiplier) + (static_cast<float>(ResiliencePoints) * DefPerRes);

	// Update Max Attributes
	AttrSet->SetMaxHealth(FinalMaxHealth);
	AttrSet->SetMaxStamina(FinalMaxStamina);
	AttrSet->SetMaxPoise(FinalMaxPoise);
	AttrSet->SetAttackPower(FinalAttackPower);
	AttrSet->SetDefense(FinalDefense);

	// Top off current health and stamina to max if currently above or on level up
	if (AttrSet->GetHealth() > FinalMaxHealth || CurrentXP == 0)
	{
		AttrSet->SetHealth(FinalMaxHealth);
	}
	if (AttrSet->GetStamina() > FinalMaxStamina || CurrentXP == 0)
	{
		AttrSet->SetStamina(FinalMaxStamina);
	}
	if (AttrSet->GetPoise() > FinalMaxPoise || CurrentXP == 0)
	{
		AttrSet->SetPoise(FinalMaxPoise);
	}
}

void UAegisPlayerProgressionComponent::HandleEnemyKilled(AActor* DeadEnemy, AActor* Killer, int32 XPReward)
{
	if (Killer == GetOwner())
	{
		AddXP(XPReward);
	}
}
