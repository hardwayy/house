#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NeighborTypes.h"
#include "NeighborCharacter.generated.h"

// Forward Declaration (dice al compilatore che questa classe esiste)
class UTextRenderComponent;

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

	// Funzione chiamata dal Task per catturare il giocatore
	void CatchPlayer();

	// Componente per il testo sopra la testa (Dichiarazione fondamentale!)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
	UTextRenderComponent* DebugStateText;

private:
	// Funzione interna per aggiornare il testo
	void UpdateDebugText();
};