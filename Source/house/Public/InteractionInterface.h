// InteractionInterface.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionInterface.generated.h"

// Parte 1: Necessaria per la Reflection di Unreal (Non toccare)
UINTERFACE(MinimalAPI)
class UInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

// Parte 2: L'interfaccia reale (Scrivi qui)
class HOUSE_API IInteractionInterface
{
	GENERATED_BODY()

public:
	// Definisci la funzione "Interact".
	// BlueprintNativeEvent: Permette implementazione mista C++/Blueprint.
	// BlueprintCallable: Permette di chiamarla anche da altri nodi BP se serve.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Instigator);
};