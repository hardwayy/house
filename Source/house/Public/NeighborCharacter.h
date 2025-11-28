#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NeighborCharacter.generated.h"

UCLASS()
class HOUSE_API ANeighborCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ANeighborCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Utile per debuggare o per logica specifica (es. "Ti ho preso!")
	UFUNCTION(BlueprintCallable, Category = "Neighbor AI")
	void CatchPlayer();
};