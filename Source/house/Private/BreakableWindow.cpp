	#include "BreakableWindow.h"
	#include "Components/StaticMeshComponent.h"
	#include "GeometryCollection/GeometryCollectionComponent.h"
	#include "GameFramework/Character.h" 
	#include "Field/FieldSystemTypes.h" // Necessario per applicare Strain precisi

	ABreakableWindow::ABreakableWindow()
	{
		PrimaryActorTick.bCanEverTick = false;

		RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
		RootComponent = RootScene;

		// --- 1. MESH SANA (IL "MURO") ---
		WindowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WindowMesh"));
		WindowMesh->SetupAttachment(RootComponent);
	
		// Configurazione Collisioni: Blocca tutto, genera Hit, MA NON SI MUOVE.
		WindowMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		WindowMesh->SetNotifyRigidBodyCollision(true); 
	
		// <--- FIX FONDAMENTALE: Mai simulare fisica sulla mesh sana. Deve restare inchiodata.
		WindowMesh->SetSimulatePhysics(false); 
		WindowMesh->SetMobility(EComponentMobility::Static); // O Stationary, basta che non sia Movable+Physics

		// --- 2. GEOMETRY COLLECTION (IL "CAOS") ---
		ShatteredMesh = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("ShatteredMesh"));
		ShatteredMesh->SetupAttachment(RootComponent);

		// Configurazione Iniziale: Invisibile e Spenta
		ShatteredMesh->SetVisibility(true);
		ShatteredMesh->SetSimulatePhysics(false);
		ShatteredMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
		// Deve essere Movable per potersi svegliare dopo
		ShatteredMesh->SetMobility(EComponentMobility::Movable); 

		BreakThreshold = 50.0f; // La soglia del NOSTRO codice (non di Chaos)
	}

	void ABreakableWindow::BeginPlay()
	{
		Super::BeginPlay();
	
		// Forza bruta all'inizio del gioco per assicurarsi che la finestra non cada
		if(WindowMesh)
		{
			WindowMesh->SetSimulatePhysics(false);
			WindowMesh->OnComponentHit.AddDynamic(this, &ABreakableWindow::OnHit);
			UE_LOG(LogTemp, Warning, TEXT("registered hit event!"));
		}
		if(ShatteredMesh)
		{
			ShatteredMesh->SetVisibility(false);
			UE_LOG(LogTemp, Warning, TEXT("Shattered mesh hidden at begin play."));
		}
	}

	void ABreakableWindow::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
	{
		// Se la ShatteredMesh è già visibile, vuol dire che siamo già rotti.
		if (ShatteredMesh->IsVisible()) return;

		// --- RIMOSSO: ShatteredMesh->SetVisibility(true); --- 
		// NON toccare la visibilità qui! Lo faremo solo se si rompe davvero.

		float ImpactForce = NormalImpulse.Size();

		// Logica Player (Breaching)
		ACharacter* CharacterRef = Cast<ACharacter>(OtherActor);
		if (CharacterRef && CharacterRef->GetVelocity().Size() > 400.0f)
		{
			ImpactForce = BreakThreshold + 1000.0f;
		}

		UE_LOG(LogTemp, Warning, TEXT("Hit Force: %f"), ImpactForce);

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
		// Doppio controllo di sicurezza
		if (ShatteredMesh->IsVisible()) return;

		UE_LOG(LogTemp, Warning, TEXT("--- WINDOW SHATTERING ---"));

		// 1. SWAP MESH
		WindowMesh->SetVisibility(false);
		WindowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// 2. PREPARAZIONE CHAOS
		ShatteredMesh->SetVisibility(true);
		UE_LOG(LogTemp, Warning, TEXT("--- SHATTER MESH VISIBLE ---"));

		// FIX: Forziamo il Tick del componente anche se l'attore non ticka
		ShatteredMesh->SetComponentTickEnabled(true);

		ShatteredMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ShatteredMesh->SetCollisionProfileName(TEXT("Destructible"));

		// 3. ATTIVAZIONE FISICA
		ShatteredMesh->SetSimulatePhysics(true);

		// FIX: Forziamo l'aggiornamento delle proprietà fisiche
		ShatteredMesh->RecreatePhysicsState();
		ShatteredMesh->WakeAllRigidBodies();

		// 4. APPLICAZIONE FORZA
		FVector ImpulseDir = HitResult.ImpactNormal * -1.0f;

		// Impulso Strain (Rottura)
		ShatteredMesh->AddRadialImpulse(HitResult.ImpactPoint, 100.0f, 2000000.0f, ERadialImpulseFalloff::RIF_Constant, true);

		// Impulso Spinta
		ShatteredMesh->AddRadialImpulse(HitResult.ImpactPoint, 500.0f, ImpactForce * 2.0f, ERadialImpulseFalloff::RIF_Linear, true);

		BreakWindow(HitResult.ImpactPoint);
	}
	void ABreakableWindow::OnUnsuccesfulImpact_Implementation(float ImpactForce, const FHitResult& HitResult, AActor* InstigatorActor)
	{
		// Solo effetti sonori/visivi leggeri
		TouchWindow(HitResult.ImpactPoint);
		UE_LOG(LogTemp, Warning, TEXT("Window hit but not shattered. Impact Force: %f"), ImpactForce);
	}