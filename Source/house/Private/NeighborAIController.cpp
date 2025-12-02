#include "NeighborAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameFramework/Character.h"
#include "NeighborCharacter.h" 
#include "Perception/AISenseConfig_Hearing.h" 
#include "Perception/AISense_Hearing.h"
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

		AIPerceptionComp->ConfigureSense(*SightConfig);
	}

	// 3. Setup Configurazione Udito (NUOVO)
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	if (HearingConfig)
	{
		HearingConfig->HearingRange = 3000.0f; // Sente fino a 30 metri
		// HearingConfig->LoSHearingRange = 3500.0f; // Opzionale: udito diverso se non ci sono muri

		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

		AIPerceptionComp->ConfigureSense(*HearingConfig);
	}

	// Impostiamo la vista come senso dominante (opzionale, ma utile per logica interna UE)
	if (SightConfig)
	{
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
EBlackboardNotificationResult ANeighborAIController::OnBlackboardStateChanged(const UBlackboardComponent& InBlackboard, FBlackboard::FKey ChangedKeyID)
{
	if (ChangedKeyID == CurrentStateKeyID)
	{
		// 1. Recuperiamo il valore dalla Blackboard
		FName KeyName = InBlackboard.GetKeyName(ChangedKeyID);
		uint8 EnumValue = InBlackboard.GetValueAsEnum(KeyName);

		// 2. Chiamiamo l'evento BLUEPRINT sul Character
		if (ANeighborCharacter* MyNeighbor = Cast<ANeighborCharacter>(GetPawn()))
		{
			MyNeighbor->OnAIStateChanged(static_cast<E_NeighborState>(EnumValue));
		}

		return EBlackboardNotificationResult::ContinueObserving;
	}

	return EBlackboardNotificationResult::ContinueObserving;
}
void ANeighborAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
	// Recuperiamo il personaggio controllato dal giocatore
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	UBlackboardComponent* BlackboardComp = GetBlackboardComponent();

	if (!BlackboardComp) return;

	// --- LOGICA VISTA ---
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (Actor == PlayerPawn)
		{
			if (Stimulus.WasSuccessfullySensed())
			{
				// Visto! Inseguimento
				BlackboardComp->SetValueAsEnum(TEXT("CurrentState"), (uint8)ENeighborState::Chase);
				BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);
				BlackboardComp->SetValueAsVector(TEXT("LastKnownLocation"), Stimulus.StimulusLocation);

				UE_LOG(LogTemp, Warning, TEXT("VISTA: Ti vedo! Inseguo %s"), *Actor->GetName());
			}
			else
			{
				// Perso di vista -> Investigo ultima posizione nota
				//BlackboardComp->SetValueAsVector(TEXT("LastKnownLocation"), Stimulus.StimulusLocation);
				//BlackboardComp->ClearValue(TEXT("TargetActor")); // Smetto di mirare direttamente all'actor

				//UE_LOG(LogTemp, Log, TEXT("VISTA: Perso! Vado a investigare l'ultima posizione."));
			}
		}
	}
	// --- LOGICA UDITO (NUOVO) ---
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			FVector NoiseLocation = Stimulus.StimulusLocation;
			BlackboardComp->SetValueAsVector(TEXT("LastKnownLocation"), NoiseLocation);

			uint8 CurrentState = BlackboardComp->GetValueAsEnum(TEXT("CurrentState"));

			// SE NON STA GIÀ CORRENDO DIETRO AL GIOCATORE:
			if (CurrentState != (uint8)ENeighborState::Investigate)
			{

				BlackboardComp->SetValueAsEnum(TEXT("CurrentState"), (uint8)ENeighborState::Investigate);

				UE_LOG(LogTemp, Warning, TEXT("UDITO: Ho sentito un rumore! Cambio stato e vado a controllare."));
			}
		}
	}
}