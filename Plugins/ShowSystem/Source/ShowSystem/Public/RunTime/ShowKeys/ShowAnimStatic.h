// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTime/ShowBase.h"
#include "ShowAnimStatic.generated.h"

USTRUCT(BlueprintType)
struct FShowAnimStaticKey : public FShowKey
{
    GENERATED_BODY()

    FShowAnimStaticKey()
    {
        KeyType = EShowKeyType::ShowKey_Anim;
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Show")
    UAnimSequence* AnimSequence;
};

/**
 * 
 */
UCLASS()
class SHOWSYSTEM_API UShowAnimStatic : public UShowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Show")
	TSubclassOf<UAnimSequence> AnimSequenceClass;
};
