#include "BreakableWindow.h"
#include "Components/PrimitiveComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Physics/Experimental/ChaosInterfaceUtils.h"
#include "DrawDebugHelpers.h"

ABreakableWindow::ABreakableWindow()
{
	PrimaryActorTick.bCanEverTick = false;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	// 1. Creiamo direttamente la Geometry Collection come protagonista
	ShatteredMesh = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("ShatteredMesh"));
	ShatteredMesh->SetupAttachment(RootComponent);

	// 2. CONFIGURAZIONE INIZIALE: "SOLIDO E FERMO"
	// BlockAllDynamic permette di bloccare oggetti e ricevere l'evento Hit
	ShatteredMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	ShatteredMesh->SetNotifyRigidBodyCollision(true); // Fondamentale per ricevere OnHit
	ShatteredMesh->SetGenerateOverlapEvents(true);

	// IMPORTANTE: Disabilitando la fisica all'inizio, la GC agisce come un oggetto Statico (Kinematic).
	// Non cadrà e non si romperà finché non lo diciamo noi.
	ShatteredMesh->SetSimulatePhysics(false);

	// Visibile da subito
	ShatteredMesh->SetVisibility(true);

	// Override Massa (opzionale ma consigliato per vetri leggeri)
	if (ShatteredMesh)
	{
		ShatteredMesh->SetMassOverrideInKg(NAME_None, 50.0f, true);
	}

	BreakThreshold = 15000.0f;
}

void ABreakableWindow::BeginPlay()
{
	Super::BeginPlay();

	// 3. Ci iscriviamo all'evento Hit direttamente sulla Geometry Collection
	if (ShatteredMesh)
	{
		ShatteredMesh->OnComponentHit.AddDynamic(this, &ABreakableWindow::OnHit);
	}
}

void ABreakableWindow::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Se la fisica è già attiva, non serve ricalcolare la rottura (evita spam di eventi sui frammenti a terra)
	if (ShatteredMesh->IsSimulatingPhysics()) return;

	float ImpactForce = NormalImpulse.Size();

	// Debug visivo del punto d'impatto
	DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 10.0f, 12, FColor::Red, false, 2.0f);
	UE_LOG(LogTemp, Log, TEXT("Impact Force Detected: %f"), ImpactForce);

	if (ImpactForce >= BreakThreshold)
	{
		if (Implements<UImpactInterface>())
		{
			IImpactInterface::Execute_OnImpact(this, ImpactForce, Hit, OtherActor);
		}
	}
	else
	{
		if (Implements<UImpactInterface>())
		{
			IImpactInterface::Execute_OnUnsuccesfulImpact(this, ImpactForce, Hit, OtherActor);
		}
	}
}

void ABreakableWindow::OnImpact_Implementation(float ImpactForce, const FHitResult& HitResult, AActor* InstigatorActor)
{
	if (!ShatteredMesh) return;

	UE_LOG(LogTemp, Warning, TEXT("WINDOW BREAKING!"));

	// 4. ATTIVAZIONE: SVEGLIA LA GC!
	// Abilitando la fisica ora, la GC smette di essere "Statica" e risponde alla gravità e agli impulsi.
	ShatteredMesh->SetSimulatePhysics(true);

	// Cambiamo profilo di collisione se necessario (spesso "Destructible" o lasciamo quello corrente se funziona)
	ShatteredMesh->SetCollisionProfileName(TEXT("Destructible"));

	// Assicuriamoci che tutti i pezzi siano svegli
	ShatteredMesh->WakeAllRigidBodies();

	// 5. APPLICAZIONE FORZA
	// Applichiamo un impulso nel punto esatto in cui l'oggetto ha colpito per spingere i frammenti
	// Invertiamo la normale per spingere "dentro" la finestra
	FVector ImpulseDir = HitResult.ImpactNormal * -1.0f;

	// AddRadialImpulse è ottimo per le GC perché "esplode" la zona colpita
	// Radius 500.0f assicura che i pezzi vicini vengano spinti via
	ShatteredMesh->AddRadialImpulse(HitResult.ImpactPoint, 500.0f, ImpactForce * 0.5f, ERadialImpulseFalloff::RIF_Linear, true);

	// Rimuoviamo il delegate Hit per risparmiare performance (i frammenti a terra non devono richiamare questa logica)
	ShatteredMesh->OnComponentHit.RemoveDynamic(this, &ABreakableWindow::OnHit);
}

void ABreakableWindow::OnUnsuccesfulImpact_Implementation(float ImpactForce, const FHitResult& HitResult, AActor* InstigatorActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Impact too weak: %f / %f"), ImpactForce, BreakThreshold);
	// Qui potresti mettere un suono di "tump" o un effetto particellare di polvere
}