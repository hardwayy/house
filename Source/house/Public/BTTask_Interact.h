#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_Interact.generated.h"

/**
 * Task generico per interagire con qualsiasi attore che implementa IInteractionInterface.
 * Prende l'attore dalla Blackboard Key specificata.
 */
UCLASS()
class HOUSE_API UBTTask_Interact : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_Interact();

	/** Funzione principale che esegue la logica del nodo */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};