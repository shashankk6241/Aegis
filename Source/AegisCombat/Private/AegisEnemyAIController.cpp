#include "AegisEnemyAIController.h"
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
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	AIPerceptionComp->ConfigureSense(*SightConfig);

	// Default hearing configuration
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 800.0f;
	HearingConfig->SetMaxAge(3.0f);
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	AIPerceptionComp->ConfigureSense(*HearingConfig);

	AIPerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());
}

void AAegisEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Enemy character will call StartBehaviorTreeFromDataAsset after its own setup
}

void AAegisEnemyAIController::OnUnPossess()
{
	// Safely stop the behavior tree
	UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BTComp)
	{
		BTComp->StopTree(EBTStopMode::Safe);
	}

	Super::OnUnPossess();
}

void AAegisEnemyAIController::StartBehaviorTreeFromDataAsset(const UAegisEnemyDataAsset* DataAsset)
{
	if (!DataAsset || !DataAsset->BehaviorTree) return;

	// Configure perception from data asset values
	SetupPerception(DataAsset);

	// Initialize blackboard from the BT's blackboard asset
	if (UseBlackboard(DataAsset->BehaviorTree->BlackboardAsset, Blackboard))
	{
		InitializeBlackboard();

		// Set data-driven values
		if (Blackboard)
		{
			Blackboard->SetValueAsFloat(BB_AttackRange, DataAsset->AttackRange);
			Blackboard->SetValueAsVector(BB_HomeLocation, GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector);
		}

		RunBehaviorTree(DataAsset->BehaviorTree);
	}

	// Bind perception delegate
	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AAegisEnemyAIController::OnPerceptionUpdated);
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

void AAegisEnemyAIController::InitializeBlackboard()
{
	if (!Blackboard) return;

	Blackboard->SetValueAsBool(BB_HasAggroTarget, false);
	Blackboard->SetValueAsBool(BB_HasLineOfSight, false);
	Blackboard->SetValueAsBool(BB_CanAttack, true);
	Blackboard->SetValueAsFloat(BB_DistanceToTarget, 0.0f);
}

void AAegisEnemyAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor || !GetPawn() || Actor == GetPawn()) return;

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

		// Update blackboard
		if (Blackboard)
		{
			AActor* TopThreat = AggroComp->GetHighestThreatTarget();
			if (TopThreat)
			{
				Blackboard->SetValueAsObject(BB_TargetActor, TopThreat);
				Blackboard->SetValueAsVector(BB_TargetLocation, TopThreat->GetActorLocation());
				Blackboard->SetValueAsBool(BB_HasAggroTarget, true);
			}
		}
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
