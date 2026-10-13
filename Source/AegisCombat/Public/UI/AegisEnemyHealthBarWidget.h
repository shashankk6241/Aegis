#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AegisEnemyHealthBarWidget.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * UAegisEnemyHealthBarWidget
 * Floating enemy health bar widget displaying name, level, health bar, and poise bar.
 * Designed to be hosted by UAegisEnemyWidgetComponent on enemy actors.
 */
UCLASS()
class AEGISCOMBAT_API UAegisEnemyHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Update the displayed enemy name */
	UFUNCTION(BlueprintCallable, Category = "Aegis|UI")
	void SetEnemyName(const FText& Name);

	/** Update the displayed enemy level */
	UFUNCTION(BlueprintCallable, Category = "Aegis|UI")
	void SetEnemyLevel(int32 Level);

	/** Update health bar fill (0.0 to 1.0) */
	UFUNCTION(BlueprintCallable, Category = "Aegis|UI")
	void SetHealthPercent(float Percent);

	/** Update poise bar fill (0.0 to 1.0) */
	UFUNCTION(BlueprintCallable, Category = "Aegis|UI")
	void SetPoisePercent(float Percent);

	/** Set the bar colors based on archetype */
	UFUNCTION(BlueprintCallable, Category = "Aegis|UI")
	void SetArchetypeStyle(bool bIsBoss);

protected:
	virtual void NativeConstruct() override;

	/** Bound via BindWidget in Blueprint or programmatically */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> PoiseBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EnemyNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EnemyLevelText;
};
