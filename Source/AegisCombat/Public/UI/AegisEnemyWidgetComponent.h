#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "AegisEnemyWidgetComponent.generated.h"

class UAegisEnemyHealthBarWidget;

/**
 * UAegisEnemyWidgetComponent
 * Billboard widget component that hosts the floating health bar.
 * Automatically faces the camera and fades based on distance.
 */
UCLASS(ClassGroup = (Aegis), meta = (BlueprintSpawnableComponent))
class AEGISCOMBAT_API UAegisEnemyWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UAegisEnemyWidgetComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Get the health bar widget instance (cast) */
	UFUNCTION(BlueprintPure, Category = "Aegis|UI")
	UAegisEnemyHealthBarWidget* GetHealthBarWidget() const;

	/** Show the health bar */
	UFUNCTION(BlueprintCallable, Category = "Aegis|UI")
	void ShowHealthBar();

	/** Hide the health bar */
	UFUNCTION(BlueprintCallable, Category = "Aegis|UI")
	void HideHealthBar();

protected:
	/** Maximum distance at which the health bar is visible (cm) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display", meta = (ClampMin = "0.0"))
	float MaxVisibleDistance = 2500.0f;

	/** Distance at which the health bar starts fading (cm) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display", meta = (ClampMin = "0.0"))
	float FadeStartDistance = 1500.0f;

	/** If true, only show when the enemy has taken damage */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	bool bOnlyShowWhenDamaged = true;

	/** Time after last damage before auto-hiding */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display", meta = (ClampMin = "0.0"))
	float HideAfterSeconds = 5.0f;

private:
	float TimeSinceLastDamage = 0.0f;
	bool bManuallyVisible = false;

	void UpdateDistanceFade();
};
