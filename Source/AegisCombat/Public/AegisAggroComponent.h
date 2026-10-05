#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AegisAggroComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAggroTargetChanged, AActor*, NewTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAggroCleared, AActor*, PreviousTarget);

/**
 * UAegisAggroComponent
 * Threat table component tracking per-actor aggro with decay, taunt support,
 * and distance-based cleanup. Drives AI target selection.
 */
UCLASS(ClassGroup = (Aegis), meta = (BlueprintSpawnableComponent))
class AEGISCOMBAT_API UAegisAggroComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAegisAggroComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Add threat from a source actor */
	UFUNCTION(BlueprintCallable, Category = "Aegis|Aggro")
	void AddThreat(AActor* Source, float Amount);

	/** Remove all threat from a source actor */
	UFUNCTION(BlueprintCallable, Category = "Aegis|Aggro")
	void RemoveThreat(AActor* Source);

	/** Force aggro on a specific target (taunt mechanic) */
	UFUNCTION(BlueprintCallable, Category = "Aegis|Aggro")
	void ForceTaunt(AActor* TauntTarget, float Duration);

	/** Clear all aggro entries */
	UFUNCTION(BlueprintCallable, Category = "Aegis|Aggro")
	void ClearAllAggro();

	/** Get the current highest-threat target (respects taunt) */
	UFUNCTION(BlueprintPure, Category = "Aegis|Aggro")
	AActor* GetHighestThreatTarget() const;

	/** Check if any targets are on the threat table */
	UFUNCTION(BlueprintPure, Category = "Aegis|Aggro")
	bool HasAggroTarget() const;

	/** Get threat amount for a specific actor */
	UFUNCTION(BlueprintPure, Category = "Aegis|Aggro")
	float GetThreatForActor(AActor* Source) const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAggroTargetChanged OnAggroTargetChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAggroCleared OnAggroCleared;

protected:
	/** Rate at which threat decays per second */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aggro", meta = (ClampMin = "0.0"))
	float ThreatDecayRate = 5.0f;

	/** Minimum threat to maintain aggro (below this, entry is removed) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aggro", meta = (ClampMin = "0.0"))
	float MinThreatThreshold = 1.0f;

	/** Maximum distance before threat is automatically cleared */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aggro", meta = (ClampMin = "0.0"))
	float MaxAggroDistance = 2500.0f;

private:
	/** Threat table mapping actors to their accumulated threat */
	TMap<TWeakObjectPtr<AActor>, float> ThreatTable;

	/** Cached current top-threat target */
	TWeakObjectPtr<AActor> CurrentTarget;

	/** Taunt state */
	TWeakObjectPtr<AActor> ForcedTauntTarget;
	float TauntTimer = 0.0f;

	void UpdateTopThreat();
	void DecayThreats(float DeltaTime);
	void CleanupStaleEntries();
};
