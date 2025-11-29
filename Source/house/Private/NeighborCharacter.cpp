#include "NeighborCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/TextRenderComponent.h" // Necessario per il testo
#include "NeighborAIController.h" // Necessario per leggere l'Enum
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h" // Per OpenLevel

ANeighborCharacter::ANeighborCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Setup Movimento
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Setup TextRender (Debug)
	DebugStateText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DebugStateText"));
	DebugStateText->SetupAttachment(RootComponent);
	DebugStateText->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f)); // Sopra la testa
	DebugStateText->SetHorizontalAlignment(EHTA_Center);
	DebugStateText->SetTextRenderColor(FColor::Red);
	DebugStateText->SetText(FText::FromString("Init..."));
}

void ANeighborCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ANeighborCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Aggiorniamo il testo ogni frame
	UpdateDebugText();
}

void ANeighborCharacter::UpdateDebugText()
{
    if (!DebugStateText) return;

    // Check Controller
    AController* MyController = GetController();
    if (!MyController)
    {
        // Se non ho controller, è normale vedere Init...
        // UE_LOG(LogTemp, Warning, TEXT("CHARACTER: Nessun Controller")); 
        return;
    }

    ANeighborAIController* AICon = Cast<ANeighborAIController>(MyController);
    if (!AICon)
    {
        DebugStateText->SetText(FText::FromString("WRONG CONTROLLER"));
        DebugStateText->SetTextRenderColor(FColor::Purple); // Viola = Errore Classe
        return;
    }

    // Check Blackboard
    if (!AICon->GetBlackboardComponent())
    {
        DebugStateText->SetText(FText::FromString("NO BLACKBOARD"));
        DebugStateText->SetTextRenderColor(FColor::Orange); // Arancione = BB mancante
        return;
    }

    // Se arrivo qui, leggo lo stato
    uint8 StateIndex = AICon->GetBlackboardComponent()->GetValueAsEnum(TEXT("CurrentState"));

    // Per debuggare, forziamo la conversione in Int se l'Enum fallisce
    FString StateName = FString::FromInt(StateIndex);

    const UEnum* EnumPtr = StaticEnum<ENeighborState>();
    if (EnumPtr)
    {
        StateName = EnumPtr->GetNameStringByValue((int64)StateIndex);
    }

    DebugStateText->SetText(FText::FromString(StateName));

    if (StateName.Contains("Chase") || StateName.Contains("Cattura") || StateIndex == 3) // 3 = Chase
    {
        DebugStateText->SetTextRenderColor(FColor::Red);
    }
    else
    {
        DebugStateText->SetTextRenderColor(FColor::Green);
    }
}

void ANeighborCharacter::CatchPlayer()
{
	UE_LOG(LogTemp, Error, TEXT("PRESO! GAME OVER."));
	/*
	// Riavvia il livello dopo 1 secondo
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
		}, 1.0f, false);
		*/
}