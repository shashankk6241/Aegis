#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AegisTargetLockComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetLocked, AActor*, NewTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetUnlocked, AActor*, OldTarget);

/**
 * UAegisTargetLockComponent
 * Souls-like action RPG target locking system.
 * Automatically aligns player camera, tracks enemy positions, manages target switching,
 * and synchronizes with Motion Warping for combat abilities.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AEGISCOMBAT_API UAegisTargetLockComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAegisTargetLockComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Target Lock Controls
	UFUNCTION(BlueprintCallable, Category = "Aegis|TargetLock")
	void ToggleTargetLock();

	UFUNCTION(BlueprintCallable, Category = "Aegis|TargetLock")
	void SetTargetLock(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "Aegis|TargetLock")
	void ClearTargetLock();

	UFUNCTION(BlueprintCallable, Category = "Aegis|TargetLock")
	void SwitchTarget(float Direction); // > 0 right, < 0 left

	UFUNCTION(BlueprintPure, Category = "Aegis|TargetLock")
	bool HasTarget() const { return TargetActor != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Aegis|TargetLock")
	AActor* GetTargetActor() const { return TargetActor; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTargetLocked OnTargetLocked;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTargetUnlocked OnTargetUnlocked;

protected:
	virtual void BeginPlay() override;

	AActor* FindBestLockTarget(float DirectionBias = 0.0f) const;
	bool HasLineOfSightTo(AActor* Candidate) const;
	void UpdateCameraOrientation(float DeltaTime);
	void UpdateMotionWarpingTarget();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aegis|TargetLock")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|TargetLock")
	float MaxLockDistance = 1800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|TargetLock")
	float MaxBreakDistance = 2200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|TargetLock")
	float LockConeHalfAngle = 65.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|TargetLock")
	float CameraInterpSpeed = 9.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|TargetLock")
	FVector TargetOffset = FVector(0.0f, 0.0f, 40.0f);

	float LostSightTimer = 0.0f;
	const float MaxLostSightDuration = 1.5f;
};
