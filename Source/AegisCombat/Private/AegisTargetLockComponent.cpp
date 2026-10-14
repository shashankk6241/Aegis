#include "AegisTargetLockComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "MotionWarpingComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AegisGameplayTags.h"

UAegisTargetLockComponent::UAegisTargetLockComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UAegisTargetLockComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAegisTargetLockComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!TargetActor) return;

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;

	// 1. Distance check
	const float Dist = FVector::Dist(Character->GetActorLocation(), TargetActor->GetActorLocation());
	if (Dist > MaxBreakDistance)
	{
		ClearTargetLock();
		return;
	}

	// 2. Dead target check
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC && (TargetASC->HasMatchingGameplayTag(FAegisGameplayTags::Get().State_AI_Dead) ||
					  TargetASC->HasMatchingGameplayTag(FAegisGameplayTags::Get().State_Player_Dead)))
	{
		// Try switching to next target or clear
		AActor* NextTarget = FindBestLockTarget();
		if (NextTarget && NextTarget != TargetActor)
		{
			SetTargetLock(NextTarget);
		}
		else
		{
			ClearTargetLock();
		}
		return;
	}

	// 3. Line of sight check
	if (!HasLineOfSightTo(TargetActor))
	{
		LostSightTimer += DeltaTime;
		if (LostSightTimer >= MaxLostSightDuration)
		{
			ClearTargetLock();
			return;
		}
	}
	else
	{
		LostSightTimer = 0.0f;
	}

	// 4. Update camera and motion warping
	UpdateCameraOrientation(DeltaTime);
	UpdateMotionWarpingTarget();
}

void UAegisTargetLockComponent::ToggleTargetLock()
{
	if (TargetActor)
	{
		ClearTargetLock();
	}
	else
	{
		AActor* BestTarget = FindBestLockTarget();
		if (BestTarget)
		{
			SetTargetLock(BestTarget);
		}
	}
}

void UAegisTargetLockComponent::SetTargetLock(AActor* NewTarget)
{
	if (TargetActor == NewTarget) return;

	AActor* OldTarget = TargetActor;
	TargetActor = NewTarget;
	LostSightTimer = 0.0f;

	if (OldTarget)
	{
		OnTargetUnlocked.Broadcast(OldTarget);
	}

	if (TargetActor)
	{
		OnTargetLocked.Broadcast(TargetActor);
		UpdateMotionWarpingTarget();
	}
}

void UAegisTargetLockComponent::ClearTargetLock()
{
	if (!TargetActor) return;

	AActor* OldTarget = TargetActor;
	TargetActor = nullptr;
	LostSightTimer = 0.0f;

	OnTargetUnlocked.Broadcast(OldTarget);
}

void UAegisTargetLockComponent::SwitchTarget(float Direction)
{
	if (!TargetActor) return;

	AActor* NewTarget = FindBestLockTarget(Direction);
	if (NewTarget && NewTarget != TargetActor)
	{
		SetTargetLock(NewTarget);
	}
}

AActor* UAegisTargetLockComponent::FindBestLockTarget(float DirectionBias) const
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return nullptr;

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (!PC) return nullptr;

	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	const FVector CamFwd = CamRot.Vector();
	const FVector CamRight = UKismetMathLibrary::GetRightVector(CamRot);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Character);

	TArray<FHitResult> Hits;
	const bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetWorld(),
		CamLoc,
		CamLoc + CamFwd * MaxLockDistance,
		MaxLockDistance * 0.5f,
		ObjectTypes,
		false,
		IgnoreActors,
		EDrawDebugTrace::None,
		Hits,
		true
	);

	if (!bHit) return nullptr;

	AActor* BestCandidate = nullptr;
	float BestScore = -TNumericLimits<float>::Max();

	for (const FHitResult& Hit : Hits)
	{
		AActor* Candidate = Hit.GetActor();
		if (!Candidate || Candidate == Character || Candidate == TargetActor) continue;

		// Check candidate is alive
		UAbilitySystemComponent* CandidateASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Candidate);
		if (CandidateASC && CandidateASC->HasMatchingGameplayTag(FAegisGameplayTags::Get().State_AI_Dead))
		{
			continue;
		}

		const FVector ToCandidate = (Candidate->GetActorLocation() - CamLoc).GetSafeNormal();
		const float ForwardDot = FVector::DotProduct(CamFwd, ToCandidate);
		const float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(ForwardDot, -1.0f, 1.0f)));

		if (Angle > LockConeHalfAngle) continue;

		if (!HasLineOfSightTo(Candidate)) continue;

		// Score candidate: closer to forward dot and matching direction bias is favored
		float Score = ForwardDot * 100.0f;
		if (DirectionBias != 0.0f)
		{
			const float RightDot = FVector::DotProduct(CamRight, ToCandidate);
			Score += (RightDot * DirectionBias) * 50.0f;
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			BestCandidate = Candidate;
		}
	}

	return BestCandidate;
}

bool UAegisTargetLockComponent::HasLineOfSightTo(AActor* Candidate) const
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || !Candidate) return false;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);
	QueryParams.AddIgnoredActor(Candidate);

	const FVector Start = Character->GetActorLocation() + FVector(0, 0, 50.0f);
	const FVector End = Candidate->GetActorLocation() + FVector(0, 0, 50.0f);

	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

	return !bBlocked;
}

void UAegisTargetLockComponent::UpdateCameraOrientation(float DeltaTime)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || !TargetActor) return;

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (!PC) return;

	const FVector CamLoc = PC->PlayerCameraManager ? PC->PlayerCameraManager->GetCameraLocation() : Character->GetActorLocation();
	const FVector TargetLoc = TargetActor->GetActorLocation() + TargetOffset;

	const FRotator DesiredRot = (TargetLoc - CamLoc).Rotation();
	const FRotator CurrentRot = PC->GetControlRotation();

	FRotator NewRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, CameraInterpSpeed);
	NewRot.Roll = 0.0f;

	PC->SetControlRotation(NewRot);
}

void UAegisTargetLockComponent::UpdateMotionWarpingTarget()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || !TargetActor) return;

	UMotionWarpingComponent* WarpingComp = Character->FindComponentByClass<UMotionWarpingComponent>();
	if (!WarpingComp) return;

	FVector TargetLoc = TargetActor->GetActorLocation();
	TargetLoc.Z = Character->GetActorLocation().Z;

	FRotator TargetRot = (TargetActor->GetActorLocation() - Character->GetActorLocation()).Rotation();
	TargetRot.Pitch = 0.0f;
	TargetRot.Roll = 0.0f;

	WarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
		FName("CombatTarget"),
		TargetLoc - (TargetRot.Vector() * 90.0f),
		TargetRot
	);
}
