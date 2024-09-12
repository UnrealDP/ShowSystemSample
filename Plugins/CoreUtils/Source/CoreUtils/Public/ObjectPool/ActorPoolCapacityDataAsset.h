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
    EActorPoolType PoolType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoolSettings")
    int32 InitialCapacity;

    FPoolTypeSettings()
        : PoolType(EActorPoolType::Max), InitialCapacity(10) {}
};

/**
 * 
 */
UCLASS()
class COREUTILS_API UActorPoolCapacityDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoolSettings")
    TArray<FPoolTypeSettings> PoolSettings;
	
};
