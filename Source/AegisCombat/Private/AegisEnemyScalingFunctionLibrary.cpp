#include "AegisEnemyScalingFunctionLibrary.h"
#include "AegisEnemyDataAsset.h"
#include "Curves/CurveFloat.h"

float UAegisEnemyScalingFunctionLibrary::ScaleAttributeLinear(float BaseValue, int32 Level, float GrowthRate)
{
	const int32 ClampedLevel = FMath::Max(Level, 1);
	return BaseValue * (1.0f + (ClampedLevel - 1) * GrowthRate);
}

float UAegisEnemyScalingFunctionLibrary::ScaleAttributeWithCurve(float BaseValue, int32 Level, const UCurveFloat* GrowthCurve)
{
	if (!GrowthCurve)
	{
		return BaseValue;
	}
	const float Multiplier = GrowthCurve->GetFloatValue(static_cast<float>(Level));
	return BaseValue * Multiplier;
}

void UAegisEnemyScalingFunctionLibrary::GetScaledStatsFromDataAsset(
	const UAegisEnemyDataAsset* DataAsset,
	int32 Level,
	float& OutHealth,
	float& OutStamina,
	float& OutPoise,
	float& OutAttackPower,
	float& OutDefense)
{
	if (!DataAsset)
	{
		OutHealth = 100.0f;
		OutStamina = 100.0f;
		OutPoise = 50.0f;
		OutAttackPower = 25.0f;
		OutDefense = 10.0f;
		return;
	}

	OutHealth = DataAsset->GetScaledStat(DataAsset->BaseHealth, DataAsset->HealthGrowthRate, Level);
	OutStamina = DataAsset->GetScaledStat(DataAsset->BaseStamina, DataAsset->StaminaGrowthRate, Level);
	OutPoise = DataAsset->GetScaledStat(DataAsset->BasePoise, DataAsset->PoiseGrowthRate, Level);
	OutAttackPower = DataAsset->GetScaledStat(DataAsset->BaseAttackPower, DataAsset->AttackPowerGrowthRate, Level);
	OutDefense = DataAsset->GetScaledStat(DataAsset->BaseDefense, DataAsset->DefenseGrowthRate, Level);
}

int32 UAegisEnemyScalingFunctionLibrary::CalculateXPReward(int32 BaseXP, int32 EnemyLevel, int32 PlayerLevel)
{
	// Scale XP based on level difference:
	// +10% per level the enemy is above player, -10% per level below
	// Clamped between 10% and 300% of base
	const int32 LevelDiff = EnemyLevel - PlayerLevel;
	const float Multiplier = FMath::Clamp(1.0f + LevelDiff * 0.1f, 0.1f, 3.0f);
	return FMath::Max(1, FMath::RoundToInt(BaseXP * Multiplier));
}
