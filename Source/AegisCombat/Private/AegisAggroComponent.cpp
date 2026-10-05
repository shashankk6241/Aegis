#include "AegisAggroComponent.h"

UAegisAggroComponent::UAegisAggroComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.25f; // 4 updates per second for performance
}

void UAegisAggroComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Handle taunt expiry
	if (TauntTimer > 0.0f)
	{
		TauntTimer -= DeltaTime;
		if (TauntTimer <= 0.0f)
		{
			ForcedTauntTarget.Reset();
		}
	}

	DecayThreats(DeltaTime);
	CleanupStaleEntries();
	UpdateTopThreat();
}

void UAegisAggroComponent::AddThreat(AActor* Source, float Amount)
{
	if (!Source || Amount <= 0.0f) return;

	if (float* ExistingThreat = ThreatTable.Find(Source))
	{
		*ExistingThreat += Amount;
	}
	else
	{
		ThreatTable.Add(Source, Amount);
	}

	UpdateTopThreat();
}

void UAegisAggroComponent::RemoveThreat(AActor* Source)
{
	if (!Source) return;

	ThreatTable.Remove(Source);
	UpdateTopThreat();
}

void UAegisAggroComponent::ForceTaunt(AActor* TauntTarget, float Duration)
{
	if (!TauntTarget) return;

	ForcedTauntTarget = TauntTarget;
	TauntTimer = Duration;

	// Ensure taunt target is in the threat table with high threat
	if (!ThreatTable.Contains(TauntTarget))
	{
		ThreatTable.Add(TauntTarget, 1000.0f);
	}

	UpdateTopThreat();
}

void UAegisAggroComponent::ClearAllAggro()
{
	AActor* PreviousTarget = CurrentTarget.Get();
	ThreatTable.Empty();
	CurrentTarget.Reset();
	ForcedTauntTarget.Reset();
	TauntTimer = 0.0f;

	if (PreviousTarget)
	{
		OnAggroCleared.Broadcast(PreviousTarget);
	}
}

AActor* UAegisAggroComponent::GetHighestThreatTarget() const
{
	// Taunt overrides normal threat
	if (ForcedTauntTarget.IsValid() && TauntTimer > 0.0f)
	{
		return ForcedTauntTarget.Get();
	}
	return CurrentTarget.Get();
}

bool UAegisAggroComponent::HasAggroTarget() const
{
	return GetHighestThreatTarget() != nullptr;
}

float UAegisAggroComponent::GetThreatForActor(AActor* Source) const
{
	if (!Source) return 0.0f;
	const float* Threat = ThreatTable.Find(Source);
	return Threat ? *Threat : 0.0f;
}

void UAegisAggroComponent::UpdateTopThreat()
{
	AActor* PreviousTarget = CurrentTarget.Get();
	AActor* NewTarget = nullptr;
	float HighestThreat = 0.0f;

	for (const auto& Pair : ThreatTable)
	{
		if (Pair.Key.IsValid() && Pair.Value > HighestThreat)
		{
			HighestThreat = Pair.Value;
			NewTarget = Pair.Key.Get();
		}
	}

	CurrentTarget = NewTarget;

	if (NewTarget != PreviousTarget)
	{
		if (NewTarget)
		{
			OnAggroTargetChanged.Broadcast(NewTarget);
		}
		else if (PreviousTarget)
		{
			OnAggroCleared.Broadcast(PreviousTarget);
		}
	}
}

void UAegisAggroComponent::DecayThreats(float DeltaTime)
{
	for (auto& Pair : ThreatTable)
	{
		Pair.Value -= ThreatDecayRate * DeltaTime;
	}
}

void UAegisAggroComponent::CleanupStaleEntries()
{
	const AActor* Owner = GetOwner();

	for (auto It = ThreatTable.CreateIterator(); It; ++It)
	{
		// Remove invalid or below-threshold entries
		if (!It->Key.IsValid() || It->Value < MinThreatThreshold)
		{
			It.RemoveCurrent();
			continue;
		}

		// Remove if out of max aggro distance
		if (Owner && It->Key.IsValid())
		{
			const float Dist = FVector::Dist(Owner->GetActorLocation(), It->Key->GetActorLocation());
			if (Dist > MaxAggroDistance)
			{
				It.RemoveCurrent();
			}
		}
	}
}
