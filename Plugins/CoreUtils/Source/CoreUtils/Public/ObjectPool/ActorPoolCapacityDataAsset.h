// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ActorPoolType.h"
#include "ActorPoolCapacityDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FPoolTypeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoolSettings")
    EActorPoolType PoolType = EActorPoolType::Max;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoolSettings")
    TSubclassOf<AActor> ActorClass;    

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoolSettings", Meta = (ClampMin = "1", Tooltip = "Initial Capacity must be greater than or equal to Reserved Actor Count."))
    int32 InitialCapacity = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoolSettings", Meta = (ClampMin = "1", Tooltip = "Reserved Actor Count must be less than or equal to Initial Capacity."))
    int32 ReservedActorCount = 5;
};

/**
 * 
 */
UCLASS()
class COREUTILS_API UActorPoolCapacityDataAsset : public UDataAsset
{
	GENERATED_BODY()

private:
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoolSettings")
    TArray<FPoolTypeSettings> PoolSettings;
	
};
