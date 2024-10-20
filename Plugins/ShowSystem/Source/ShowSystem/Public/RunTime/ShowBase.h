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
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ShowKey")
    EShowKeyType KeyType = EShowKeyType::Max;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowKey")
    float StartTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowKey")
    float Length = 0.0f;
};

/**
 * ShowInterface
 */
UCLASS(Abstract)
class SHOWSYSTEM_API UShowBase : public UObject, public IPooled
{
	GENERATED_BODY()

#if WITH_EDITOR
    friend FShowSequencerEditorHelper;
#endif


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

        ShowKeyState = EShowKeyState::ShowKey_Wait;

        ShowSequencer = InShowSequencer;
        CachedTimeScale = ShowSequencer->GetTimeScale();

        ShowKey = InShowKey;
        Length = InShowKey->Length;

        Initialize();
        Length = GetShowLength();
    }

    void BaseTick(float DeltaTime)
    {
        PassedTime += (DeltaTime * CachedTimeScale);

        // 키의 종류에 따라서 시간이 아닌 이벤트 같은거로 처리할 수 있음 (예시: UShowAnimStatic)
        /*if (CachedTimeScale > 0)
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
        }*/
        
        Tick(DeltaTime, PassedTime);
    }

    void ExecutePlay()
    {
        if (ShowKeyState == EShowKeyState::ShowKey_Playing)
		{
			return;
		}
        ShowKeyState = EShowKeyState::ShowKey_Playing;
		Play();
    }
    void ExecuteStop()
    {
        if (ShowKeyState == EShowKeyState::ShowKey_End)
        {
            return;
        }
        ShowKeyState = EShowKeyState::ShowKey_End;
        Stop();
    }
    void ExecuteReset()
    {
        ShowKeyState = EShowKeyState::ShowKey_Wait;

        PassedTime = 0.0f;

        Length = ShowKey->Length;
        Reset();
        Length = GetShowLength();
    }
    void ExecutePause()
    {
		if (ShowKeyState != EShowKeyState::ShowKey_Playing)
		{
			return;
		}
        ShowKeyState = EShowKeyState::ShowKey_Pause;
		Pause();
	}
    void ExecuteUnPause()
    {
        if (ShowKeyState != EShowKeyState::ShowKey_Pause)
        {
            return;
        }
        ShowKeyState = EShowKeyState::ShowKey_Playing;
        UnPause();
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
			Length = GetShowLength();
		}
        return Length;
    }

    const FShowKey* GetShowKey() const { return ShowKey; }

#if WITH_EDITOR
    float EditorInitializeAssetLength()
	{
		return GetShowLength();
	}
#endif
     
public:
    virtual FString GetTitle() PURE_VIRTUAL(UShowBase::GetTitle, return "ShowBase";);
    virtual float GetShowLength() PURE_VIRTUAL(UShowBase::GetShowLength, return 0.f;);

protected:
    AActor* GetOwner() const
    {
        return ShowSequencer->GetOwner();
    }

    virtual void Initialize() PURE_VIRTUAL(UShowBase::Initialize, );
    // 여기 있는 Length 는 실제 리소스의 Length와 ShowKey에 설정한 Length로 플레이해야할 Length를 구한 값이다.
    // Initialize 된 후에 호출된다
    virtual void Dispose() PURE_VIRTUAL(UShowBase::Dispose, );
    virtual void Play() PURE_VIRTUAL(UShowBase::Play, );
    virtual void Stop() PURE_VIRTUAL(UShowBase::Stop, );
    virtual void Reset() PURE_VIRTUAL(UShowBase::Reset, );
    virtual void Pause() PURE_VIRTUAL(UShowBase::Pause, );
    virtual void UnPause() PURE_VIRTUAL(UShowBase::UnPause, );
    virtual void Tick(float DeltaTime, float BasePassedTime) PURE_VIRTUAL(UShowBase::Tick, );
    virtual void ApplyTimeScale(float FinalTimeScale) PURE_VIRTUAL(UShowBase::ApplyTimeScale, );

protected:
    TObjectPtr<UShowSequencer> ShowSequencer = nullptr;

    // ShowSequencer 어셋으로 받아온거라 절대 편집하면 안됨, 편집은 오로지 툴에서만 가능함
    const FShowKey* ShowKey = nullptr;

    EShowKeyState ShowKeyState = EShowKeyState::ShowKey_Wait;

    // 여기 있는 Length 는 실제 리소스의 Length와 ShowKey에 설정한 Length로 플레이해야할 Length를 구한 값이다.
    float Length = FLT_MAX;
    float PassedTime = 0.0f;
    float KeyTimeScale = 1.0f;
    float CachedTimeScale = 1.0f;
};

    