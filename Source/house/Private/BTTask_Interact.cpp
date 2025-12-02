#include "BTTask_Interact.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "InteractionInterface.h" //

UBTTask_Interact::UBTTask_Interact()
{
	NodeName = "Interact With Target"; // Nome che vedrai nell'editor del Behavior Tree

	// Filtriamo la chiave della blackboard per accettare solo Oggetti (Actors)
	BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Interact, BlackboardKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_Interact::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 1. Otteniamo il Controller AI
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	// 2. Recuperiamo l'oggetto dalla Blackboard usando la chiave selezionata
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(BlackboardKey.SelectedKeyName));

	if (!TargetActor)
	{
		// Se la variabile è vuota, falliamo silenziosamente
		return EBTNodeResult::Failed;
	}

	// 3. Verifichiamo se l'oggetto implementa l'interfaccia
	// Nota: Usiamo Implements<UInteractionInterface> per il controllo C++ nativo/misto
	if (TargetActor->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
	{


		IInteractionInterface::Execute_Interact(TargetActor, AIController->GetPawn());

		// Successo!
		return EBTNodeResult::Succeeded;
	}

	// L'oggetto non era interagibile
	return EBTNodeResult::Failed;
}