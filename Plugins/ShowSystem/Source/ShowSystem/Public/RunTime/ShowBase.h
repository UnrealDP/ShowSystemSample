// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectPool/Pooled.h"
#include "UObject/NoExportTypes.h"
#include "RunTime/EShowKeyType.h"
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
    float Length = 0.0f;

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

        Length = FLT_MAX;
        PassedTime = 0.0f;
        KeyTimeScale = 1.0f;
        CachedTimeScale = 1.0f;

        Dispose();
        ShowKeyState = EShowKeyState::ShowKey_Wait;
    }

    void InitShowKey(TObjectPtr<UShowSequencer> InShowSequencer, const FShowKey* InShowKey)
    {
        checkf(InShowSequencer, TEXT("UShowBase::InitShowKey: The InShowSequencer is invalid."));
        checkf(InShowKey, TEXT("UShowBase::InitShowKey: The InShowKey is invalid."));

        ShowSequencer = InShowSequencer;
        CachedTimeScale = ShowSequencer->GetTimeScale();

        ShowKey = InShowKey;
        Length = InShowKey->Length;

        Initialize();
        Length = InitializeAssetLength();

        ShowKeyState = EShowKeyState::ShowKey_Wait;
    }

    void Tick(float DeltaTime)
    {
        PassedTime += (DeltaTime * CachedTimeScale);

        if (CachedTimeScale > 0)
        {
            if (Length <= PassedTime)
            {
                ShowKeyState = EShowKeyState::ShowKey_End;
            }
        }
        else
        {
            if (PassedTime <= 0.0f)
            {
                ShowKeyState = EShowKeyState::ShowKey_End;
            }
        }
        
        Tick(DeltaTime, PassedTime);
    }

    void ExecutePlay()
    {
        if (ShowKeyState == EShowKeyState::ShowKey_Playing)
		{
			return;
		}
		Play();
		ShowKeyState = EShowKeyState::ShowKey_Playing;
    }
    void ExecuteStop()
    {
        if (ShowKeyState == EShowKeyState::ShowKey_End)
        {
            return;
        }
        Stop();
        ShowKeyState = EShowKeyState::ShowKey_End;
    }
    void ExecuteReset()
    {
        if (ShowKeyState == EShowKeyState::ShowKey_Wait)
        {
            return;
        }

        PassedTime = 0.0f;

        Reset();
        ShowKeyState = EShowKeyState::ShowKey_Wait;
    }
    void ExecutePause()
    {
		if (ShowKeyState != EShowKeyState::ShowKey_Playing)
		{
			return;
		}
		Pause();
		ShowKeyState = EShowKeyState::ShowKey_Pause;
	}
    void ExecuteUnPause()
    {
        if (ShowKeyState != EShowKeyState::ShowKey_Pause)
        {
            return;
        }
        UnPause();
        ShowKeyState = EShowKeyState::ShowKey_Playing;
    }

    void SetKeyTimeScale(float InKeyTimeScale)
	{
        KeyTimeScale = InKeyTimeScale;
        CachedTimeScale = KeyTimeScale * ShowSequencer->GetTimeScale();
        ApplyTimeScale(CachedTimeScale);
	}

    void OnUpdateSequenceTimeScale()
	{
        CachedTimeScale = KeyTimeScale * ShowSequencer->GetTimeScale();
        ApplyTimeScale(CachedTimeScale);
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
    bool IsPassedStartTime(float InPassedTime) const 
    { 
        checkf(ShowKey, TEXT("UShowBase::IsPassed: The ShowKey provided is invalid or null."));
        return ShowKey->StartTime <= InPassedTime;
    }

    float GetStartTime()
    {
        if (!ShowKey)
        {
			return 0.0f;
		}
        return ShowKey->StartTime;
    }
    float GetLength() 
    { 
        if (Length == FLT_MAX)
		{
			Length = InitializeAssetLength();
		}
        return Length;
    }

    const FShowKey* GetShowKey() const { return ShowKey; }

#if WITH_EDITOR
    float EditorInitializeAssetLength()
	{
		return InitializeAssetLength();
	}
#endif
     

protected:
    AActor* GetOwner() const
    {
        return ShowSequencer->GetOwner();
    }

    virtual void Initialize() PURE_VIRTUAL(UShowBase::Initialize, );
    // 여기 있는 Length 는 실제 리소스의 Length와 ShowKey에 설정한 Length로 플레이해야할 Length를 구한 값이다.
    // Initialize 된 후에 호출된다
    virtual float InitializeAssetLength() PURE_VIRTUAL(UShowBase::InitializeAssetLength, return 0.f;);
    virtual void Dispose() PURE_VIRTUAL(UShowBase::Dispose, );
    virtual void Play() PURE_VIRTUAL(UShowBase::Play, );
    virtual void Stop() PURE_VIRTUAL(UShowBase::Stop, );
    virtual void Reset() PURE_VIRTUAL(UShowBase::Reset, );
    virtual void Pause() PURE_VIRTUAL(UShowBase::Pause, );
    virtual void UnPause() PURE_VIRTUAL(UShowBase::UnPause, );
    virtual void Tick(float DeltaTime, float BasePassedTime) PURE_VIRTUAL(UShowBase::Tick, );
    virtual void ApplyTimeScale(float FinalTimeScale) PURE_VIRTUAL(UShowBase::ApplyTimeScale, );

protected:
    TObjectPtr<UShowSequencer> ShowSequencer;

    // ShowSequencer 어셋으로 받아온거라 절대 편집하면 안됨, 편집은 오로지 툴에서만 가능함
    const FShowKey* ShowKey = nullptr;

    EShowKeyState ShowKeyState = EShowKeyState::ShowKey_Wait;

    // 여기 있는 Length 는 실제 리소스의 Length와 ShowKey에 설정한 Length로 플레이해야할 Length를 구한 값이다.
    // InitializeAssetLength 에서 초기화 되어야함
    float Length = FLT_MAX;
    float PassedTime = 0.0f;
    float KeyTimeScale = 1.0f;
    float CachedTimeScale = 1.0f;
};

    