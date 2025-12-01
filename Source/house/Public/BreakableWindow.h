#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ImpactInterface.h" 
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "BreakableWindow.generated.h"

UCLASS()
class HOUSE_API ABreakableWindow : public AActor, public IImpactInterface
{
	GENERATED_BODY()

public:
	ABreakableWindow();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* RootScene;

	// La mesh integra (economica da renderizzare)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* WindowMesh;

	// La mesh fratturata (Chaos Geometry Collection)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UGeometryCollectionComponent* ShatteredMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact Settings")
	float BreakThreshold;

	virtual void BeginPlay() override;

public:
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void OnImpact_Implementation(float ImpactForce, const FHitResult& HitResult, AActor* InstigatorActor) override;

	virtual void OnUnsuccesfulImpact_Implementation(float ImpactForce, const FHitResult& HitResult, AActor* InstigatorActor) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Impact")
	void BreakWindow(const FVector& HitLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Impact")
	void TouchWindow(const FVector& HitLocation);
};