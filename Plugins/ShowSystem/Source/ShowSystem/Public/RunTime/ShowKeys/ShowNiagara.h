// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RunTime/ShowBase.h"
#include "ShowNiagara.generated.h"


USTRUCT(BlueprintType)
struct FShowNiagaraKey : public FShowKey
{
    GENERATED_BODY()

    FShowNiagaraKey()
    {
        KeyType = EShowKeyType::ShowKey_Niagara;
    }
};

/**
 * 
 */
UCLASS()
class SHOWSYSTEM_API UShowNiagara : public UShowBase
{
	GENERATED_BODY()

    virtual FString GetTitle() override;
    virtual float GetLength() override;

protected:
    virtual void Initialize() override;
    virtual void Dispose() override;
    virtual void Play() override;
    virtual void Reset();
    virtual void Tick(float ScaleDeltaTime, float SystemDeltaTime, float BasePassedTime) override;
    virtual void ApplyTimeScale(float FinalTimeScale) override;
};
