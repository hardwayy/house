#include "NeighborAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h" 
#include "BehaviorTree/BlackboardComponent.h" 
#include <Kismet/GameplayStatics.h>
ANeighborAIController::ANeighborAIController()
{
	// 1. Setup Componente Percezione
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	SetPerceptionComponent(*AIPerceptionComp);

	// 2. Setup Configurazione Vista
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	if (SightConfig)
	{
		SightConfig->SightRadius = 1500.0f;
		SightConfig->LoseSightRadius = 1800.0f;
		SightConfig->PeripheralVisionAngleDegrees = 90.0f;

		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

		// Applica la configurazione
		AIPerceptionComp->ConfigureSense(*SightConfig);
		AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
	}
}

void ANeighborAIController::BeginPlay()
{
	Super::BeginPlay();
	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ANeighborAIController::OnTargetDetected);
	}
}


void ANeighborAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 1. Avviamo il Behavior Tree e inizializziamo la Blackboard
	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}
}

void ANeighborAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	// Recuperiamo il personaggio controllato dal giocatore
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	// Verifichiamo che sia il player e non un altro oggetto
	if (Actor == PlayerPawn)
	{
		// Otteniamo la Blackboard per scriverci sopra
		UBlackboardComponent* BlackboardComp = GetBlackboardComponent();

		if (BlackboardComp && Stimulus.WasSuccessfullySensed())
		{
			
			BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);
			BlackboardComp->SetValueAsVector(TEXT("LastKnownLocation"), Stimulus.StimulusLocation);

			UE_LOG(LogTemp, Warning, TEXT("INSEGUIMENTO ATTIVO: %s"), *Actor->GetName());
		}
		else if (BlackboardComp)
		{

			BlackboardComp->SetValueAsVector(TEXT("LastKnownLocation"), Stimulus.StimulusLocation);

			UE_LOG(LogTemp, Log, TEXT("PERSO DI VISTA. Vado all'ultima posizione nota."));
		}
	}
}