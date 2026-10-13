#include "AI/AegisBTTask_ActivateAbility.h"
#include "AegisEnemyCharacter.h"
#include "AegisEnemyDataAsset.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UAegisBTTask_ActivateAbility::UAegisBTTask_ActivateAbility()
{
	NodeName = "Activate GAS Ability";
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UAegisBTTask_ActivateAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn())
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AIController->GetPawn());
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}

	TSubclassOf<UGameplayAbility> TargetAbility = AbilityClass;
	if (!TargetAbility)
	{
		if (const AAegisEnemyCharacter* EnemyChar = Cast<AAegisEnemyCharacter>(AIController->GetPawn()))
		{
			if (EnemyChar->GetEnemyDataAsset() && EnemyChar->GetEnemyDataAsset()->GrantedAbilities.Num() > 0)
			{
				TargetAbility = EnemyChar->GetEnemyDataAsset()->GrantedAbilities[0];
			}
		}
	}

	if (!TargetAbility)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AegisAI] ActivateAbility task: No AbilityClass set on node and no GrantedAbilities in DataAsset!"));
		return EBTNodeResult::Failed;
	}

	const bool bActivated = ASC->TryActivateAbilityByClass(TargetAbility);
	if (!bActivated)
	{
		return EBTNodeResult::Failed;
	}

	if (bWaitForAbilityEnd)
	{
		ElapsedTime = 0.0f;
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Succeeded;
}

void UAegisBTTask_ActivateAbility::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!bWaitForAbilityEnd) return;

	ElapsedTime += DeltaSeconds;

	// Timeout safety
	if (ElapsedTime >= MaxWaitTime)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AIController->GetPawn());
	if (!ASC)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Check if the ability has finished
	bool bStillActive = false;
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Ability && Spec.Ability->GetClass() == AbilityClass && Spec.IsActive())
		{
			bStillActive = true;
			break;
		}
	}

	if (!bStillActive)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

void UAegisBTTask_ActivateAbility::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	ElapsedTime = 0.0f;
}

FString UAegisBTTask_ActivateAbility::GetStaticDescription() const
{
	return FString::Printf(TEXT("Activate Ability: %s%s"),
		AbilityClass ? *AbilityClass->GetName() : TEXT("None"),
		bWaitForAbilityEnd ? TEXT(" (Wait for End)") : TEXT(""));
}
