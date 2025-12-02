#include "UBTService_CheckPathObstacle.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "InteractionInterface.h"
#include "NeighborTypes.h" // <--- Assicurati che questo sia il file col tuo Enum
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
UUBTService_CheckPathObstacle::UUBTService_CheckPathObstacle()
{
	NodeName = "Check Path Obstacles";

	// Frequenza alta (0.15s) per reattività immediata sulle porte in faccia
	Interval = 0.15f;
	RandomDeviation = 0.05f;

	// Inizializziamo il tempo a 0
	LastDetectionTime = 0.0f;
}

void UUBTService_CheckPathObstacle::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* AIPawn = AIController ? AIController->GetPawn() : nullptr;
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!AIPawn || !BlackboardComp) return;

	// 1. Recuperiamo il Target (Giocatore)
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));

	// Se non c'è un target, non possiamo calcolare la direzione "intelligente". Usciamo.
	if (!TargetActor) return;

	// 2. Calcoli Vettori (Verso il Target)
	// Alziamo di 50 unità (Z) per partire dal petto e non dai piedi (evita tappeti/pavimento)
	FVector Start = AIPawn->GetActorLocation() + FVector(0, 0, 50);
	FVector TargetLoc = TargetActor->GetActorLocation() + FVector(0, 0, 50);

	// Direzione normalizzata verso il giocatore
	FVector DirectionToTarget = (TargetLoc - Start).GetSafeNormal();

	// Calcoliamo la fine del raggio
	// Aumentiamo leggermente il raggio se siamo in movimento veloce per aprire prima
	float DynamicDistance = CheckDistance * 1.1f;
	FVector End = Start + (DirectionToTarget * DynamicDistance);
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		// Recuperiamo l'istanza dei dati di navigazione (Il NavMesh corrente)
		ANavigationData* NavData = NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);

		if (NavData)
		{
			FVector NavHitLoc;
			// Raycast diretto sui dati di navigazione.
			// Ritorna TRUE se colpisce un bordo (percorso bloccato).
			// Ritorna FALSE se il raggio viaggia libero (percorso calpestabile).
			bool bIsPathBlocked = NavData->Raycast(Start, End, NavHitLoc, nullptr);

			if (!bIsPathBlocked)
			{
				// VIA LIBERA! Il NavMesh dice che si può passare.
				// Significa che se c'è una porta, è aperta (il NavMesh dinamico si è aggiornato).
				// Quindi usciamo e NON cerchiamo di interagire.
				UE_LOG(LogTemp, Log, TEXT("Path libero secondo il NavMesh, nessuna interazione necessaria."));
				return;
			}
		}
	}
	// Usiamo una sfera piccola (30cm) per evitare di colpire muri laterali o stipiti inutili
	float SphereRadius = 80.0f;

	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(AIPawn);
	Params.AddIgnoredActor(TargetActor);

	// 3. Eseguiamo la Sweep (Multi Trace)
	// Multi perché vogliamo attraversare eventuali stipiti non interagibili e trovare la porta vera
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Start,
		End,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(SphereRadius),
		Params
	);

	// Debug Visivo (Solo in Editor/Development)
#if UE_BUILD_DEVELOPMENT 
	// Verde se colpisce qualcosa, Rosso se vuoto
	DrawDebugSphere(GetWorld(), End, SphereRadius, 8, bHit ? FColor::Green : FColor::Red, false, 0.2f);
#endif

	// Tempo attuale del gioco per il Cooldown
	double CurrentTime = GetWorld()->GetTimeSeconds();

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor) continue;

			// Controlliamo se implementa l'interfaccia di interazione
			if (HitActor->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
			{
				// --- LOGICA COOLDOWN ---
				// Se è lo stesso attore dell'ultima volta...
				if (HitActor == LastDetectedObstacle.Get())
				{
					// ...e non è passato abbastanza tempo, IGNORALO.
					if ((CurrentTime - LastDetectionTime) < ObstacleCooldown)
					{
						continue;
					}
				}

				// --- LOGICA DI COMPORTAMENTO (CHASE vs IDLE) ---

				// Leggiamo lo stato attuale dalla Blackboard
				uint8 CurrentStateEnum = BlackboardComp->GetValueAsEnum(CurrentStateKey.SelectedKeyName);

				// CASO 1: STIAMO INSEGUENDO (CHASE) -> "RUN & OPEN"
				if (CurrentStateEnum == (uint8)ENeighborState::Chase)
				{
					// Agiamo SUBITO senza fermare il movimento
					UE_LOG(LogTemp, Warning, TEXT("RUN & OPEN: Apro %s al volo!"), *HitActor->GetName());

					IInteractionInterface::Execute_Interact(HitActor, AIPawn);

					// Aggiorniamo la memoria del cooldown
					LastDetectedObstacle = HitActor;
					LastDetectionTime = CurrentTime;

					// IMPORTANTE: Non settiamo la BlackboardKey "BlockingObject".
					// In questo modo il Behavior Tree NON cambia ramo e il MoveTo continua fluido.
					return;
				}

				// CASO 2: SIAMO TRANQUILLI (IDLE/PATROL) -> "STOP & INTERACT"
				else
				{
					UE_LOG(LogTemp, Log, TEXT("STOP & INTERACT: Ho trovato %s, mi fermo."), *HitActor->GetName());

					// Segnaliamo l'oggetto al Behavior Tree
					// Questo attiverà la Sequence "Resolve Obstacle" che ferma l'AI.
					BlackboardComp->SetValueAsObject(BlackboardKey.SelectedKeyName, HitActor);

					// Aggiorniamo la memoria
					LastDetectedObstacle = HitActor;
					LastDetectionTime = CurrentTime;

					return;
				}
			}
		}
	}

	// Se non abbiamo trovato nulla o siamo in cooldown, puliamo la variabile
	// (Così se eravamo in Idle, riprendiamo a camminare)
	BlackboardComp->ClearValue(BlackboardKey.SelectedKeyName);
}