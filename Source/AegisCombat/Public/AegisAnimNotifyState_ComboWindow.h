#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AegisAnimNotifyState_ComboWindow.generated.h"

/**
 * UAegisAnimNotifyState_ComboWindow
 * Opens the input buffering combo window, allowing the player to queue the subsequent chain attack.
 */
UCLASS()
class AEGISCOMBAT_API UAegisAnimNotifyState_ComboWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAegisAnimNotifyState_ComboWindow();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
