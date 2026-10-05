#include "AegisWeapon.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"

AAegisWeapon::AAegisWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TrailComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailComponent"));
	TrailComponent->SetupAttachment(WeaponMesh);
	TrailComponent->bAutoActivate = false;
}

void AAegisWeapon::EquipToCharacter(ACharacter* InCharacter, FName SocketName)
{
	if (!InCharacter || !InCharacter->GetMesh()) return;

	AttachToComponent(
		InCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		SocketName
	);

	SetOwner(InCharacter);
}

void AAegisWeapon::ActivateWeaponTrail()
{
	if (TrailComponent && TrailSystem)
	{
		TrailComponent->SetAsset(TrailSystem);
		TrailComponent->Activate(true);
	}
}

void AAegisWeapon::DeactivateWeaponTrail()
{
	if (TrailComponent && TrailComponent->IsActive())
	{
		TrailComponent->Deactivate();
	}
}

FVector AAegisWeapon::GetTraceStartLocation() const
{
	return WeaponMesh ? WeaponMesh->GetSocketLocation(SocketStart) : GetActorLocation();
}

FVector AAegisWeapon::GetTraceEndLocation() const
{
	return WeaponMesh ? WeaponMesh->GetSocketLocation(SocketEnd) : GetActorLocation() + GetActorForwardVector() * 100.0f;
}
