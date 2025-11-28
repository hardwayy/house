// Copyright Epic Games, Inc. All Rights Reserved.

#include "houseCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InteractionInterface.h" 
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "GrabbableActor.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AhouseCharacter::AhouseCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false; 
	bUseControllerRotationYaw = true;    
	bUseControllerRotationRoll = false;

	
	GetCharacterMovement()->bOrientRotationToMovement = false; 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); 

	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;


	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	FollowCamera->SetupAttachment(GetMesh(), TEXT("head"));

	FollowCamera->bUsePawnControlRotation = true;

	FollowCamera->SetRelativeLocation(FVector(10.f, 0.f, 0.f));

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
	PhysicsHandle->LinearDamping = 200.f;

	PhysicsHandle->LinearStiffness = 15000.f;

	PhysicsHandle->InterpolationSpeed = 50.f;

	PhysicsHandle->AngularDamping = 100.f;
	PhysicsHandle->AngularStiffness = 15000.f;
}
void AhouseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		// 3. Ottieni il Subsystem dell'Input (il gestore delle periferiche)
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// 4. Aggiungi il Mapping Context (la "mappa dei tasti")
			if (DefaultMappingContext)
			{
				// Priorità 0 è lo standard.
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
				UE_LOG(LogTemplateCharacter, Log, TEXT("Mapping Context Added Successfully!"));
			}
			else
			{
				UE_LOG(LogTemplateCharacter, Error, TEXT("ATTENZIONE: DefaultMappingContext non assegnato nel Blueprint!"));
			}
		}
	}
}

void AhouseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Controllo rapido: Se non abbiamo un PhysicsHandle o non stiamo tenendo nulla, usciamo subito.
	if (!PhysicsHandle || !PhysicsHandle->GetGrabbedComponent())
	{
		CurrentPullBack = 0.f;
		CurrentHeldObject = nullptr; // Reset sicurezza
		return;
	}

	// 1. Calcolo Posizione Target
	const FVector CamLoc = FollowCamera->GetComponentLocation();
	const FRotator CamRot = FollowCamera->GetComponentRotation();
	const FVector ForwardDir = CamRot.Vector();

	// Costruiamo la posizione target usando gli assi della camera
	FVector BaseLoc = CamLoc
		+ (ForwardDir * HoldOffset.X)
		+ (FRotationMatrix(CamRot).GetScaledAxis(EAxis::Y) * HoldOffset.Y)
		+ (FRotationMatrix(CamRot).GetScaledAxis(EAxis::Z) * HoldOffset.Z);

	// Applichiamo il "PullBack" (effetto molla quando tocchi i muri)
	FVector FinalLoc = BaseLoc - (ForwardDir * CurrentPullBack);

	// 2. Calcolo Rotazione Target (FIXED ROTATION)
	FRotator FinalRot = CamRot;

	// Usiamo il puntatore cachato invece di fare Cast ogni frame
	if (CurrentHeldObject)
	{
		// MATEMATICA QUATERNIONI:
		// Convertiamo tutto in Quaternioni per evitare Gimbal Lock.
		// Ordine: RotazioneCamera * OffsetOggetto.
		// Questo applica l'offset dell'oggetto RELATIVAMENTE alla vista della camera.
		FQuat CameraQuat = CamRot.Quaternion();
		FQuat OffsetQuat = CurrentHeldObject->HoldRotationOffset.Quaternion();

		FinalRot = (CameraQuat * OffsetQuat).Rotator();
	}

	// 3. Applichiamo al Physics Handle
	PhysicsHandle->SetTargetLocationAndRotation(FinalLoc, FinalRot);
}

void AhouseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AhouseCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AhouseCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AhouseCharacter::Look);

		//Interacting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AhouseCharacter::TryInteract);

		// Start grab
		EnhancedInputComponent->BindAction(GrabAction, ETriggerEvent::Started, this, &AhouseCharacter::OnGrabInput);
		// End grab/throw
		EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Started, this, &AhouseCharacter::OnThrowInputStarted);
		EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Completed, this, &AhouseCharacter::OnThrowInputCompleted);
		// Select Inv. Slot
		EnhancedInputComponent->BindAction(Slot1Action, ETriggerEvent::Started, this, &AhouseCharacter::OnSelectSlot1);
		EnhancedInputComponent->BindAction(Slot2Action, ETriggerEvent::Started, this, &AhouseCharacter::OnSelectSlot2);
		EnhancedInputComponent->BindAction(Slot3Action, ETriggerEvent::Started, this, &AhouseCharacter::OnSelectSlot3);
		EnhancedInputComponent->BindAction(Slot4Action, ETriggerEvent::Started, this, &AhouseCharacter::OnSelectSlot4);
		EnhancedInputComponent->BindAction(UseItemAction, ETriggerEvent::Started, this, &AhouseCharacter::OnUseItemInput);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AhouseCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AhouseCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AhouseCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AhouseCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AhouseCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AhouseCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}
