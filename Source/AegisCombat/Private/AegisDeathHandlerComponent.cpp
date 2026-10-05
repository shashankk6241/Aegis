#include "AegisDeathHandlerComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "AegisGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

UAegisDeathHandlerComponent::UAegisDeathHandlerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAegisDeathHandlerComponent::ConfigureFromData(UAnimMontage* InDeathMontage, int32 InXP, const TArray<FAegisLootDropInfo>& InLootTable)
{
	DeathMontage = InDeathMontage;
	XPReward = InXP;
	LootTable = InLootTable;
}

void UAegisDeathHandlerComponent::ExecuteDeathFlow(AActor* Killer)
{
	if (bDeathInProgress) return;
	bDeathInProgress = true;
	KillerActor = Killer;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	// 1. Add death tag to ASC and cancel all abilities
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (ASC)
	{
		ASC->AddLooseGameplayTag(FAegisGameplayTags::Get().State_AI_Dead);
		ASC->CancelAllAbilities();
	}

	// 2. Disable collision and movement
	ACharacter* Character = Cast<ACharacter>(Owner);
	if (Character)
	{
		if (Character->GetCapsuleComponent())
		{
			Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (Character->GetCharacterMovement())
		{
			Character->GetCharacterMovement()->DisableMovement();
		}
	}

	// 3. Play death montage
	if (DeathMontage && Character)
	{
		const float MontageLength = Character->PlayAnimMontage(DeathMontage);
		if (MontageLength > 0.0f)
		{
			FTimerHandle MontageTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				MontageTimerHandle,
				this,
				&UAegisDeathHandlerComponent::OnDeathMontageEnded,
				MontageLength,
				false
			);
			return; // Wait for montage to finish
		}
	}

	// If no montage, proceed directly
	OnDeathMontageEnded();
}

void UAegisDeathHandlerComponent::OnDeathMontageEnded()
{
	// Broadcast death event with XP reward
	OnEnemyDied.Broadcast(GetOwner(), KillerActor.Get(), XPReward);

	// Evaluate loot drops
	EvaluateLootDrops();

	// Enable ragdoll after delay
	if (bEnableRagdoll)
	{
		GetWorld()->GetTimerManager().SetTimer(
			RagdollTimerHandle,
			this,
			&UAegisDeathHandlerComponent::EnableRagdoll,
			RagdollDelay,
			false
		);
	}

	// Schedule actor cleanup
	GetWorld()->GetTimerManager().SetTimer(
		DestroyTimerHandle,
		this,
		&UAegisDeathHandlerComponent::CleanupAndDestroy,
		DestroyDelay,
		false
	);
}

void UAegisDeathHandlerComponent::EnableRagdoll()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || !Character->GetMesh()) return;

	Character->GetMesh()->SetSimulatePhysics(true);
	Character->GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
}

void UAegisDeathHandlerComponent::EvaluateLootDrops()
{
	TArray<FAegisLootDropInfo> DroppedItems;

	for (const FAegisLootDropInfo& Entry : LootTable)
	{
		if (!Entry.ItemClass) continue;

		const float Roll = FMath::FRand();
		if (Roll <= Entry.DropChance)
		{
			const int32 Quantity = FMath::RandRange(Entry.MinQuantity, Entry.MaxQuantity);
			for (int32 i = 0; i < Quantity; ++i)
			{
				// Spawn loot item near the dead enemy with slight scatter
				FVector SpawnLocation = GetOwner()->GetActorLocation();
				SpawnLocation.Z += 50.0f;
				SpawnLocation += FMath::VRand() * FMath::RandRange(30.0f, 80.0f);

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				GetWorld()->SpawnActor<AActor>(Entry.ItemClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
			}
			DroppedItems.Add(Entry);
		}
	}

	if (DroppedItems.Num() > 0)
	{
		OnLootDropped.Broadcast(GetOwner(), DroppedItems);
	}
}

void UAegisDeathHandlerComponent::CleanupAndDestroy()
{
	if (AActor* Owner = GetOwner())
	{
		Owner->Destroy();
	}
}
