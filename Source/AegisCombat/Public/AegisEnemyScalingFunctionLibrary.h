#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AegisEnemyScalingFunctionLibrary.generated.h"

class UAegisEnemyDataAsset;
class UCurveFloat;

/**
 * UAegisEnemyScalingFunctionLibrary
 * Static utility functions for computing level-scaled enemy attributes and rewards.
 */
UCLASS()
class AEGISCOMBAT_API UAegisEnemyScalingFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Scale a single attribute value by level using linear growth: Base × (1 + (Level-1) × GrowthRate) */
	UFUNCTION(BlueprintPure, Category = "Aegis|Scaling", meta = (DisplayName = "Scale Attribute (Linear)"))
	static float ScaleAttributeLinear(float BaseValue, int32 Level, float GrowthRate);

	/** Scale a single attribute value by level using a UCurveFloat (X = Level, Y = Multiplier) */
	UFUNCTION(BlueprintPure, Category = "Aegis|Scaling", meta = (DisplayName = "Scale Attribute (Curve)"))
	static float ScaleAttributeWithCurve(float BaseValue, int32 Level, const UCurveFloat* GrowthCurve);

	/** Get all scaled stats from a data asset at a given level */
	UFUNCTION(BlueprintPure, Category = "Aegis|Scaling")
	static void GetScaledStatsFromDataAsset(
		const UAegisEnemyDataAsset* DataAsset,
		int32 Level,
		float& OutHealth,
		float& OutStamina,
		float& OutPoise,
		float& OutAttackPower,
		float& OutDefense);

	/** Calculate XP reward with level-difference scaling (bonus for overleveled enemies, penalty for underleveled) */
	UFUNCTION(BlueprintPure, Category = "Aegis|Scaling")
	static int32 CalculateXPReward(int32 BaseXP, int32 EnemyLevel, int32 PlayerLevel);
};
