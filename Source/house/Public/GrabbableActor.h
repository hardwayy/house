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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* BaseMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	class UTexture2D* ItemIcon;



public:
	virtual void Interact_Implementation(AActor* InstigatorActor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FRotator HoldRotationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FName ItemName;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnUse(AActor* User);

};
