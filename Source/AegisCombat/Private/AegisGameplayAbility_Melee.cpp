#include "AegisGameplayAbility_Melee.h"
#include "AegisAbilitySystemComponent.h"
#include "AegisGameplayTags.h"
#include "GameFramework/Character.h"
#include "MotionWarpingComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UAegisGameplayAbility_Melee::UAegisGameplayAbility_Melee()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    const FAegisGameplayTags& Tags = FAegisGameplayTags::Get();
    SetAssetTags(FGameplayTagContainer(Tags.InputTag_Attack_Light));
    ActivationOwnedTags.AddTag(Tags.State_Combat_Attacking);
}

void UAegisGameplayAbility_Melee::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (ComboChain.IsEmpty())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    CurrentComboIndex = 0;
    PlayCurrentComboAttack();
}

void UAegisGameplayAbility_Melee::PlayCurrentComboAttack()
{
    if (!ComboChain.IsValidIndex(CurrentComboIndex))
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    AlreadyHitActors.Empty();
    UpdateMotionWarpingTarget();

    const FAegisAttackMontageInfo& CurrentAttack = ComboChain[CurrentComboIndex];
    if (!CurrentAttack.Montage)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }

    if (ActiveMontageTask)
    {
        ActiveMontageTask->EndTask();
        ActiveMontageTask = nullptr;
    }
    if (ActiveEventTask)
    {
        ActiveEventTask->EndTask();
        ActiveEventTask = nullptr;
    }

    ActiveMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this,
        NAME_None,
        CurrentAttack.Montage,
        1.0f,
        NAME_None,
        true,
        1.0f,
        0.0f
    );

    if (!ActiveMontageTask)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }
    
    ActiveMontageTask->OnCompleted.AddDynamic(this, &UAegisGameplayAbility_Melee::OnMontageCompleted);
    ActiveMontageTask->OnBlendOut.AddDynamic(this, &UAegisGameplayAbility_Melee::OnMontageCompleted);
    ActiveMontageTask->OnInterrupted.AddDynamic(this, &UAegisGameplayAbility_Melee::OnMontageCancelled);
    ActiveMontageTask->OnCancelled.AddDynamic(this, &UAegisGameplayAbility_Melee::OnMontageCancelled);
    ActiveMontageTask->ReadyForActivation();

    // Listen for the Combo Window Anim Notify to end, so we can transition snappily instead of waiting for the full blend out!
    ActiveEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this, FAegisGameplayTags::Get().State_Combat_CanCombo);
    if (ActiveEventTask)
    {
        ActiveEventTask->EventReceived.AddDynamic(this, &UAegisGameplayAbility_Melee::OnComboWindowClosed);
        ActiveEventTask->ReadyForActivation();
    }
}

void UAegisGameplayAbility_Melee::UpdateMotionWarpingTarget()
{
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Character) return;

    UMotionWarpingComponent* WarpingComp = Character->FindComponentByClass<UMotionWarpingComponent>();
    if (!WarpingComp)
    {
        WarpingComp = NewObject<UMotionWarpingComponent>(Character, TEXT("RuntimeMotionWarpingComp"));
        if (WarpingComp)
        {
            WarpingComp->RegisterComponent();
        }
    }
    if (!WarpingComp) return;

    // Search for nearest enemy target within 5-meter cone
    FVector Start = Character->GetActorLocation();
    FVector Forward = Character->GetActorForwardVector();
    float SearchRadius = ComboChain.IsValidIndex(CurrentComboIndex) ? FMath::Max(ComboChain[CurrentComboIndex].WarpDistance, 500.0f) : 500.0f;

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(Character);

    TArray<FHitResult> Hits;
    bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
        GetWorld(),
        Start,
        Start + Forward * SearchRadius,
        120.0f,
        ObjectTypes,
        false,
        IgnoreActors,
        EDrawDebugTrace::None,
        Hits,
        true
    );

    if (bHit && Hits.Num() > 0)
    {
        AActor* BestTarget = nullptr;
        float ClosestDistSq = TNumericLimits<float>::Max();

        for (const FHitResult& Hit : Hits)
        {
            AActor* HitActor = Hit.GetActor();
            if (HitActor && HitActor != Character)
            {
                float DistSq = FVector::DistSquared(Start, HitActor->GetActorLocation());
                if (DistSq < ClosestDistSq)
                {
                    ClosestDistSq = DistSq;
                    BestTarget = HitActor;
                }
            }
        }

        if (BestTarget)
        {
            // Position character cleanly in front of target
            FVector TargetLoc = BestTarget->GetActorLocation() - (Forward * 90.0f);
            TargetLoc.Z = Character->GetActorLocation().Z;

            FRotator TargetRot = (BestTarget->GetActorLocation() - Start).Rotation();
            TargetRot.Pitch = 0.0f;
            TargetRot.Roll = 0.0f;

            WarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
                FName("CombatTarget"),
                TargetLoc,
                TargetRot
            );
        }
    }
}

void UAegisGameplayAbility_Melee::PerformWeaponTrace(FName StartSocket, FName EndSocket, float Radius)
{
    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (!Avatar) return;

    ACharacter* Character = Cast<ACharacter>(Avatar);
    if (!Character || !Character->GetMesh()) return;

    FVector StartLoc = Character->GetMesh()->GetSocketLocation(StartSocket);
    FVector EndLoc = Character->GetMesh()->GetSocketLocation(EndSocket);

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(Avatar);

    TArray<FHitResult> HitResults;
    bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
        GetWorld(),
        StartLoc,
        EndLoc,
        Radius,
        ObjectTypes,
        false,
        IgnoreActors,
        EDrawDebugTrace::None,
        HitResults,
        true
    );

    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            AActor* HitActor = Hit.GetActor();
            if (HitActor && !AlreadyHitActors.Contains(HitActor))
            {
                AlreadyHitActors.Add(HitActor);

                // Apply Damage GameplayEffect
                UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
                UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

                if (TargetASC && SourceASC && DamageEffectClass)
                {
                    FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
                    Context.AddHitResult(Hit);

                    FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, Context);
                    if (SpecHandle.IsValid())
                    {
                        SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
                    }
                }
            }
        }
    }
}

// Removed unused OnComboWindowOpened

void UAegisGameplayAbility_Melee::OnComboWindowClosed(FGameplayEventData Payload)
{
    TryAdvanceCombo();
}

bool UAegisGameplayAbility_Melee::TryAdvanceCombo()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    const FAegisGameplayTags& Tags = FAegisGameplayTags::Get();

    // If player queued up input during the combo window, advance to the next attack in the combo!
    if (ASC && ASC->HasMatchingGameplayTag(Tags.State_Combat_InputQueued) && ComboChain.IsValidIndex(CurrentComboIndex + 1))
    {
        ASC->RemoveLooseGameplayTag(Tags.State_Combat_InputQueued);
        ASC->RemoveLooseGameplayTag(Tags.State_Combat_CanCombo);
        CurrentComboIndex++;
        PlayCurrentComboAttack();
        return true;
    }
    return false;
}

void UAegisGameplayAbility_Melee::OnMontageCompleted()
{
    // Check one last time if we should advance when the montage naturally finishes blending out
    if (!TryAdvanceCombo())
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
}

void UAegisGameplayAbility_Melee::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UAegisGameplayAbility_Melee::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (ASC)
    {
        const FAegisGameplayTags& Tags = FAegisGameplayTags::Get();
        ASC->RemoveLooseGameplayTag(Tags.State_Combat_CanCombo);
        ASC->RemoveLooseGameplayTag(Tags.State_Combat_InputQueued);
    }

    CurrentComboIndex = 0;
}
