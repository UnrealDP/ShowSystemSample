// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ShowBase.generated.h"


USTRUCT(Atomic, BlueprintType)
struct FShowKey
{
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowKey")
    float StartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowKey")
    float Duration = 0.0f;
};

/**
 * 
 */
UCLASS()
class SHOWSYSTEM_API UShowBase : public UObject
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Show")
    FShowKey ShowKey;

    UFUNCTION(BlueprintCallable, Category = "Show")
    void Play();

    UFUNCTION(BlueprintCallable, Category = "Show")
    void Stop();

    UFUNCTION(BlueprintCallable, Category = "Show")
    void Pause();

    UFUNCTION(BlueprintCallable, Category = "Show")
    void UnPause();

private:
    float CurrentTime;
};
