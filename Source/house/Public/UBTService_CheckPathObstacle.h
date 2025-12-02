#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "UBTService_CheckPathObstacle.generated.h"

UCLASS()
class HOUSE_API UUBTService_CheckPathObstacle : public UBTService_BlackboardBase
{
    GENERATED_BODY()

public:
    UUBTService_CheckPathObstacle();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "AI")
    FBlackboardKeySelector TargetActorKey;

    UPROPERTY(EditAnywhere, Category = "AI")
    float CheckDistance = 200.0f;

    // --- NUOVO: Cooldown System ---

    // Tempo in secondi per ignorare una porta dopo averla segnalata (es. 3 secondi)
    UPROPERTY(EditAnywhere, Category = "AI")
    float ObstacleCooldown = 3.0f;

private:
    // Ricordiamo l'ultimo attore segnalato
    TWeakObjectPtr<AActor> LastDetectedObstacle;

    // Quando lo abbiamo segnalato?
    double LastDetectionTime;
};