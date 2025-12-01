#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "ImpactInterface.h"
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
	// Componente Radice per tenere tutto ordinato
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootScene;

	// 1. LA MESH SANA (Quella che vediamo all'inizio)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* WindowMesh;

	// 2. LA MESH ROTTA (Nascosta e disabilitata all'inizio)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UGeometryCollectionComponent* ShatteredMesh;

	// Soglia di forza
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Break Settings")
	float BreakThreshold;

	// Funzione per gestire l'impatto fisico (Hit Event sulla Static Mesh)
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// Eventi Blueprint per audio/fx
	UFUNCTION(BlueprintImplementableEvent, Category = "Impact")
	void BreakWindow(const FVector& HitLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Impact")
	void TouchWindow(const FVector& HitLocation);

	// Implementazione Interfaccia
	virtual void OnImpact_Implementation(float ImpactForce, const FHitResult& HitResult, AActor* InstigatorActor) override;
	virtual void OnUnsuccesfulImpact_Implementation(float ImpactForce, const FHitResult& HitResult, AActor* InstigatorActor) override;
};