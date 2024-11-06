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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimStaticKey")
    TSoftObjectPtr<UAnimSequenceBase> AnimSequenceAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimStaticKey")
    float BlendOutTriggerTime = -1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimStaticKey")
    float InTimeToStartMontageAt = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimStaticKey")
    int32 LoopCount = 1;
};

/**
 * 
 */
UCLASS()
class SHOWSYSTEM_API UShowAnimStatic : public UShowBase
{
	GENERATED_BODY()

public:
    TWeakObjectPtr<UAnimSequenceBase> GetAnimSequenceBase() const
    {
        return TWeakObjectPtr<UAnimSequenceBase>(AnimSequenceBase.Get());
    }

    virtual FString GetTitle() override;
    virtual float GetLength() override;

protected:
    virtual void Initialize() override;
    virtual void Dispose() override;
    virtual void Play() override;
    virtual void Reset();
    virtual void Pause() override {};
    virtual void UnPause() override {};
    virtual void Tick(float ScaleDeltaTime, float SystemDeltaTime, float BasePassedTime) override {};
    virtual void ApplyTimeScale(float FinalTimeScale) override;

    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:
    const FShowAnimStaticKey* AnimStaticKeyPtr = nullptr;

    UPROPERTY()
    TObjectPtr<UAnimSequenceBase> AnimSequenceBase = nullptr;
    
    UPROPERTY()
    TObjectPtr<UAnimMontage> AnimMontage = nullptr;    
    
    UPROPERTY()
    TObjectPtr<UAnimInstance> AnimInstance = nullptr;

    float OriginalLength = 0.0f;
};
