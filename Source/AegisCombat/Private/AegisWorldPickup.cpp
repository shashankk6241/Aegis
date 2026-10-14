#include "AegisWorldPickup.h"
#include "AegisItemDefinitionDataAsset.h"
#include "AegisInventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

AAegisWorldPickup::AAegisWorldPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->InitSphereRadius(100.0f);
	SphereComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = SphereComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bAutoPickupOnOverlap = true;
	Quantity = 1;
}

void AAegisWorldPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshMeshFromDefinition();
}

void AAegisWorldPickup::BeginPlay()
{
	Super::BeginPlay();

	RefreshMeshFromDefinition();

	if (SphereComponent)
	{
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AAegisWorldPickup::HandleSphereBeginOverlap);
	}
}

void AAegisWorldPickup::RefreshMeshFromDefinition()
{
	if (MeshComponent && ItemDefinition && ItemDefinition->WorldMesh)
	{
		MeshComponent->SetStaticMesh(ItemDefinition->WorldMesh);
	}
}

void AAegisWorldPickup::HandleSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bAutoPickupOnOverlap && OtherActor)
	{
		CollectPickup(OtherActor);
	}
}

bool AAegisWorldPickup::CollectPickup(AActor* Interactor)
{
	if (!Interactor || !ItemDefinition || Quantity <= 0) return false;

	UAegisInventoryComponent* Inventory = Interactor->FindComponentByClass<UAegisInventoryComponent>();
	if (!Inventory) return false;

	if (Inventory->AddItem(ItemDefinition, Quantity))
	{
		Destroy();
		return true;
	}

	return false;
}
