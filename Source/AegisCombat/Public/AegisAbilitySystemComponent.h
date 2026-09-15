#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "AegisAbilitySystemComponent.generated.h"

/**
 * UAegisAbilitySystemComponent
 * Custom ASC providing input buffering, combo state tracking, and character initialization.
 */
UCLASS()
class AEGISCOMBAT_API UAegisAbilitySystemComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()

public:
    UAegisAbilitySystemComponent();

    // Input Handling
    UFUNCTION(BlueprintCallable, Category = "Aegis|Input")
    void AbilityInputTagPressed(const FGameplayTag& InputTag);

    UFUNCTION(BlueprintCallable, Category = "Aegis|Input")
    void AbilityInputTagReleased(const FGameplayTag& InputTag);
    void ProcessAbilityInput(float DeltaTime, bool bGamePaused);

    // Buffering system
    bool TryBufferAbility(const FGameplayTag& InputTag);
    void ClearBufferedInput();

    // Startup utilities
    void AddCharacterAbilities(const TArray<TSubclassOf<class UGameplayAbility>>& StartupAbilities);

protected:
    FGameplayTag BufferedInputTag;
    float BufferTimer = 0.0f;
    const float MaxBufferWindow = 0.35f;
};
