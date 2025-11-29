#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h" // Serve per le macro UENUM
#include "NeighborTypes.generated.h" // <--- MAGIA: Questo collega il C++ all'Editor

// Definiamo l'Enum qui, visibile a tutto il progetto
UENUM(BlueprintType)
enum class ENeighborState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Pattuglia"),
    Investigate UMETA(DisplayName = "Indaga"),
    Chase       UMETA(DisplayName = "INSEGUIMENTO"),
    Catch       UMETA(DisplayName = "Cattura")
};

// Creiamo una classe dummy per forzare Unreal a generare il file .generated.h
// Senza questo, UHT potrebbe ignorare il file se contiene solo un Enum.
UCLASS()
class HOUSE_API UNeighborTypes : public UObject
{
    GENERATED_BODY()
};