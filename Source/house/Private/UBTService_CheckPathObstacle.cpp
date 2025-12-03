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

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));

	// --- CALCOLO VETTORI ---
	FVector Start = AIPawn->GetActorLocation() + FVector(0, 0, 50);

	// Usiamo la velocità se ci muoviamo, o il forward se siamo bloccati
	FVector CheckDirection = (AIPawn->GetVelocity().SizeSquared() > 100.0f)
		? AIPawn->GetVelocity().GetSafeNormal()
		: AIPawn->GetActorForwardVector();

	float DynamicDistance = CheckDistance * 1.2f;
	FVector End = Start + (CheckDirection * DynamicDistance);

	// --- 1. CHECK NAVMESH ---
	// (Manteniamo questo per efficienza: se il NavMesh dice libero e ci muoviamo, non sprechiamo trace)
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		ANavigationData* NavData = NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
		if (NavData)
		{
			FVector NavHitLoc;
			bool bIsNavBlocked = NavData->Raycast(Start, End, NavHitLoc, nullptr);

			if (!bIsNavBlocked)
			{
				float CurrentSpeed = AIPawn->GetVelocity().Size();
				if (CurrentSpeed > 10.0f) return; // Via libera e movimento OK -> Esci
			}
		}
	}

	// --- 2. TRACE VISIVO ---
	float SphereRadius = 40.0f;
	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(AIPawn);
	if (TargetActor) Params.AddIgnoredActor(TargetActor);

	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults, Start, End, FQuat::Identity, ECC_Visibility,
		FCollisionShape::MakeSphere(SphereRadius), Params
	);

	double CurrentTime = GetWorld()->GetTimeSeconds();

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor) continue;

			if (HitActor->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
			{
				// --- NUOVO: CONTROLLO BOOLEANO DIRETTO ---
				// Chiediamo all'oggetto: "Sei già aperto?"
				// Nota: Execute_IsAlreadyOpen è la funzione generata automaticamente da Unreal per le interfacce BP
				bool bIsAlreadyOpen = IInteractionInterface::Execute_IsAlreadyOpen(HitActor);

				if (bIsAlreadyOpen)
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Door open, chasing!"));
					continue;
				}else{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Not open, trying to open!"));
				}

				// --- SE ARRIVIAMO QUI, È CHIUSO ---

				// Check Cooldown (Sicurezza extra)
				if (HitActor == LastDetectedObstacle.Get())
				{
					if ((CurrentTime - LastDetectionTime) < ObstacleCooldown) continue;
				}

				uint8 CurrentStateEnum = BlackboardComp->GetValueAsEnum(CurrentStateKey.SelectedKeyName);

				if (CurrentStateEnum == (uint8)ENeighborState::Chase)
				{
					// Run & Open
					IInteractionInterface::Execute_Interact(HitActor, AIPawn);
					LastDetectedObstacle = HitActor;
					LastDetectionTime = CurrentTime;
					return;
				}
				else
				{
					// Idle Stop
					BlackboardComp->SetValueAsObject(BlackboardKey.SelectedKeyName, HitActor);
					LastDetectedObstacle = HitActor;
					LastDetectionTime = CurrentTime;
					return;
				}
			}
		}
	}

	BlackboardComp->ClearValue(BlackboardKey.SelectedKeyName);
}