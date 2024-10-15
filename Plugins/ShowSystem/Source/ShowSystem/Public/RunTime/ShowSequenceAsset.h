// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InstancedStruct.h"
#include "ShowSequenceAsset.generated.h"

/**
 * 
 */
UCLASS()
class SHOWSYSTEM_API UShowSequenceAsset : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Show")
	TArray<FInstancedStruct> ShowKeys;
};