void AhouseCharacter::TryInteract()
{
	// Safety check: ensure the camera exists
	if (!FollowCamera) return;

	FHitResult HitResult;
	FVector Start = FollowCamera->GetComponentLocation();
	FVector End = Start + (FollowCamera->GetForwardVector() * 300.f);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
	{
		Params.AddIgnoredActor(PhysicsHandle->GetGrabbedComponent()->GetOwner());
	}

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	// DEBUG: Draw a red line to visualize the trace (remove this later)
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();

		if (HitActor)
		{
			// Check if the actor implements the interface using the U-Class version
			if (HitActor->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
			{
				// Execute the interaction
				IInteractionInterface::Execute_Interact(HitActor, this);
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Interacted!"));
				UE_LOG(LogTemplateCharacter, Log, TEXT("Interaction successful with: %s"), *HitActor->GetName());
			}
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("Interact trace hit nothing."));
	}
}

void AhouseCharacter::OnGrabInput()
{
	if (!FollowCamera) return;

	FHitResult HitResult;
	FVector Start = FollowCamera->GetComponentLocation();
	FVector End = Start + (FollowCamera->GetForwardVector() * 250.f);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	// Ignora l'oggetto attualmente tenuto (se esiste)
	AActor* OldActor = nullptr;
	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
	{
		OldActor = PhysicsHandle->GetGrabbedComponent()->GetOwner();
		Params.AddIgnoredActor(OldActor);
	}

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		// 1. Validazione Actor Tipizzato
		AGrabbableActor* HitGrabbable = Cast<AGrabbableActor>(HitResult.GetActor());
		if (!HitGrabbable) return;

		// 2. Otteniamo la Mesh (che è figlia della Root) usando il nuovo getter
		UPrimitiveComponent* HitComponent = HitGrabbable->GetMesh();

		// Verifica che la mesh esista e che sia quella colpita (o parte dell'actor colpito)
		if (HitComponent && (HitResult.GetComponent() == HitComponent))
		{
			// Se la mesh non sta simulando, attiviamola ora per permettere il grab fisico
			if (!HitComponent->IsSimulatingPhysics())
			{
				HitComponent->SetSimulatePhysics(true);
			}

			// --- SWAP LOGIC ---
			if (OldActor)
			{
				PhysicsHandle->ReleaseComponent();
				AddToInventory(OldActor); // Assumiamo che AddToInventory gestisca lo spegnimento della fisica
			}

			// CACHE POINTER
			CurrentHeldObject = HitGrabbable;
			bIsGrabbingObject = true;

			// --- CALCOLO ROTAZIONE ---
			FQuat CameraQuat = FollowCamera->GetComponentRotation().Quaternion();
			// Usiamo l'offset definito nell'Actor
			FQuat OffsetQuat = HitGrabbable->HoldRotationOffset.Quaternion();
			FRotator TargetRotation = (CameraQuat * OffsetQuat).Rotator();

			// Ruotiamo la MESH, non l'Actor (perché è la mesh che simula la fisica)
			HitComponent->SetWorldRotation(TargetRotation, false, nullptr, ETeleportType::TeleportPhysics);

			// --- GRAB ---
			PhysicsHandle->GrabComponentAtLocationWithRotation(
				HitComponent,
				NAME_None,
				HitComponent->GetComponentLocation(), // Prendi la mesh dove si trova
				TargetRotation
			);

			// --- SETUP COLLISIONI ---
			SetItemCollision(HitComponent, true);
			HitComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

			// Aggiunta inventario
			if (!Inventory.Contains(HitGrabbable))
			{
				Inventory.AddUnique(HitGrabbable);
				if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast();
			}

			ThrowButtonPressTime = GetWorld()->GetTimeSeconds();
			UE_LOG(LogTemplateCharacter, Log, TEXT("Grabbed object mesh: %s"), *HitComponent->GetName());
		}
	}
}
void AhouseCharacter::OnThrowInputStarted()
{
	if (!PhysicsHandle || !PhysicsHandle->GetGrabbedComponent()) return;

	//if (bIsGrabbingObject) return;

	ThrowButtonPressTime = GetWorld()->GetTimeSeconds();
	BP_StartThrowCharge();
	UE_LOG(LogTemplateCharacter, Warning, TEXT("Event Sent to BP: Start Charge"));
}
void AhouseCharacter::OnThrowInputCompleted()
{
	
	if (!PhysicsHandle || !PhysicsHandle->GetGrabbedComponent()) return;


	UPrimitiveComponent* HeldComponent = PhysicsHandle->GetGrabbedComponent();
	AActor* HeldActor = HeldComponent->GetOwner();
	int32 OldIndex = -1;
	if (HeldActor)
	{
		OldIndex = Inventory.Find(HeldActor);
	}
	if (HeldActor && Inventory.Contains(HeldActor))
	{
		Inventory.Remove(HeldActor);
		UE_LOG(LogTemplateCharacter, Log, TEXT("Removed from Inventory: %s"), *HeldActor->GetName());
		if (OnInventoryUpdated.IsBound())
		{
			OnInventoryUpdated.Broadcast();
		}
	}

	
	double HoldDuration = GetWorld()->GetTimeSeconds() - ThrowButtonPressTime;

	
	PhysicsHandle->ReleaseComponent(); 

	if (HeldComponent)
	{
		SetItemCollision(HeldComponent, false);
		HeldComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);


		HeldComponent->SetSimulatePhysics(false);
		HeldComponent->SetSimulatePhysics(true);
		HeldComponent->WakeAllRigidBodies();

		if (HoldDuration < DropThreshold)
		{
			UE_LOG(LogTemplateCharacter, Log, TEXT("Short Press: Dropped (Mass Logic Active)"));
		}
		else
		{
			
			float ChargeTime = HoldDuration - DropThreshold;
			float Alpha = FMath::Clamp(ChargeTime / MaxChargeTime, 0.f, 1.f);

			float RawImpulse = FMath::Lerp(MinThrowForce, MaxThrowForce, Alpha);

			float ObjectMass = HeldComponent->GetMass();

			float MaxAllowedImpulse = ObjectMass * MaxLaunchVelocity;

			float FinalImpulse = FMath::Min(RawImpulse, MaxAllowedImpulse);

			HeldComponent->AddImpulse(FollowCamera->GetForwardVector() * FinalImpulse, NAME_None, false);

			UE_LOG(LogTemplateCharacter, Log, TEXT("Throw - Mass: %fkg | Raw Impulse: %f | Final Impulse: %f"), ObjectMass, RawImpulse, FinalImpulse);
		}
	}
	if (Inventory.Num() > 0 && OldIndex != -1) {
		EquipItemAtIndex(FMath::Clamp(OldIndex, 0, Inventory.Num() - 1));
	}

	BP_StopThrowCharge();
}
void AhouseCharacter::AddToInventory(AActor* NewItem)
{
	if (!NewItem) return;

	if (!Inventory.Contains(NewItem))
	{
		if (Inventory.Num() >= MaxInventorySlots)
		{
			UE_LOG(LogTemplateCharacter, Warning, TEXT("Inventory FULL! Cannot pick up %s"), *NewItem->GetName());
			return;
		}

		Inventory.Add(NewItem);
	}



	NewItem->SetActorHiddenInGame(true);
	NewItem->SetActorEnableCollision(false);

	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(NewItem->GetRootComponent()))
	{
		RootPrim->SetSimulatePhysics(false);
	}

	UE_LOG(LogTemplateCharacter, Log, TEXT("Item Secured in Pocket: %s. Slots used: %d/%d"), *NewItem->GetName(), Inventory.Num(), MaxInventorySlots);

	if (OnInventoryUpdated.IsBound())
	{
		OnInventoryUpdated.Broadcast();
	}
}
void AhouseCharacter::EquipItemAtIndex(int32 Index)
{
	// 1. VALIDAZIONE INDICE
	if (!Inventory.IsValidIndex(Index))
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("Slot %d is empty or invalid!"), Index + 1);
		return;
	}

	AGrabbableActor* NewItem = Cast<AGrabbableActor>(Inventory[Index]);
	if (!NewItem) return;

	// OTTIMIZZAZIONE: Se stiamo già tenendo QUESTO oggetto (controllando il proprietario della mesh presa)
	if (PhysicsHandle->GetGrabbedComponent() && PhysicsHandle->GetGrabbedComponent()->GetOwner() == NewItem)
	{
		return;
	}

	// 2. METTI VIA L'OGGETTO CORRENTE
	if (PhysicsHandle->GetGrabbedComponent())
	{
		UPrimitiveComponent* OldComp = PhysicsHandle->GetGrabbedComponent();
		AActor* OldActor = OldComp->GetOwner();

		PhysicsHandle->ReleaseComponent();

		// Logica "Metti in tasca"
		OldActor->SetActorHiddenInGame(true);
		OldActor->SetActorEnableCollision(false);

		// Spegni la fisica sulla mesh
		OldComp->SetSimulatePhysics(false);

		// RESETTA LA MESH ALLA POSIZIONE DELLA ROOT
		// Questo è vitale: quando rimetti via l'oggetto, riallinea la mesh alla SceneRoot
		// così la prossima volta che sposti l'Actor, la mesh è nel posto giusto.
		OldComp->AttachToComponent(OldActor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

		SetItemCollision(OldComp, false);
	}

	// 3. ESTRAI IL NUOVO OGGETTO
	NewItem->SetActorHiddenInGame(false);
	NewItem->SetActorEnableCollision(true);

	// Otteniamo la Mesh (NON la RootComponent, che è la SceneRoot)
	UStaticMeshComponent* NewMesh = NewItem->GetMesh();

	if (NewMesh)
	{
		// A. Posizioniamo l'ACTOR (SceneRoot) davanti alla camera
		FVector StartLoc = FollowCamera->GetComponentLocation() + (FollowCamera->GetForwardVector() * HoldOffset.X);

		FQuat CameraQuat = FollowCamera->GetComponentRotation().Quaternion();
		FQuat OffsetQuat = NewItem->HoldRotationOffset.Quaternion();
		FRotator TargetRotation = (CameraQuat * OffsetQuat).Rotator();

		// Spostiamo l'Actor intero (la mesh seguirà perché è attaccata e non simula ancora)
		NewItem->SetActorLocationAndRotation(StartLoc, TargetRotation, false, nullptr, ETeleportType::TeleportPhysics);

		// B. Assicuriamoci che la Mesh sia resettata localmente (0,0,0 rispetto alla root)
		NewMesh->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);

		// C. Attiva fisica sulla MESH
		NewMesh->SetSimulatePhysics(true);
		SetItemCollision(NewMesh, true);
		NewMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

		// D. GRAB sulla MESH
		PhysicsHandle->GrabComponentAtLocationWithRotation(
			NewMesh,
			NAME_None,
			NewMesh->GetComponentLocation(), // Ora coincide con la root perché l'abbiamo resettata
			TargetRotation
		);

		// Reset variabili stato
		CurrentHeldObject = NewItem; // Aggiorna il puntatore all'oggetto corrente
		bIsGrabbingObject = true;
		ThrowButtonPressTime = GetWorld()->GetTimeSeconds();

		UE_LOG(LogTemplateCharacter, Log, TEXT("Equipped Item Mesh from Slot %d: %s"), Index + 1, *NewItem->GetName());
	}
}
void AhouseCharacter::SetItemCollision(UPrimitiveComponent* Item, bool bIsHeld)
{
	if (!Item) return;

	if (bIsHeld)
	{

		Item->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
		Item->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
		Item->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		Item->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);

		
	}
	else
	{
		
		Item->SetCollisionProfileName(TEXT("PhysicsActor"));
	}
}
void AhouseCharacter::OnUseItemInput()
{
	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
	{
		AActor* HeldActor = PhysicsHandle->GetGrabbedComponent()->GetOwner();

		// 2. Controlla se è un GrabbableActor (e non un muro o altro per errore)
		AGrabbableActor* GrabbableItem = Cast<AGrabbableActor>(HeldActor);

		if (GrabbableItem)
		{

			GrabbableItem->OnUse(GrabbableItem);

			UE_LOG(LogTemplateCharacter, Log, TEXT("Used Item: %s"), *GrabbableItem->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("Cannot use item: Hand is empty."));
	}
}