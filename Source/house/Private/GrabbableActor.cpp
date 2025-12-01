#include "GrabbableActor.h"
#include "Components/StaticMeshComponent.h"
#include "houseCharacter.h"
AGrabbableActor::AGrabbableActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // 1. Crea la Root invisibile
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    // 2. Crea la Mesh e attaccala alla Root
    BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
    BaseMesh->SetupAttachment(RootComponent);

    
    BaseMesh->SetSimulatePhysics(true);
    BaseMesh->SetMassOverrideInKg(NAME_None, 10.f, true);
    BaseMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
    BaseMesh->BodyInstance.bUseCCD = true;
    BaseMesh->SetLinearDamping(0.1f);
    BaseMesh->SetAngularDamping(0.5f);
    BaseMesh->SetNotifyRigidBodyCollision(true);

    // I tag vanno sulla mesh perché è quella che colpiamo col raggio
    BaseMesh->ComponentTags.Add(FName("CanGrab"));
}

void AGrabbableActor::Interact_Implementation(AActor* InstigatorActor)
{
	// Usa il nuovo nome
	AhouseCharacter* PlayerCharacter = Cast<AhouseCharacter>(InstigatorActor);

	if (PlayerCharacter)
	{
		PlayerCharacter->AddToInventory(this);
		UE_LOG(LogTemp, Log, TEXT("%s has been picked up via Interaction."), *GetName());
	}
}
void AGrabbableActor::OnUse_Implementation(AActor* User)
{

	UE_LOG(LogTemp, Log, TEXT("OnUse called on %s (Default Implementation)"), *GetName());
}