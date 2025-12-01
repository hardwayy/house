#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ImpactInterface.generated.h"

// UInterface standard boilerplate
UINTERFACE(MinimalAPI)
class UImpactInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interfaccia per attori che reagiscono agli impatti fisici.
 */
class HOUSE_API IImpactInterface
{
	GENERATED_BODY()

public:
	/**
	 * Evento scatenato quando avviene un impatto fisico rilevante.
	 * @param ImpactForce - La forza dell'impatto (magnitudine dell'impulso).
	 * @param HitResult - Dettagli sulla collisione (punto, normale, ecc.).
	 * @param InstigatorActor - L'attore che ha causato l'impatto (opzionale).
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Physics Interaction")
	void OnImpact(float ImpactForce, const FHitResult& HitResult, AActor* InstigatorActor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Physics Interaction")
	void OnUnsuccesfulImpact(float ImpactForce, const FHitResult& HitResult, AActor* InstigatorActor);
};