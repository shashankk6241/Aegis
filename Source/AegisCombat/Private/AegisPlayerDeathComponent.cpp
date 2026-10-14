#include "AegisPlayerDeathComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "AegisGameplayTags.h"
#include "AegisAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

UAegisPlayerDeathComponent::UAegisPlayerDeathComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAegisPlayerDeathComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner)
	{
		InitialSpawnTransform = Owner->GetActorTransform();

		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
		if (ASC)
		{
			UAegisAttributeSet* AttrSet = const_cast<UAegisAttributeSet*>(ASC->GetSet<UAegisAttributeSet>());
			if (AttrSet)
			{
				AttrSet->OnHealthChanged.AddDynamic(this, &UAegisPlayerDeathComponent::OnHealthChanged);
			}
		}
	}
}

void UAegisPlayerDeathComponent::ConfigureDeath(UAnimMontage* InDeathMontage, UAnimMontage* InRespawnMontage, bool bInEnableRespawn, float InRespawnDelay)
{
	DeathMontage = InDeathMontage;
	RespawnMontage = InRespawnMontage;
	bEnableRespawn = bInEnableRespawn;
	RespawnDelay = InRespawnDelay;
}

void UAegisPlayerDeathComponent::OnHealthChanged(AActor* InstigatorActor, AActor* TargetActor, float OldValue, float NewValue)
{
	if (NewValue <= 0.0f && !bIsDead && TargetActor == GetOwner())
	{
		ExecuteDeathFlow(InstigatorActor);
	}
}

void UAegisPlayerDeathComponent::ExecuteDeathFlow(AActor* Killer)
{
	if (bIsDead) return;
	bIsDead = true;

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;

	// 1. Add player dead tag to ASC and cancel abilities
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character);
	if (ASC)
	{
		ASC->AddLooseGameplayTag(FAegisGameplayTags::Get().State_Player_Dead);
		ASC->CancelAllAbilities();
	}

	// 2. Disable movement and input
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->DisableMovement();
	}

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (PC)
	{
		Character->DisableInput(PC);
	}

	// 3. Play death animation
	if (DeathMontage)
	{
		Character->PlayAnimMontage(DeathMontage);
	}

	OnPlayerDied.Broadcast(Character, Killer);

	// 4. Schedule respawn
	if (bEnableRespawn && Character->HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(
			RespawnTimerHandle,
			this,
			&UAegisPlayerDeathComponent::RespawnPlayer,
			RespawnDelay,
			false
		);
	}
}

void UAegisPlayerDeathComponent::RespawnPlayer()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;

	bIsDead = false;

	// 1. Teleport back to spawn point
	Character->SetActorTransform(InitialSpawnTransform, false, nullptr, ETeleportType::ResetPhysics);

	// 2. Restore Attributes to Max
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character);
	if (ASC)
	{
		ASC->RemoveLooseGameplayTag(FAegisGameplayTags::Get().State_Player_Dead);

		UAegisAttributeSet* AttrSet = const_cast<UAegisAttributeSet*>(ASC->GetSet<UAegisAttributeSet>());
		if (AttrSet)
		{
			AttrSet->SetHealth(AttrSet->GetMaxHealth());
			AttrSet->SetStamina(AttrSet->GetMaxStamina());
			AttrSet->SetPoise(AttrSet->GetMaxPoise());
		}
	}

	// 3. Re-enable movement & input
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (PC)
	{
		Character->EnableInput(PC);
	}

	// 4. Play respawn animation if available
	if (RespawnMontage)
	{
		Character->PlayAnimMontage(RespawnMontage);
	}

	OnPlayerRespawned.Broadcast(Character);
}
