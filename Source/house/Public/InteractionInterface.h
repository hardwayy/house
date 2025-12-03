#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionInterface.generated.h"

UINTERFACE(MinimalAPI)
class UInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

class HOUSE_API IInteractionInterface
{
	GENERATED_BODY()

public:
	// Funzione per agire (Apri/Chiudi)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* InstigatorActor);

	// --- NUOVO: Funzione "Getter" ---
	// Chiede all'oggetto il suo stato. Ritorna TRUE se è già aperto/interagito.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool IsAlreadyOpen();
};