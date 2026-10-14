#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AegisItemTypes.h"
#include "AegisWorldPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UAegisItemDefinitionDataAsset;

/**
 * AAegisWorldPickup
 * Physical actor placed or spawned in the level representing loot or dropped items.
 * Interacting or walking over it transfers the item to the player's inventory.
 */
UCLASS()
class AEGISCOMBAT_API AAegisWorldPickup : public AActor
{
	GENERATED_BODY()

public:
	AAegisWorldPickup();

	virtual void OnConstruction(const FTransform& Transform) override;

	// ==========================================
	// Configuration
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aegis|Pickup", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UAegisItemDefinitionDataAsset> ItemDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aegis|Pickup", meta = (ClampMin = "1", ExposeOnSpawn = "true"))
	int32 Quantity = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aegis|Pickup")
	bool bAutoPickupOnOverlap = true;

	// ==========================================
	// Interaction
	// ==========================================

	/** Manual pickup interaction */
	UFUNCTION(BlueprintCallable, Category = "Aegis|Pickup")
	bool CollectPickup(AActor* Interactor);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UFUNCTION()
	void HandleSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void RefreshMeshFromDefinition();
};
