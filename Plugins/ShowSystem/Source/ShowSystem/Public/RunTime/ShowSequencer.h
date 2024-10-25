// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InstancedStruct.h"
#include "ObjectPool/ObjectPoolManager.h"
#include "ObjectPool/ObjectPoolType.h"
#include "ShowSequenceAsset.h"
#include "ShowSequencer.generated.h"

class UShowBase;
struct FShowKey;

OBJPOOL_TYPE_INDEX(UShowSequencer, EObjectPoolType::ObjectPool_ShowSequencer)

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
class SHOWSYSTEM_API UShowSequencer : public UObject, public IPooled
{
	GENERATED_BODY()

#if WITH_EDITOR
    friend class FShowSequencerEditorHelper;
#endif
    friend class UShowSequencerComponent;

public:
    virtual void OnPooled() override;
    virtual void OnReturnedToPool() override;

private:
    void Play();
    void Reset();
    void Pause();
    void UnPause();

public:
    // setter getter
    void ClearOwner() { Owner = nullptr; }
    AActor* GetOwner() { return Owner; }

    void SetDontDestroy() { bIsDontDestroy = true; }
    void ReleaseDontDestroy() { bIsDontDestroy = false; }

    float GetTimeScale() const { return TimeScale; }
    float GetPassedTime() const { return PassedTime; }
    // end of setter getter

    void Initialize(AActor* InOwner, TObjectPtr<UShowSequenceAsset> InShowSequenceAsset);
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
    UPROPERTY()
    TObjectPtr<UShowSequenceAsset> ShowSequenceAsset = nullptr;

    AActor* Owner = nullptr;

    bool bIsDontDestroy = false;
    EShowSequencerState ShowSequencerState = EShowSequencerState::ShowSequencer_Wait;
    float PassedTime = 0.0f;    
    TArray<UShowBase*> RuntimeShowKeys;
    float TimeScale = 1.0f;
};
