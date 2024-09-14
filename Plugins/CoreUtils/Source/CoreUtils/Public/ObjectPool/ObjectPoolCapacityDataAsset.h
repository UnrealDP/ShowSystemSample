// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ObjectPoolType.h"
#include "ObjectPoolCapacityDataAsset.generated.h"


USTRUCT(BlueprintType)
struct FObjectPoolTypeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoolSettings")
    EObjectPoolType PoolType = EObjectPoolType::Max;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoolSettings")
    TSubclassOf<UObject> ObjectClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoolSettings", Meta = (ClampMin = "1", Tooltip = "Initial Capacity must be greater than or equal to Reserved Actor Count."))
    int32 InitialCapacity = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoolSettings", Meta = (ClampMin = "1", Tooltip = "Reserved Object Count must be less than or equal to Initial Capacity."))
    int32 ReservedObjectCount = 5;
};


/**
 * 
 */
UCLASS()
class COREUTILS_API UObjectPoolCapacityDataAsset : public UDataAsset
{
	GENERATED_BODY()

private:
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PoolSettings")
    TArray<FObjectPoolTypeSettings> PoolSettings;

};
