#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AegisWeapon.generated.h"

class UStaticMeshComponent;
class UNiagaraComponent;
class UNiagaraSystem;

/**
 * AAegisWeapon
 * Equippable weapon actor managing mesh attachment, sockets, and Niagara weapon trail effects.
 */
UCLASS()
class AEGISCOMBAT_API AAegisWeapon : public AActor
{
	GENERATED_BODY()

public:
	AAegisWeapon();

	// Equipping
	UFUNCTION(BlueprintCallable, Category = "Aegis|Weapon")
	void EquipToCharacter(ACharacter* InCharacter, FName SocketName = FName("hand_r_socket"));

	// Trail Effects
	UFUNCTION(BlueprintCallable, Category = "Aegis|Weapon")
	void ActivateWeaponTrail();

	UFUNCTION(BlueprintCallable, Category = "Aegis|Weapon")
	void DeactivateWeaponTrail();

	// Socket Getters
	FVector GetTraceStartLocation() const;
	FVector GetTraceEndLocation() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> TrailComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> TrailSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sockets")
	FName SocketStart = FName("Socket_WeaponStart");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sockets")
	FName SocketEnd = FName("Socket_WeaponEnd");
};
