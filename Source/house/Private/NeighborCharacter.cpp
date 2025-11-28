#include "NeighborCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

ANeighborCharacter::ANeighborCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Configurazione standard per un'IA che deve muoversi
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false; // Lascia che sia il MovementComponent a ruotare la mesh
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Il vicino si gira verso dove sta camminando
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);

	// IMPORTANTE: Assicura che l'IA prenda possesso del corpo appena il gioco inizia o viene spawnato
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ANeighborCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ANeighborCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ANeighborCharacter::CatchPlayer()
{
	// Logica futura: Riavvia il livello o riproduci animazione jump scare
	UE_LOG(LogTemp, Warning, TEXT("Il Vicino ha catturato il giocatore!"));
}