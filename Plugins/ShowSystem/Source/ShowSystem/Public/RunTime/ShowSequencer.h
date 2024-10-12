// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InstancedStruct.h"
#include "ObjectPool/ObjectPoolManager.h"
#include "ShowSequencer.generated.h"

class UShowBase;
struct FShowKey;

UENUM(BlueprintType)
enum class EShowSequencerState : uint8
{
    ShowSequencer_Wait UMETA(DisplayName = "ShowSequencer Wait"),
    ShowSequencer_Playing UMETA(DisplayName = "ShowSequencer Playing"),
    ShowSequencer_Pause UMETA(DisplayName = "ShowSequencer Pause"),
    ShowSequencer_End UMETA(DisplayName = "ShowSequencer End"),
};

/**
 * 
 */
UCLASS(BlueprintType)
class SHOWSYSTEM_API UShowSequencer : public UObject
{
	GENERATED_BODY()

    friend class UShowSequencerComponent;

public:
    UShowSequencer();
    virtual void BeginDestroy() override;

    float GetPassedTime() const { return PassedTime; }

private:
    void Play();
    void Stop();
    void Pause();
    void UnPause();

#if WITH_EDITOR
public:
    void EditorInitialize();
    FShowKey* EditorAddKey(FInstancedStruct& Key);
    void EditorReset();
    void EditorPlay();
    void EditorStop();
    void EditorPause() { Pause(); }
    void EditorUnPause() { UnPause(); }
    void EditorClearShowObjects();
    void EditorBeginDestroy();
    UShowBase* EditorGetShowBase(FShowKey* ShoeKey);
    TArray<TObjectPtr<UShowBase>>* EditorGetShowKeys() { return &RuntimeShowKeys; }

    TArray<FObjectPoolTypeSettings> EditorPoolSettings;
#endif

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Show")
    TArray<FInstancedStruct> ShowKeys;

public:
    // setter getter
    void SetOwner(AActor* InOwner) { Owner = InOwner; }
    void ClearOwner() { Owner = nullptr; }
    AActor* GetOwner() { return Owner.Get(); }

    void SetDontDestroy() { bIsDontDestroy = true; }
    void ReleaseDontDestroy() { bIsDontDestroy = false; }

    float GetTimeScale() const { return TimeScale; }
    // end of setter getter

    void Dispose()
    {
        // Owner 를 null 먼저 하면 ClearShowObjects 에서 checkf(Owner) 에서 에러 발생
        if (Owner)
        {
            ClearShowObjects();
            Owner = nullptr;
        }

        bIsDontDestroy = false;
        ShowSequencerState = EShowSequencerState::ShowSequencer_Wait;
        PassedTime = 0.0f;
        TimeScale = 1.0f;
    }
    void Tick(float DeltaTime);

    EShowSequencerState GetShowSequencerState() const { return ShowSequencerState; }
    bool IsWait() const { return ShowSequencerState == EShowSequencerState::ShowSequencer_Wait; }
    bool IsPlaying() const { return ShowSequencerState == EShowSequencerState::ShowSequencer_Playing; }
    bool IsPause() const { return ShowSequencerState == EShowSequencerState::ShowSequencer_Pause; }
    bool IsEnd() const { return ShowSequencerState == EShowSequencerState::ShowSequencer_End; }

    void ChangeTimeScale(float InTimeScale);

private:
    void GenerateShowBase();
    class UShowBase* CreateShowObject(const FShowKey* InShowKey);
    void ClearShowObjects();
        
private:
    bool bIsDontDestroy = false;
    EShowSequencerState ShowSequencerState = EShowSequencerState::ShowSequencer_Wait;
    float PassedTime = 0.0f;
    TObjectPtr<AActor> Owner;
    TArray<TObjectPtr<UShowBase>> RuntimeShowKeys;
    float TimeScale = 1.0f;
};
