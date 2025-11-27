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
DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AhouseCharacter::AhouseCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// --- FIRST PERSON ROTATION SETTINGS ---
	// Why: In FPS, when you move the mouse right, the whole body should turn right.
	// Unlike Third Person, we don't want independent camera rotation.
	bUseControllerRotationPitch = false; // Usually false, we don't want the capsule to tip over looking up
	bUseControllerRotationYaw = true;    // TRUE: Character rotates with mouse input
	bUseControllerRotationRoll = false;

	// Configure character movement
	// Why: In FPS, we want to strafe. We don't want the character to turn to face the movement direction automatically.
	GetCharacterMovement()->bOrientRotationToMovement = false; // FALSE: Standard FPS movement
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // Still useful for smooth turns if needed

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// --- CAMERA SETUP ---
	// No CameraBoom (SpringArm) needed for True First Person.

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	// Attach directly to the Mesh on the "head" socket
	// CRITICAL: Ensure your Skeleton actually has a socket named "head" (case sensitive!)
	FollowCamera->SetupAttachment(GetMesh(), TEXT("head"));

	// Enable PawnControlRotation so the camera follows the mouse/controller input
	FollowCamera->bUsePawnControlRotation = true;

	// Adjust camera position slightly to avoid clipping through the face geometry (optional but recommended)
	FollowCamera->SetRelativeLocation(FVector(10.f, 0.f, 0.f));

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
}
void AhouseCharacter::BeginPlay()
{
	// 1. Chiama la versione base della funzione (obbligatorio!)
	Super::BeginPlay();

	// 2. Ottieni il Player Controller (il "pilota")
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

	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
	{

		FVector CamLoc = FollowCamera->GetComponentLocation();
		FRotator CamRot = FollowCamera->GetComponentRotation();

		FVector BaseLoc = CamLoc
			+ (CamRot.Vector() * HoldOffset.X)
			+ (FRotationMatrix(CamRot).GetScaledAxis(EAxis::Y) * HoldOffset.Y)
			+ (FRotationMatrix(CamRot).GetScaledAxis(EAxis::Z) * HoldOffset.Z);

		FVector FinalLoc = BaseLoc - (CamRot.Vector() * CurrentPullBack);

		PhysicsHandle->SetTargetLocationAndRotation(FinalLoc, CamRot);
	}
	else
	{
		CurrentPullBack = 0.f;
	}
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

	AActor* CurrentHeldActor = nullptr;
	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
	{
		CurrentHeldActor = PhysicsHandle->GetGrabbedComponent()->GetOwner();
		Params.AddIgnoredActor(CurrentHeldActor);
	}

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		UPrimitiveComponent* HitComponent = HitResult.GetComponent();

		if (HitComponent && HitComponent->IsSimulatingPhysics())
		{
			AGrabbableActor* Grabbable = Cast<AGrabbableActor>(HitResult.GetActor());

			if(!Grabbable)
			{
				UE_LOG(LogTemplateCharacter, Warning, TEXT("Hit object is not Grabbable: %s"), *HitResult.GetActor()->GetName());
				return;
			}
			if (CurrentHeldActor)
			{
				PhysicsHandle->ReleaseComponent();

				AddToInventory(CurrentHeldActor);

				UE_LOG(LogTemplateCharacter, Log, TEXT("Swapped items: Put away %s"), *CurrentHeldActor->GetName());
			}

			PhysicsHandle->GrabComponentAtLocationWithRotation(
				HitComponent,
				NAME_None,
				HitResult.Location,
				HitComponent->GetComponentRotation()
			);

			HitComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

			AActor* NewActor = HitResult.GetActor();
			if (NewActor)
			{
				Inventory.AddUnique(NewActor);

				if (OnInventoryUpdated.IsBound()) OnInventoryUpdated.Broadcast();
			}

			
			UE_LOG(LogTemplateCharacter, Log, TEXT("Grabbed new object: %s"), *NewActor->GetName());
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

	if (!Inventory.IsValidIndex(Index))
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("Slot %d is empty!"), Index + 1);
		return;
	}

	AActor* NewItem = Inventory[Index];
	if (!NewItem) return;

	if (PhysicsHandle->GetGrabbedComponent() && PhysicsHandle->GetGrabbedComponent()->GetOwner() == NewItem)
	{
		return; 
	}

	
	if (PhysicsHandle->GetGrabbedComponent())
	{
		UPrimitiveComponent* OldComp = PhysicsHandle->GetGrabbedComponent();
		AActor* OldActor = OldComp->GetOwner();

		PhysicsHandle->ReleaseComponent();

		OldActor->SetActorHiddenInGame(true);
		OldActor->SetActorEnableCollision(false);
		OldComp->SetSimulatePhysics(false);
	}


	NewItem->SetActorHiddenInGame(false);
	NewItem->SetActorEnableCollision(true);

	UPrimitiveComponent* NewComp = Cast<UPrimitiveComponent>(NewItem->GetRootComponent());
	if (NewComp)
	{
		
		FVector StartLoc = FollowCamera->GetComponentLocation() + (FollowCamera->GetForwardVector() * 100.f);
		NewItem->SetActorLocation(StartLoc);

		NewComp->SetSimulatePhysics(true);
		NewComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

		PhysicsHandle->GrabComponentAtLocationWithRotation(
			NewComp,
			NAME_None,
			StartLoc,
			NewItem->GetActorRotation()
		);

		bIsGrabbingObject = true;
		ThrowButtonPressTime = GetWorld()->GetTimeSeconds();

		UE_LOG(LogTemplateCharacter, Log, TEXT("Equipped Item from Slot %d: %s"), Index + 1, *NewItem->GetName());
	}
}