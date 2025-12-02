#include "UBTService_CheckPathObstacle.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "InteractionInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h" 

UUBTService_CheckPathObstacle::UUBTService_CheckPathObstacle()
{
	NodeName = "Check Path Obstacles";
	Interval = 0.15f;
	RandomDeviation = 0.05f;

	// Inizializziamo a 0
	LastDetectionTime = 0.0f;
}

void UUBTService_CheckPathObstacle::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* AIPawn = AIController ? AIController->GetPawn() : nullptr;
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!AIPawn || !BlackboardComp) return;

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor) return;

	// Calcoli vettoriali (Uguale a prima)
	FVector Start = AIPawn->GetActorLocation() + FVector(0, 0, 50);
	FVector TargetLoc = TargetActor->GetActorLocation() + FVector(0, 0, 50);
	FVector DirectionToTarget = (TargetLoc - Start).GetSafeNormal();

	float SphereRadius = 80.0f;
	FVector End = Start + (DirectionToTarget * CheckDistance);

	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(AIPawn);
	Params.AddIgnoredActor(TargetActor);

	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Start,
		End,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(SphereRadius),
		Params
	);

#if UE_BUILD_DEVELOPMENT 
	
	DrawDebugSphere(GetWorld(), End, SphereRadius, 8, bHit ? FColor::Green : FColor::Red, false, 0.2f);
#endif

	// Tempo attuale del gioco
	double CurrentTime = GetWorld()->GetTimeSeconds();

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor) continue;

			if (HitActor->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
			{
				UE_LOG(LogTemp, Log, TEXT("Ostacolo rilevato: %s"), *HitActor->GetName());
				// --- NUOVA LOGICA COOLDOWN ---

				// 1. È lo stesso attore di prima?
				if (HitActor == LastDetectedObstacle.Get())
				{
					// 2. È passato abbastanza tempo?
					if ((CurrentTime - LastDetectionTime) < ObstacleCooldown)
					{
						// COOLDOWN ATTIVO: Ignoriamo questa porta per permettere all'AI di passarci attraverso
						// UE_LOG(LogTemp, Log, TEXT("Ignoro %s per cooldown"), *HitActor->GetName());
						continue;
					}
				}
				UE_LOG(LogTemp, Log, TEXT("Segnalo ostacolo %s alla AI"), *HitActor->GetName());

				// Trovato ostacolo valido (nuovo o cooldown scaduto)
				// Salviamo timestamp e attore
				LastDetectedObstacle = HitActor;
				LastDetectionTime = CurrentTime;

				BlackboardComp->SetValueAsObject(BlackboardKey.SelectedKeyName, HitActor);
				UE_LOG(LogTemp, Log, TEXT("key assegnata come: %s"), *HitActor->GetName());
				return;
			}
		}
	}

	BlackboardComp->ClearValue(BlackboardKey.SelectedKeyName);
}