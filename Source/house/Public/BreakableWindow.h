#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "ImpactInterface.h" // Assumo tu abbia questa interfaccia
#include "BreakableWindow.generated.h"

UCLASS()
class HOUSE_API ABreakableWindow : public AActor, public IImpactInterface
{
	GENERATED_BODY()
	
public:	
	ABreakableWindow();

protected:
	virtual void BeginPlay() override;

public:	
	// Componente principale: La Geometry Collection
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UGeometryCollectionComponent* ShatteredMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootScene;

	// Soglia di forza per rompere il vetro
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Break Settings")
	float BreakThreshold;

	// Funzione per gestire l'impatto fisico
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// Implementazione Interfaccia
	virtual void OnImpact_Implementation(float ImpactForce, const FHitResult& HitResult, AActor* InstigatorActor) override;
	virtual void OnUnsuccesfulImpact_Implementation(float ImpactForce, const FHitResult& HitResult, AActor* InstigatorActor) override;
};