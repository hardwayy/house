#include "GrabbableActor.h"
#include "Components/StaticMeshComponent.h"
#include "houseCharacter.h"
#include "Perception/AISense_Hearing.h"
AGrabbableActor::AGrabbableActor()
{
    PrimaryActorTick.bCanEverTick = true;
    LastNoiseTime = -10.0f;
    // 1. Crea la Root invisibile
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    // 2. Crea la Mesh e attaccala alla Root
    BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
    BaseMesh->SetupAttachment(RootComponent);

    
    BaseMesh->SetSimulatePhysics(true);
    BaseMesh->SetMassOverrideInKg(NAME_None, 10.f, true);
    BaseMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
    BaseMesh->BodyInstance.bUseCCD = true;
    BaseMesh->SetLinearDamping(0.1f);
    BaseMesh->SetAngularDamping(0.5f);
    BaseMesh->SetNotifyRigidBodyCollision(true);
	BaseMesh->SetGenerateOverlapEvents(true);
    BaseMesh->BodyInstance.bUseCCD = true;

    // I tag vanno sulla mesh perché è quella che colpiamo col raggio
    BaseMesh->ComponentTags.Add(FName("CanGrab"));
}
void AGrabbableActor::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
    Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

    if (HasAuthority())
    {
        // 1. CHECK DEL COOLDOWN
        // Se non è passato abbastanza tempo dall'ultimo rumore, usciamo subito.
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastNoiseTime < NoiseCooldown)
        {
            return;
        }

        // 2. CHECK DELL'IMPULSO
        float ImpactForce = NormalImpulse.Size();
        if (ImpactForce > MinImpulseForNoise)
        {
            // Aggiorniamo il timer
            LastNoiseTime = CurrentTime;

            FName NoiseTag = TEXT("ObjectImpact");
            UAISense_Hearing::ReportNoiseEvent(GetWorld(), HitLocation, ImpactLoudness, this, 0.0f, NoiseTag);

            // Debug visivo (opzionale, rimuovi per pulizia)
            DrawDebugSphere(GetWorld(), HitLocation, 30.0f, 12, FColor::Red, false, 2.0f);
            UE_LOG(LogTemp, Warning, TEXT("BUM! Rumore emesso (Forza: %f)"), ImpactForce);
        }
    }
}

void AGrabbableActor::Interact_Implementation(AActor* InstigatorActor)
{
	// Usa il nuovo nome
	AhouseCharacter* PlayerCharacter = Cast<AhouseCharacter>(InstigatorActor);

	if (PlayerCharacter)
	{
		PlayerCharacter->AddToInventory(this);
		UE_LOG(LogTemp, Log, TEXT("%s has been picked up via Interaction."), *GetName());
	}
}
void AGrabbableActor::OnUse_Implementation(AActor* User)
{

	UE_LOG(LogTemp, Log, TEXT("OnUse called on %s (Default Implementation)"), *GetName());
}