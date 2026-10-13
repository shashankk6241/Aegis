#include "AegisGameplayAbility_EnemyRanged.h"
#include "AegisGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UAegisGameplayAbility_EnemyRanged::UAegisGameplayAbility_EnemyRanged()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	const FAegisGameplayTags& Tags = FAegisGameplayTags::Get();
	SetAssetTags(FGameplayTagContainer(Tags.Ability_Enemy_Ranged));
	ActivationOwnedTags.AddTag(Tags.State_Combat_Attacking);

	ActivationBlockedTags.AddTag(Tags.State_Combat_PoiseBroken);
	ActivationBlockedTags.AddTag(Tags.State_AI_Dead);
}

void UAegisGameplayAbility_EnemyRanged::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CastMontage || !ProjectileClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Clean up any previous task
	if (ActiveMontageTask)
	{
		ActiveMontageTask->EndTask();
		ActiveMontageTask = nullptr;
	}

	// Play cast animation
	ActiveMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		CastMontage,
		1.0f,
		NAME_None,
		true,
		1.0f,
		0.0f
	);

	if (!ActiveMontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActiveMontageTask->OnCompleted.AddDynamic(this, &UAegisGameplayAbility_EnemyRanged::OnCastMontageCompleted);
	ActiveMontageTask->OnBlendOut.AddDynamic(this, &UAegisGameplayAbility_EnemyRanged::OnCastMontageCompleted);
	ActiveMontageTask->OnInterrupted.AddDynamic(this, &UAegisGameplayAbility_EnemyRanged::OnCastMontageCancelled);
	ActiveMontageTask->OnCancelled.AddDynamic(this, &UAegisGameplayAbility_EnemyRanged::OnCastMontageCancelled);
	ActiveMontageTask->ReadyForActivation();
}

void UAegisGameplayAbility_EnemyRanged::OnCastMontageCompleted()
{
	SpawnProjectile();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAegisGameplayAbility_EnemyRanged::OnCastMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UAegisGameplayAbility_EnemyRanged::SpawnProjectile()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar || !ProjectileClass)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(Avatar);
	if (!Character || !Character->GetMesh())
	{
		return;
	}

	// Get spawn location from mesh socket
	FVector SpawnLocation = Character->GetMesh()->GetSocketLocation(ProjectileSpawnSocket);
	FRotator SpawnRotation = Character->GetActorRotation();

	// Aim at the current target if available
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		// Try to find current target from the owner's controller
		AController* Controller = Character->GetController();
		if (Controller)
		{
			AActor* FocusTarget = Controller->GetViewTarget();
			if (FocusTarget && FocusTarget != Character)
			{
				const FVector Direction = (FocusTarget->GetActorLocation() - SpawnLocation).GetSafeNormal();
				SpawnRotation = Direction.Rotation();
			}
		}
	}

	// Spawn the projectile
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.Instigator = Character;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* Projectile = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (Projectile)
	{
		// If the projectile has a ProjectileMovementComponent, set its speed
		UProjectileMovementComponent* ProjMovement = Projectile->FindComponentByClass<UProjectileMovementComponent>();
		if (ProjMovement)
		{
			ProjMovement->InitialSpeed = ProjectileSpeed;
			ProjMovement->MaxSpeed = ProjectileSpeed;
		}
	}
}

void UAegisGameplayAbility_EnemyRanged::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
