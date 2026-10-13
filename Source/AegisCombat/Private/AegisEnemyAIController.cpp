#include "AegisEnemyAIController.h"
#include "AegisEnemyCharacter.h"
#include "AegisEnemyDataAsset.h"
#include "AegisAggroComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"

// Static Blackboard key name definitions
const FName AAegisEnemyAIController::BB_TargetActor       = FName("TargetActor");
const FName AAegisEnemyAIController::BB_TargetLocation    = FName("TargetLocation");
const FName AAegisEnemyAIController::BB_LastKnownLocation = FName("LastKnownLocation");
const FName AAegisEnemyAIController::BB_PatrolLocation    = FName("PatrolLocation");
const FName AAegisEnemyAIController::BB_DistanceToTarget  = FName("DistanceToTarget");
const FName AAegisEnemyAIController::BB_HasAggroTarget    = FName("HasAggroTarget");
const FName AAegisEnemyAIController::BB_HasLineOfSight    = FName("HasLineOfSight");
const FName AAegisEnemyAIController::BB_EnemyState        = FName("EnemyState");
const FName AAegisEnemyAIController::BB_AttackRange       = FName("AttackRange");
const FName AAegisEnemyAIController::BB_CanAttack         = FName("CanAttack");
const FName AAegisEnemyAIController::BB_HomeLocation      = FName("HomeLocation");

AAegisEnemyAIController::AAegisEnemyAIController()
{
	// Create perception component
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	SetPerceptionComponent(*AIPerceptionComp);

	// Default sight configuration
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1200.0f;
	SightConfig->LoseSightRadius = 1600.0f;
	SightConfig->PeripheralVisionAngleDegrees = 75.0f;
	SightConfig->SetMaxAge(5.0f);
	SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	AIPerceptionComp->ConfigureSense(*SightConfig);

	// Default hearing configuration
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 800.0f;
	HearingConfig->SetMaxAge(3.0f);
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	AIPerceptionComp->ConfigureSense(*HearingConfig);

	AIPerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());
}

void AAegisEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (UAegisAggroComponent* AggroComp = InPawn->FindComponentByClass<UAegisAggroComponent>())
	{
		AggroComp->OnAggroTargetChanged.AddUniqueDynamic(this, &AAegisEnemyAIController::HandleAggroTargetChanged);
		AggroComp->OnAggroCleared.AddUniqueDynamic(this, &AAegisEnemyAIController::HandleAggroCleared);
	}

	if (const AAegisEnemyCharacter* EnemyChar = Cast<AAegisEnemyCharacter>(InPawn))
	{
		if (EnemyChar->GetEnemyDataAsset())
		{
			StartBehaviorTreeFromDataAsset(EnemyChar->GetEnemyDataAsset());
		}
	}
}

void AAegisEnemyAIController::OnUnPossess()
{
	if (GetPawn())
	{
		if (UAegisAggroComponent* AggroComp = GetPawn()->FindComponentByClass<UAegisAggroComponent>())
		{
			AggroComp->OnAggroTargetChanged.RemoveDynamic(this, &AAegisEnemyAIController::HandleAggroTargetChanged);
			AggroComp->OnAggroCleared.RemoveDynamic(this, &AAegisEnemyAIController::HandleAggroCleared);
		}
	}

	// Safely stop the behavior tree
	UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BTComp)
	{
		BTComp->StopTree(EBTStopMode::Safe);
	}

	Super::OnUnPossess();
}


void AAegisEnemyAIController::StartBehaviorTreeFromDataAsset(
    const UAegisEnemyDataAsset* DataAsset)
{
    if (!DataAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AegisAI] %s: Cannot start BT - EnemyDataAsset is NULL! Assign DA_EnemyGrunt on enemy."), *GetName());
        return;
    }

    if (!DataAsset->BehaviorTree)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AegisAI] %s: Cannot start BT - BehaviorTree is NOT assigned in Data Asset '%s'!"), *GetName(), *DataAsset->GetName());
        return;
    }

    if (!DataAsset->BehaviorTree->BlackboardAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AegisAI] %s: Cannot start BT - BlackboardAsset is NOT assigned in BehaviorTree '%s'!"), *GetName(), *DataAsset->BehaviorTree->GetName());
        return;
    }

    // Configure perception from data asset values.
    SetupPerception(DataAsset);

    // UE 5.5 requires UBlackboardComponent*& here,
    // so use a raw pointer rather than the TObjectPtr member.
    UBlackboardComponent* BlackboardComp = nullptr;

    if (UseBlackboard(
        DataAsset->BehaviorTree->BlackboardAsset.Get(),
        BlackboardComp))
    {
        // Store the initialized component.
        Blackboard = BlackboardComp;

        // Initialize Aegis-specific Blackboard values.
        InitializeEnemyBlackboard();

        Blackboard->SetValueAsFloat(
            BB_AttackRange,
            DataAsset->AttackRange);

        Blackboard->SetValueAsVector(
            BB_HomeLocation,
            GetPawn()
                ? GetPawn()->GetActorLocation()
                : FVector::ZeroVector);

        RunBehaviorTree(DataAsset->BehaviorTree);
        UE_LOG(LogTemp, Log, TEXT("[AegisAI] %s: Successfully started Behavior Tree '%s'"), *GetName(), *DataAsset->BehaviorTree->GetName());
    }

    // Bind only once, even if initialization is called repeatedly.
    if (AIPerceptionComp)
    {
        AIPerceptionComp->OnTargetPerceptionUpdated.AddUniqueDynamic(
            this,
            &AAegisEnemyAIController::OnPerceptionUpdated);
    }
}


