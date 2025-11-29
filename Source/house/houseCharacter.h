// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "houseCharacter.generated.h"



class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChanged, float, StaminaPercent);
DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
UCLASS(abstract)
class AhouseCharacter : public ACharacter
{
	GENERATED_BODY()


	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/*Physics Handle*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	class UPhysicsHandleComponent* PhysicsHandle;

	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* MouseLookAction;

	/* Interact Input Action*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* InteractAction;

	/* Grab Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* GrabAction;

	/* Throw Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* ThrowAction;

	/*Sprint Input Action*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Throwing")
    float CurrentPullBack;

	UFUNCTION(BlueprintImplementableEvent)
	void BP_StartThrowCharge();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_StopThrowCharge();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Throwing")
	float MinThrowForce = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Throwing")
	float MaxThrowForce = 2500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Throwing")
	float MaxLaunchVelocity = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Throwing")
	float MaxChargeTime = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Throwing")
	float DropThreshold = 0.2f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<AActor*> Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* Slot1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* Slot2Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* Slot3Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* Slot4Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* UseItemAction;

	// Funzione chiamata dal tasto
	void OnUseItemInput();

	void OnSelectSlot1() { EquipItemAtIndex(0); }
	void OnSelectSlot2() { EquipItemAtIndex(1); }
	void OnSelectSlot3() { EquipItemAtIndex(2); }
	void OnSelectSlot4() { EquipItemAtIndex(3); }

	// La logica vera e propria
	void EquipItemAtIndex(int32 Index);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina")
	float MaxStamina = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina")
	float StaminaDrainRate = 10.f; // Stamina persa al secondo

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina")
	float StaminaRegenRate = 5.f; // Stamina guadagnata al secondo

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina")
	float SprintSpeed = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina")
	float WalkSpeed = 500.f; // Deve corrispondere al default nel costruttore

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina")
	float StaminaRegenDelay = 1.0f; // Tempo di attesa prima che la stamina inizi a rigenerarsi

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Stamina")
	float CurrentStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina|Load")
	float StaminaDrainPerKg = 2.0f;

	// Peso massimo oltre il quale non è possibile scattare (Opzionale, ma consigliato per evitare bug visivi)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Stamina|Load")
	float MaxSprintWeight = 50.0f;

	// Helper per calcolare la massa totale (Inventario + Oggetto in mano)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	float CalculateTotalMass() const;


	bool bIsSprinting;

	FTimerHandle StaminaTimerHandle;
	void HandleStaminaUpdate();
	void StartSprint();
	void StopSprint();

public:

	/** Constructor */
	AhouseCharacter();	
	// Override Tick function (needed to update the physics handle position)
	virtual void Tick(float DeltaTime) override;

	// Evento a cui il Widget si iscriverà per sapere quando aggiornare la barra
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStaminaChanged OnStaminaChanged;

	// Helper per ottenere la percentuale attuale (utile per l'inizializzazione del Widget)
	UFUNCTION(BlueprintPure, Category = "Movement|Stamina")
	float GetStaminaPercent() const { return (MaxStamina > 0.f) ? CurrentStamina / MaxStamina : 0.f; }

private:
	UPROPERTY()
	class AGrabbableActor* CurrentHeldObject;

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/*Called for interacting with objects*/
	void TryInteract();

	/*Called for grabbing objects*/
	void TryGrab();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FVector HoldOffset = FVector(50.f, 40.f, -20.f);

	float ThrowButtonPressTime;
	bool bIsGrabbingObject;

	void OnGrabInput();
	
	void OnThrowInputStarted();   
	void OnThrowInputCompleted(); 
	void SetItemCollision(UPrimitiveComponent* Item, bool bIsHeld);

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	void AddToInventory(AActor* NewItem);

protected:
	virtual void BeginPlay() override;


protected:
	// Configurazione Slot
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MaxInventorySlots = 4;



public:
	// L'evento a cui il Widget si iscriverà
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryUpdated OnInventoryUpdated;

	// Getter per leggere l'inventario dal Widget Blueprint
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<AActor*> GetInventory() const { return Inventory; }

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;
public:

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

