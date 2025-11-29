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

	UE_LOG(LogTemp, Warning, TEXT("AI CONTROLLER: Preso possesso di %s"), *InPawn->GetName());

	// Controllo 1: La variabile è assegnata?
	if (BehaviorTreeAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("AI CONTROLLER: Behavior Tree trovato! Provo ad avviarlo..."));

		// Controllo 2: L'avvio ha successo?
		bool bSuccess = RunBehaviorTree(BehaviorTreeAsset);

		if (bSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("AI CONTROLLER: Behavior Tree avviato con SUCCESSO!"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AI CONTROLLER ERRORE: RunBehaviorTree ha restituito FALSE! Il Behavior Tree ha una Blackboard assegnata?"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AI CONTROLLER ERRORE: BehaviorTreeAsset è NULL! Hai assegnato il BT nel Blueprint del Controller?"));
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
			BlackboardComp->SetValueAsEnum(TEXT("CurrentState"), (uint8)ENeighborState::Chase);
			BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);
			BlackboardComp->SetValueAsVector(TEXT("LastKnownLocation"), Stimulus.StimulusLocation);

			UE_LOG(LogTemp, Warning, TEXT("INSEGUIMENTO ATTIVO: %s"), *Actor->GetName());
		}
		else if (BlackboardComp)
		{

			BlackboardComp->SetValueAsVector(TEXT("LastKnownLocation"), Stimulus.StimulusLocation);

			// 2. Cancelliamo il bersaglio diretto (così smette di "barare" inseguendo attraverso i muri)
			BlackboardComp->ClearValue(TEXT("TargetActor"));

			UE_LOG(LogTemp, Log, TEXT("PERSO! Vado a investigare l'ultima posizione."));
		}
	}
}