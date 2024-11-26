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

/**
 * 다양한 Key의 데이터를 구현하면서 필히 상속해야 하는 기본 Key Struct
 */
USTRUCT(Atomic, BlueprintType)
struct FShowKey
{
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ShowKey")
    EShowKeyType KeyType = EShowKeyType::Max;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowKey")
    float StartTime = 0.0f;

    UPROPERTY(EditAnywhere, Category = "ShowKey")
    float PlayRate = 1.0f;
};

/**
 * 다양한 Key 데이터로 생성할 인스턴스로 각 키의 기능 구현은 UShowBase를 상속해서 구현해야함
 * 각 키 기능을 구현하는 곳에서 오류, 리소스 없음 등으로 키 실행이 불가능하면 해당 구문에서 바로 EShowKeyState::ShowKey_End 로 키 종료 처리 해줘야 한다.
 * 물론 키의 기능이 정상 완료되는 곳에서도 바로 EShowKeyState::ShowKey_End 로 키 종료 처리 해줘야 한다.
 * 키가 Pause되면 TimeScale을 0으로 해서 ApplyTimeScale 의 파라미터 FinalTimeScale을 0.0f로 호출해주니 구현 필수.
 * TODO: (DIPI) Stop은 기능이 필요할지 혹은 Cancel로 할지 아직 미정으로 Show를 활용하는 방안을 논의한 후에 결정해야함. 아마 Cancel로 결정될 것으로 예상.
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
        ShowSequencerPtr = nullptr;
        ShowKey = nullptr;

        PassedTime = 0.0f;
        KeyTimeScale = 1.0f;
        CachedTimeScale = 1.0f;

        Dispose();
        ShowKeyState = EShowKeyState::ShowKey_Wait;
    }

    void InitShowKey(UShowSequencer* InShowSequencerPtr, const FShowKey* InShowKey)
    {
        checkf(InShowSequencerPtr, TEXT("UShowBase::InitShowKey: The InShowSequencerPtr is invalid."));
        checkf(InShowKey, TEXT("UShowBase::InitShowKey: The InShowKey is invalid."));

        ShowKeyState = EShowKeyState::ShowKey_Wait;

        ShowSequencerPtr = InShowSequencerPtr;
        CachedTimeScale = ShowSequencerPtr->GetTimeScale();

        ShowKey = InShowKey;
        Initialize();
    }

    void BaseTick(float DeltaTime)
    {
        float ScaleDeltaTime = DeltaTime * CachedTimeScale * ShowKey->PlayRate;
        PassedTime += ScaleDeltaTime;        
        Tick(ScaleDeltaTime, DeltaTime, PassedTime);
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
    void ExecuteReset()
    {
        ShowKeyState = EShowKeyState::ShowKey_Wait;
        PassedTime = 0.0f;
        Reset();
    }
    void ExecutePause()
    {
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
    void ExecuteCancel()
    {
        if (ShowKeyState == EShowKeyState::ShowKey_Wait)
        {
            return;
        }
        ShowKeyState = EShowKeyState::ShowKey_Wait;
        Cancel();
    }
    void ExecuteSetPassedTime(float InTime)
    {
        PassedTime = InTime;
        SetPassedTime(InTime);
    }    

    void SetKeyTimeScale(float InKeyTimeScale)
	{
        KeyTimeScale = InKeyTimeScale;
        CachedTimeScale = KeyTimeScale * ShowSequencerPtr->GetTimeScale();
        ApplyTimeScale(CachedTimeScale);
	}

    void OnUpdateSequenceTimeScale()
	{
        CachedTimeScale = KeyTimeScale * ShowSequencerPtr->GetTimeScale();
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

    const FShowKey* GetShowKey() const { return ShowKey; }

public:
    virtual FString GetTitle() PURE_VIRTUAL(UShowBase::GetTitle, return "ShowBase";);

    // 툴이나 혹은 전체 시간이 필요한 경우 사용되는 함수
    virtual float GetLength() PURE_VIRTUAL(UShowBase::GetLength, return 0.f;);

    AActor* GetShowOwner() const
    {
        return ShowSequencerPtr->GetShowOwner();
    }

protected:
    // Key가 인스턴스 생성되면서 최초 호출되는 함수
    virtual void Initialize() PURE_VIRTUAL(UShowBase::Initialize, );

    // Key가 삭제되면서 호출되는 소멸 함수
    virtual void Dispose() PURE_VIRTUAL(UShowBase::Dispose, );

    // Key가 StartTime이 지나서 플레이 시작될 때 호출되는 함수
    virtual void Play() PURE_VIRTUAL(UShowBase::Play, );    

    // Key를 다시 플레이 하기 위해 초기화로 불리는 함수 (대부분 툴에서 호출되는 함수)
    virtual void Reset() PURE_VIRTUAL(UShowBase::Reset, );

    /*
    *   ScaleDeltaTime : Show 내부 TimeScale 적용된 시간 변화량
        SystemDeltaTime : Owner로 부터 호출되는 시스템 시간 변화량
        BasePassedTime : TimeScale 적용된 현재 Key의 플레이된 지난 총 시간
    */
    virtual void Tick(float ScaleDeltaTime, float SystemDeltaTime, float BasePassedTime) PURE_VIRTUAL(UShowBase::Tick, );

    virtual void Pause() PURE_VIRTUAL(UShowBase::Pause, );
    virtual void UnPause() PURE_VIRTUAL(UShowBase::UnPause, );

    // 왜부에서 TimeScale을 변화해서 실제 최종 Key에 적용되어야 하는 FinalTimeScale 값으로 호출함 (여기서 실제 시간 변화에 대한 코드 해야함)
    virtual void ApplyTimeScale(float FinalTimeScale) PURE_VIRTUAL(UShowBase::ApplyTimeScale, );

    virtual void SetPassedTime(float InTime) PURE_VIRTUAL(UShowBase::SetPassedTime, );

    // Key를 멈출때 호출되는 함수 (이 곳에서 멈춤 처리를 해줘야 한다)
    virtual void Cancel() PURE_VIRTUAL(UShowBase::Cancel, );

protected:
    UShowSequencer* ShowSequencerPtr = nullptr;

    // ShowSequencer 어셋으로 받아온거라 절대 편집하면 안됨, 편집은 오로지 툴에서만 가능함
    const FShowKey* ShowKey = nullptr;

    EShowKeyState ShowKeyState = EShowKeyState::ShowKey_Wait;

    float PassedTime = 0.0f;    // 현재 Key의 플레이된 지난 총 시간
    float KeyTimeScale = 1.0f;  // Key의 TimeScale
    float CachedTimeScale = 1.0f; // 계산된 적용되어야할 TimeScale (Key의 TimeScale * ShowSequencer의 TimeScale)
};

    