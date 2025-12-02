#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "UBTService_CheckPathObstacle.generated.h"

/**
 * Service che controlla se ci sono oggetti interagibili (Porte, etc.) tra l'AI e il Giocatore.
 * Gestisce automaticamente la logica "Run & Open" se in Chase, o "Stop & Interact" se in Idle.
 */
UCLASS()
class HOUSE_API UUBTService_CheckPathObstacle : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UUBTService_CheckPathObstacle();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// Chiave Blackboard dove si trova il Target (il Giocatore) per calcolare la direzione
	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector TargetActorKey;

	// Chiave Blackboard per leggere lo stato attuale (es. Idle, Chase, Patrol)
	// Usa il tuo Enum ENeighborState o simile
	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector CurrentStateKey;

	// Distanza massima per controllare l'ostacolo (es. 300.0f per correre fluido)
	UPROPERTY(EditAnywhere, Category = "AI")
	float CheckDistance = 250.0f;

	// Tempo in secondi per ignorare una porta dopo aver interagito (es. 3.0s)
	// Impedisce il loop "Apro -> Chiudo -> Apro"
	UPROPERTY(EditAnywhere, Category = "AI")
	float ObstacleCooldown = 1.0f;

private:
	// Ricordiamo l'ultimo attore con cui abbiamo interagito
	TWeakObjectPtr<AActor> LastDetectedObstacle;

	// Quando abbiamo interagito l'ultima volta?
	double LastDetectionTime;
};