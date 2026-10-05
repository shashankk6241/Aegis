#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "AegisEnemyAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAegisEnemyDataAsset;

/**
 * AAegisEnemyAIController
 * Production AI controller with integrated perception (sight + hearing),
 * Behavior Tree execution, and aggro-driven blackboard management.
 */
UCLASS()
class AEGISCOMBAT_API AAegisEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AAegisEnemyAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	/** Start the behavior tree from the enemy's data asset */
	UFUNCTION(BlueprintCallable, Category = "Aegis|AI")
	void StartBehaviorTreeFromDataAsset(const UAegisEnemyDataAsset* DataAsset);

	/** Update blackboard with current combat info */
	UFUNCTION(BlueprintCallable, Category = "Aegis|AI")
	void UpdateBlackboardCombatData();

	// ==========================================
	// Blackboard Key Names (static constants)
	// ==========================================
	static const FName BB_TargetActor;
	static const FName BB_TargetLocation;
	static const FName BB_LastKnownLocation;
	static const FName BB_PatrolLocation;
	static const FName BB_DistanceToTarget;
	static const FName BB_HasAggroTarget;
	static const FName BB_HasLineOfSight;
	static const FName BB_EnemyState;
	static const FName BB_AttackRange;
	static const FName BB_CanAttack;
	static const FName BB_HomeLocation;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	/** Callback when AI perception detects or loses actors */
	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void SetupPerception(const UAegisEnemyDataAsset* DataAsset);
	void InitializeBlackboard();
};
