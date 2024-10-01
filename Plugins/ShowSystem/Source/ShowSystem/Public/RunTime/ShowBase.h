// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectPool/Pooled.h"
#include "UObject/NoExportTypes.h"
#include "RunTime/ShowSystem.h"
#include "RunTime/ShowSequencer.h"
#include "ShowBase.generated.h"

UENUM(BlueprintType)
enum class EShowKeyState : uint8
{
    ShowKey_Wait UMETA(DisplayName = "ShowKey Wait"),
    ShowKey_Playing UMETA(DisplayName = "ShowKey Playing"),
    ShowKey_Pause UMETA(DisplayName = "ShowKey Pause"),
    ShowKey_End UMETA(DisplayName = "ShowKey End"),
};


USTRUCT(Atomic, BlueprintType)
struct FShowKey
{
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowKey")
    EShowKeyType KeyType = EShowKeyType::Max;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowKey")
    float StartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowKey")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowKey")
    bool IsLoop = false;
};

/**
 * ShowInterface
 */
UCLASS(Abstract)
class SHOWSYSTEM_API UShowBase : public UObject, public IPooled
{
	GENERATED_BODY()

public:
    // 객체가 풀에서 꺼내졌을 때 호출됨
    virtual void OnPooled() override
    {
    }

    // 객체가 풀로 반환될 때 호출됨
    virtual void OnReturnedToPool() override
    {
        ShowSequencer = nullptr;
        ShowKey = nullptr;

        Dispose();
    }

    void InitShowKey(TObjectPtr<UShowSequencer> InShowSequencer, const FShowKey& InShowKey)
    {
        ShowSequencer = InShowSequencer;

        if (ShowKey.IsValid())
        {
            ShowKey.Reset();
        }
        ShowKey = TSharedPtr<const FShowKey>(&InShowKey);
        Initialize(InShowKey);
    }

    virtual void Initialize(const FShowKey& InShowKey) PURE_VIRTUAL(UShowBase::Initialize, );
    virtual void Dispose() PURE_VIRTUAL(UShowBase::Dispose, );
    virtual void Play() PURE_VIRTUAL(UShowBase::Play, );
    virtual void Stop() PURE_VIRTUAL(UShowBase::Stop, );
    virtual void Pause() PURE_VIRTUAL(UShowBase::Pause, );
    virtual void UnPause() PURE_VIRTUAL(UShowBase::UnPause, );
    virtual void Tick(float DeltaTime)
    {
        PassedTime += DeltaTime;
        Tick(DeltaTime, PassedTime);
    }

    EShowKeyType GetKeyType() const 
    { 
        checkf(ShowKey, TEXT("UShowBase::IsPassed: The ShowKey provided is invalid or null."));
        return ShowKey->KeyType; 
    }
    EShowKeyState GetShowKeyState() const { return ShowKeyState; }
    bool IsWait() const { return ShowKeyState == EShowKeyState::ShowKey_Wait; }
    bool IsPlaying() const { return ShowKeyState == EShowKeyState::ShowKey_Playing; }
    bool IsPause() const { return ShowKeyState == EShowKeyState::ShowKey_Pause; }
    bool IsEnd() const { return ShowKeyState == EShowKeyState::ShowKey_End; }
    bool IsPassed(float InPassedTime) const 
    { 
        checkf(ShowKey, TEXT("UShowBase::IsPassed: The ShowKey provided is invalid or null."));
        return ShowKey->StartTime >= InPassedTime;
    }

protected:
    AActor* GetOwner() const
    {
        return ShowSequencer->GetOwner();
    }

private:
    virtual void Tick(float DeltaTime, float BasePassedTime) PURE_VIRTUAL(UShowBase::Tick, );

protected:
    TObjectPtr<UShowSequencer> ShowSequencer;

    // ShowSequencer 어셋으로 받아온거라 절대 편집하면 안됨, 편집은 오로지 툴에서만 가능함
    TSharedPtr<const FShowKey> ShowKey;

    EShowKeyState ShowKeyState = EShowKeyState::ShowKey_Wait;

    float PassedTime = 0.0f;
};

    