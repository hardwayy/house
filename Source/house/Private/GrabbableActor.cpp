#include "GrabbableActor.h"
#include "Components/StaticMeshComponent.h"
#include "houseCharacter.h"
AGrabbableActor::AGrabbableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	RootComponent = BaseMesh;

	
	BaseMesh->SetSimulatePhysics(true); 
	BaseMesh->SetMassOverrideInKg(NAME_None, 10.f, true); 
	BaseMesh->SetCollisionProfileName(TEXT("PhysicsActor")); 
	BaseMesh->ComponentTags.Add(FName("CanGrab"));
	BaseMesh->BodyInstance.bUseCCD = true;
	BaseMesh->SetLinearDamping(0.1f);
	BaseMesh->SetAngularDamping(0.5f);
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