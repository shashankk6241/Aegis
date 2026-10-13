#include "UI/AegisEnemyWidgetComponent.h"
#include "UI/AegisEnemyHealthBarWidget.h"
#include "Kismet/GameplayStatics.h"

UAegisEnemyWidgetComponent::UAegisEnemyWidgetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // 10 updates per second

	// Billboard facing
	SetWidgetSpace(EWidgetSpace::Screen);
	SetDrawSize(FVector2D(200.0f, 50.0f));
	SetDrawAtDesiredSize(false);

	// Start hidden if configured to show only on damage
	SetVisibility(false);
	bManuallyVisible = false;
}

void UAegisEnemyWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bOnlyShowWhenDamaged && bManuallyVisible)
	{
		TimeSinceLastDamage += DeltaTime;
		if (TimeSinceLastDamage >= HideAfterSeconds)
		{
			HideHealthBar();
		}
	}

	UpdateDistanceFade();
}

UAegisEnemyHealthBarWidget* UAegisEnemyWidgetComponent::GetHealthBarWidget() const
{
	return Cast<UAegisEnemyHealthBarWidget>(GetWidget());
}

void UAegisEnemyWidgetComponent::ShowHealthBar()
{
	bManuallyVisible = true;
	TimeSinceLastDamage = 0.0f;
	SetVisibility(true);
}

void UAegisEnemyWidgetComponent::HideHealthBar()
{
	bManuallyVisible = false;
	SetVisibility(false);
}

void UAegisEnemyWidgetComponent::UpdateDistanceFade()
{
	if (!IsVisible() || !GetOwner())
	{
		return;
	}

	APlayerCameraManager* CamManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (!CamManager)
	{
		return;
	}

	const float Distance = FVector::Dist(GetOwner()->GetActorLocation(), CamManager->GetCameraLocation());

	if (Distance > MaxVisibleDistance)
	{
		SetVisibility(false);
		return;
	}

	// Calculate opacity fade
	float Opacity = 1.0f;
	if (Distance > FadeStartDistance)
	{
		const float FadeRange = MaxVisibleDistance - FadeStartDistance;
		Opacity = 1.0f - FMath::Clamp((Distance - FadeStartDistance) / FadeRange, 0.0f, 1.0f);
	}

	// Apply opacity to the widget
	UUserWidget* EnemyWidgetInstance = GetUserWidgetObject();
	if (EnemyWidgetInstance)
	{
		EnemyWidgetInstance->SetRenderOpacity(Opacity);
	}
}
