#include "BreakableWindow.h"
#include "Components/StaticMeshComponent.h"
#include "Physics/Experimental/ChaosInterfaceUtils.h"
#include "DrawDebugHelpers.h" 
ABreakableWindow::ABreakableWindow()
{
	PrimaryActorTick.bCanEverTick = false;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;


	WindowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WindowMesh"));
	WindowMesh->SetupAttachment(RootComponent);
	WindowMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	WindowMesh->SetNotifyRigidBodyCollision(true);
	WindowMesh->bDisallowNanite = true;
	ShatteredMesh = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("ShatteredMesh"));
	ShatteredMesh->SetupAttachment(RootComponent);
	ShatteredMesh->SetCollisionProfileName(TEXT("NoCollision")); 
	ShatteredMesh->SetSimulatePhysics(false);
	ShatteredMesh->SetVisibility(false);
	ShatteredMesh->SetMassOverrideInKg(NAME_None, 50.0f);
	ShatteredMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShatteredMesh->SetSimulatePhysics(false);

    if (ShatteredMesh)
    {
        ShatteredMesh->SetMassOverrideInKg(NAME_None, 50.0f, true);
    }

	BreakThreshold = 15000.0f;
}

void ABreakableWindow::BeginPlay()
{
	Super::BeginPlay();
	WindowMesh->OnComponentHit.AddDynamic(this, &ABreakableWindow::OnHit);
}

void ABreakableWindow::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Calcoliamo la forza dell'impatto basandoci sull'impulso normale restituito dalla fisica
	float ImpactForce = NormalImpulse.Size();
    DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 10.0f, 12, FColor::Red, false, 5.0f);
	// Se la forza supera la soglia, scateniamo l'interfaccia
	if (ImpactForce >= BreakThreshold)
	{
		// Chiamiamo la funzione dell'interfaccia su noi stessi (o potremmo chiamarla esternamente)
		// NOTA: Execute_OnImpact è il modo corretto di chiamare funzioni di interfaccia in C++
		if (Implements<UImpactInterface>())
		{
			IImpactInterface::Execute_OnImpact(this, ImpactForce, Hit, OtherActor);
		}
	}
	else {
		if(Implements<UImpactInterface>())
		{

			IImpactInterface::Execute_OnUnsuccesfulImpact(this, ImpactForce, Hit, OtherActor);
			UE_LOG(LogTemp, Warning, TEXT("Impact force %f below threshold %f, window remains intact."), ImpactForce, BreakThreshold);
		}
	}
}

void ABreakableWindow::OnImpact_Implementation(float ImpactForce, const FHitResult& HitResult, AActor* InstigatorActor)
{
	if (!WindowMesh->IsVisible()) return;

	// --- SWAP MESH ---
	WindowMesh->SetVisibility(false);
	WindowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ShatteredMesh->SetVisibility(true);
	ShatteredMesh->SetMobility(EComponentMobility::Movable);

	// --- ATTIVAZIONE FISICA ---
	ShatteredMesh->SetSimulatePhysics(true);
	ShatteredMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ShatteredMesh->SetCollisionProfileName(TEXT("Destructible")); // o PhysicsActor

	// --- FIX MASSA CHIRURGICO ---
	// Accediamo direttamente al "cuore" del motore fisico
	FBodyInstance* BodyInst = ShatteredMesh->GetBodyInstance();
	if (BodyInst)
	{
		// 1. Forziamo il flag override
		BodyInst->bOverrideMass = true;

		// 2. Impostiamo il valore (es. 50kg)
		BodyInst->SetMassOverride(50,true);

		// 3. CRUCIALE: Diciamo al motore fisico di ricalcolare ORA
		BodyInst->UpdateMassProperties();
	}

	// Fallback standard (per sicurezza)
	ShatteredMesh->SetMassOverrideInKg(NAME_None, 50.0f, true);

	// --- SVEGLIA E IMPULSO ---
	ShatteredMesh->WakeAllRigidBodies();

	// Log di verifica immediata
	UE_LOG(LogTemp, Warning, TEXT("[DEBUG] Final Mass Check: %f kg"), ShatteredMesh->GetMass());

	FVector ImpulseDir = HitResult.ImpactNormal * -1.0f;
	ShatteredMesh->AddRadialImpulse(HitResult.ImpactPoint, 500.0f, ImpactForce, ERadialImpulseFalloff::RIF_Linear, true);
}
void ABreakableWindow::OnUnsuccesfulImpact_Implementation(float ImpactForce, const FHitResult& HitResult, AActor* InstigatorActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Window Hit with force: %f but did not break."), ImpactForce);
	TouchWindow(HitResult.ImpactPoint);
}