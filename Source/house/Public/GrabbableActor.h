// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractionInterface.h"
#include "GrabbableActor.generated.h"

UCLASS()
class HOUSE_API AGrabbableActor : public AActor, public IInteractionInterface
{
	GENERATED_BODY()

public:
	AGrabbableActor();

protected:
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	UPROPERTY(EditAnywhere, Category = "AI Hearing")
	float ImpactLoudness = 1.0f; // 1.0 = Standard. Aumenta per oggetti pesanti.

	UPROPERTY(EditAnywhere, Category = "AI Hearing")
	float MinImpulseForNoise = 1000.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* BaseMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	class UTexture2D* ItemIcon;

	UPROPERTY(EditAnywhere, Category = "AI Hearing")
	float NoiseCooldown = 1.0f; // Tempo minimo in secondi tra due rumori

private:
	float LastNoiseTime = 0.0f;

public:
	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FRotator HoldRotationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FName ItemName;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnUse(AActor* User);

	UFUNCTION(BlueprintCallable, Category = "Grabbable")
	UStaticMeshComponent* GetMesh() const { return BaseMesh; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
	float CachedMass = 0.0f;

};
