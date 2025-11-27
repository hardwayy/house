#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractionInterface.h" // Includi la TUA interfaccia
#include "InteractableActor.generated.h"

UCLASS()
class HOUSE_API AInteractableActor : public AActor, public IInteractionInterface
{
	GENERATED_BODY()

public:
	AInteractableActor();

protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* DefaultSceneRoot;

public:
	// --- INTERFACCIA ---
	// Questa funzione viene chiamata quando premi 'E'
	virtual void Interact_Implementation(AActor* InstigatorActor) override;
};