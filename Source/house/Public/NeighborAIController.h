#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h" // FIX 1: Necessario per FAIStimulus
#include "NeighborTypes.h"
#include "NeighborAIController.generated.h"

// FIX 2: Forward Declarations. Diciamo al compilatore "Queste classi esistono, fidati".
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UBehaviorTree;

UCLASS()
class HOUSE_API ANeighborAIController : public AAIController
{
	GENERATED_BODY()

public:
	ANeighborAIController();

protected:
	virtual void BeginPlay() override;

	// Se la dichiariamo qui, dobbiamo implementarla nel .cpp!
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

public: // Ho spostato questo in public o protected solitamente, ma va bene qui
	// Behavior Tree da assegnare nell'Editor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	UBehaviorTree* BehaviorTreeAsset;

private:
	// Il "Cervello" sensoriale
	UPROPERTY(VisibleAnywhere, Category = "AI")
	UAIPerceptionComponent* AIPerceptionComp;

	// La configurazione specifica per la VISTA
	UAISenseConfig_Sight* SightConfig;
};