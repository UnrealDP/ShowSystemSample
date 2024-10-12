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
    TSoftObjectPtr<UAnimSequenceBase> AnimSequenceAsset;
};

/**
 * 
 */
UCLASS()
class SHOWSYSTEM_API UShowAnimStatic : public UShowBase
{
	GENERATED_BODY()
	
protected:
    virtual void Initialize() override;
    virtual float InitializeAssetLength() override;
    virtual void Dispose() override;
    virtual void Play() override;
    virtual void Stop() override {};
    virtual void Reset();
    virtual void Pause() override {};
    virtual void UnPause() override {};

private:
    virtual void Tick(float DeltaTime, float BasePassedTime) override
    {
    }

private:
    const FShowAnimStaticKey* AnimStaticKeyPtr;

    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComp;
    TObjectPtr<UAnimSequenceBase> AnimSequenceBase;
};