void AAegisEnemyAIController::SetupPerception(const UAegisEnemyDataAsset* DataAsset)
{
	if (!DataAsset || !AIPerceptionComp) return;

	if (SightConfig)
	{
		SightConfig->SightRadius = DataAsset->DetectionRadius;
		SightConfig->LoseSightRadius = DataAsset->LoseAggroRange;
		AIPerceptionComp->ConfigureSense(*SightConfig);
	}

	if (HearingConfig)
	{
		HearingConfig->HearingRange = DataAsset->HearingRange;
		AIPerceptionComp->ConfigureSense(*HearingConfig);
	}

	AIPerceptionComp->RequestStimuliListenerUpdate();
}


void AAegisEnemyAIController::InitializeEnemyBlackboard()
{
    if (!Blackboard)
    {
        return;
    }

    Blackboard->SetValueAsBool(BB_HasAggroTarget, false);
    Blackboard->SetValueAsBool(BB_HasLineOfSight, false);
    Blackboard->SetValueAsBool(BB_CanAttack, true);
    Blackboard->SetValueAsFloat(BB_DistanceToTarget, 0.0f);
}


void AAegisEnemyAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor || !GetPawn() || Actor == GetPawn()) return;

	// Do not aggro on fellow enemies
	if (Actor->IsA<AAegisEnemyCharacter>()) return;

	UAegisAggroComponent* AggroComp = GetPawn()->FindComponentByClass<UAegisAggroComponent>();
	if (!AggroComp) return;

	if (Stimulus.WasSuccessfullySensed())
	{
		// Actor detected — add initial threat (hearing generates less)
		float ThreatAmount = 50.0f;
		if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
		{
			ThreatAmount = 25.0f;
		}
		AggroComp->AddThreat(Actor, ThreatAmount);
	}
	else
	{
		// Actor lost — store last known location
		if (Blackboard)
		{
			Blackboard->SetValueAsVector(BB_LastKnownLocation, Stimulus.StimulusLocation);
			Blackboard->SetValueAsBool(BB_HasLineOfSight, false);
		}
	}
}

void AAegisEnemyAIController::HandleAggroTargetChanged(AActor* NewTarget)
{
	if (!Blackboard) return;

	if (NewTarget)
	{
		Blackboard->SetValueAsObject(BB_TargetActor, NewTarget);
		Blackboard->SetValueAsVector(BB_TargetLocation, NewTarget->GetActorLocation());
		Blackboard->SetValueAsBool(BB_HasAggroTarget, true);

		if (GetPawn())
		{
			const float Dist = FVector::Dist(GetPawn()->GetActorLocation(), NewTarget->GetActorLocation());
			Blackboard->SetValueAsFloat(BB_DistanceToTarget, Dist);
		}
		UE_LOG(LogTemp, Log, TEXT("[AegisAI] %s: Acquired Aggro Target %s! Entering combat."), *GetName(), *NewTarget->GetName());
	}
	else
	{
		Blackboard->ClearValue(BB_TargetActor);
		Blackboard->SetValueAsBool(BB_HasAggroTarget, false);
		Blackboard->SetValueAsFloat(BB_DistanceToTarget, 0.0f);
		UE_LOG(LogTemp, Log, TEXT("[AegisAI] %s: Aggro cleared. Returning to patrol."), *GetName());
	}
}

void AAegisEnemyAIController::HandleAggroCleared(AActor* PreviousTarget)
{
	if (!Blackboard) return;

	Blackboard->ClearValue(BB_TargetActor);
	Blackboard->SetValueAsBool(BB_HasAggroTarget, false);
	Blackboard->SetValueAsFloat(BB_DistanceToTarget, 0.0f);
}

void AAegisEnemyAIController::UpdateBlackboardCombatData()
{
	if (!Blackboard || !GetPawn()) return;

	UAegisAggroComponent* AggroComp = GetPawn()->FindComponentByClass<UAegisAggroComponent>();
	if (!AggroComp) return;

	AActor* Target = AggroComp->GetHighestThreatTarget();
	const bool bHasTarget = Target != nullptr;

	Blackboard->SetValueAsBool(BB_HasAggroTarget, bHasTarget);

	if (bHasTarget)
	{
		Blackboard->SetValueAsObject(BB_TargetActor, Target);
		Blackboard->SetValueAsVector(BB_TargetLocation, Target->GetActorLocation());

		const float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
		Blackboard->SetValueAsFloat(BB_DistanceToTarget, Distance);

		// Line of sight check
		FVector EyeLocation;
		FRotator EyeRotation;
		GetPawn()->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(GetPawn());
		const bool bHasLOS = !GetWorld()->LineTraceSingleByChannel(
			HitResult, EyeLocation, Target->GetActorLocation(),
			ECC_Visibility, QueryParams
		);
		Blackboard->SetValueAsBool(BB_HasLineOfSight, bHasLOS);
	}
	else
	{
		Blackboard->SetValueAsFloat(BB_DistanceToTarget, 0.0f);
		Blackboard->SetValueAsBool(BB_HasLineOfSight, false);
	}
}
