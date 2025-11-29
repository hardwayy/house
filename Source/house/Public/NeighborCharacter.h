#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NeighborTypes.h"
#include "NeighborCharacter.generated.h"

// Forward Declaration (dice al compilatore che questa classe esiste)
class UTextRenderComponent;
UENUM(BlueprintType)
enum class E_NeighborState : uint8
{
	Init        UMETA(DisplayName = "Init"),
	Idle        UMETA(DisplayName = "Idle"),
	Patrol      UMETA(DisplayName = "Patrol"),
	Chase       UMETA(DisplayName = "Chase"),
	Investigate UMETA(DisplayName = "Investigate"),
	Hunt        UMETA(DisplayName = "Hunt")
};
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

	UFUNCTION(BlueprintImplementableEvent, Category = "AI Events")
	void OnAIStateChanged(E_NeighborState NewState);

private:
	// Funzione interna per aggiornare il testo
	void UpdateDebugText();
};