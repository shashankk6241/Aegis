#include "UI/AegisEnemyHealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UAegisEnemyHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Set default values
	SetHealthPercent(1.0f);
	SetPoisePercent(1.0f);
}

void UAegisEnemyHealthBarWidget::SetEnemyName(const FText& Name)
{
	if (EnemyNameText)
	{
		EnemyNameText->SetText(Name);
	}
}

void UAegisEnemyHealthBarWidget::SetEnemyLevel(int32 Level)
{
	if (EnemyLevelText)
	{
		EnemyLevelText->SetText(FText::Format(NSLOCTEXT("Aegis", "LevelFormat", "Lv. {0}"), FText::AsNumber(Level)));
	}
}

void UAegisEnemyHealthBarWidget::SetHealthPercent(float Percent)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));

		// Color shifts: green → yellow → red based on health percentage
		FLinearColor BarColor;
		if (Percent > 0.5f)
		{
			// Green to Yellow
			const float T = (Percent - 0.5f) * 2.0f;
			BarColor = FLinearColor::LerpUsingHSV(FLinearColor(1.0f, 0.85f, 0.0f), FLinearColor(0.2f, 0.9f, 0.2f), T);
		}
		else
		{
			// Yellow to Red
			const float T = Percent * 2.0f;
			BarColor = FLinearColor::LerpUsingHSV(FLinearColor(0.9f, 0.1f, 0.1f), FLinearColor(1.0f, 0.85f, 0.0f), T);
		}
		HealthBar->SetFillColorAndOpacity(BarColor);
	}
}

void UAegisEnemyHealthBarWidget::SetPoisePercent(float Percent)
{
	if (PoiseBar)
	{
		PoiseBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));

		// Poise bar: cyan when full, fades to dark blue when depleted
		const float ClampedPercent = FMath::Clamp(Percent, 0.0f, 1.0f);
		FLinearColor PoiseColor = FLinearColor::LerpUsingHSV(
			FLinearColor(0.15f, 0.15f, 0.5f),  // Depleted (dark blue)
			FLinearColor(0.3f, 0.8f, 0.95f),    // Full (cyan)
			ClampedPercent
		);
		PoiseBar->SetFillColorAndOpacity(PoiseColor);
	}
}

void UAegisEnemyHealthBarWidget::SetArchetypeStyle(bool bIsBoss)
{
	if (bIsBoss && HealthBar)
	{
		// Boss enemies get a golden health bar background tint
		// This can be extended with different frame styles per archetype
	}
}
