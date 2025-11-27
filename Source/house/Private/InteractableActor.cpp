#include "InteractableActor.h"
#include "Components/StaticMeshComponent.h"

AInteractableActor::AInteractableActor()
{
    PrimaryActorTick.bCanEverTick = true;


    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;


}

void AInteractableActor::Interact_Implementation(AActor* InstigatorActor)
{

	UE_LOG(LogTemp, Log, TEXT("Interact called on: %s. Overwrite this in Blueprint to do something!"), *GetName());
}