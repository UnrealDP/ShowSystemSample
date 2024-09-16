// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RunTime/ShowBase.h"
#include "InstancedStruct.h"
#include "ShowSequencer.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class SHOWSYSTEM_API UShowSequencer : public UObject
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Show")
    TArray<FInstancedStruct> ShowKeys;

    UFUNCTION(BlueprintCallable, Category = "Show")
    void InitShowKeys();

    UFUNCTION(BlueprintCallable, Category = "Show")
    void Play();

    UFUNCTION(BlueprintCallable, Category = "Show")
    void Stop();

    UFUNCTION(BlueprintCallable, Category = "Show")
    void Pause();

    UFUNCTION(BlueprintCallable, Category = "Show")
    void UnPause();

    UFUNCTION(BlueprintCallable, Category = "Show")
    void ChangeSpeed(float Speed);

private:
    TArray<TObjectPtr<UShowBase>> RuntimeShowKeys;
};
