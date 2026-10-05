#include "AegisEnemyDataAsset.h"
#include "Curves/CurveFloat.h"

float UAegisEnemyDataAsset::GetScaledStat(float BaseValue, float GrowthRate, int32 Level) const
{
	if (StatScalingCurve)
	{
		// Use curve-based scaling if a custom curve is provided
		const float CurveMultiplier = StatScalingCurve->GetFloatValue(static_cast<float>(Level));
		return BaseValue * CurveMultiplier;
	}

	// Linear scaling: Base × (1 + (Level - 1) × GrowthRate)
	const int32 ClampedLevel = FMath::Max(Level, 1);
	return BaseValue * (1.0f + (ClampedLevel - 1) * GrowthRate);
}

FPrimaryAssetId UAegisEnemyDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("AegisEnemyData"), GetFName());
}
