#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AegisAnimNotifyState_WeaponTrace.generated.h"

/**
 * UAegisAnimNotifyState_WeaponTrace
 * Activates multi-sphere weapon hit detection along specified mesh sockets during swing frames.
 */
UCLASS()
class AEGISCOMBAT_API UAegisAnimNotifyState_WeaponTrace : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAegisAnimNotifyState_WeaponTrace();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	FName StartSocketName = FName("WeaponStart");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	FName EndSocketName = FName("WeaponEnd");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float TraceRadius = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	bool bDrawDebug = false;
};
