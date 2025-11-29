#include "BTTask_ClearKey.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ClearKey::UBTTask_ClearKey()
{
	NodeName = "Clear Blackboard Key";
}

EBTNodeResult::Type UBTTask_ClearKey::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Otteniamo il componente Blackboard
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (BlackboardComp)
	{
		// QUESTA è la funzione magica. 
		// BlackboardKey è una variabile ereditata da UBTTask_BlackboardBase.
		// GetSelectedKeyID() ci dà l'ID della chiave che hai scelto nell'Editor.
		BlackboardComp->ClearValue(BlackboardKey.GetSelectedKeyID());

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}