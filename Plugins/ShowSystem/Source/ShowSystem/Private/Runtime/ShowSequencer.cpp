// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowSequencer.h"
#include "RunTime/ShowBase.h"
#include "RunTime/ShowSystem.h"

void UShowSequencer::OnPooled()
{
}

void UShowSequencer::OnReturnedToPool()
{
    // Owner 를 null 먼저 하면 ClearShowObjects 에서 checkf(Owner) 에서 에러 발생
    if (Owner)
    {
        ClearShowObjects();
        Owner = nullptr;
    }

    ShowSequenceAsset = nullptr;
    bIsDontDestroy = false;
    ShowSequencerState = EShowSequencerState::ShowSequencer_Wait;
    PassedTime = 0.0f;
    TimeScale = 1.0f;
}

void UShowSequencer::Initialize(AActor* InOwner, TObjectPtr<UShowSequenceAsset> InShowSequenceAsset)
{
    Owner = InOwner;
    ShowSequenceAsset = InShowSequenceAsset;
    GenerateShowBase();
}

void UShowSequencer::GenerateShowBase()
{
    RuntimeShowKeys.SetNum(ShowSequenceAsset->ShowKeys.Num());
    for (int32 i = 0; i < ShowSequenceAsset->ShowKeys.Num(); ++i)
    {
        const FInstancedStruct& Key = ShowSequenceAsset->ShowKeys[i];

        checkf(Key.GetScriptStruct()->IsChildOf(FShowKey::StaticStruct()), TEXT("UShowSequencer::GenerateShowBase: not FShowKey."));

        const FShowKey* ShowKey = Key.GetPtr<FShowKey>();
        if (!ShowKey)
        {
            continue;
        }

        UShowBase* ShowBasePtr = CreateShowObject(ShowKey);
        RuntimeShowKeys[i] = ShowBasePtr;
    }
}

UShowBase* UShowSequencer::CreateShowObject(const FShowKey* InShowKey)
{
    checkf(InShowKey, TEXT("UShowSequencer::CreateShowObject: InShowKey is Invalid."));
    checkf(Owner, TEXT("UShowSequencer::CreateShowObject: Owner is Invalid."));

    UWorld* World = Owner->GetWorld();
    if (!World)
	{
		return nullptr;
	}

    UObjectPoolManager* PoolManager = World->GetSubsystem<UObjectPoolManager>();
    EObjectPoolType PoolType = ShowSystem::GetShowKeyPoolType(InShowKey->KeyType);    
    UShowBase* ShowBase = PoolManager->GetPooledObject<UShowBase>(PoolType);
    ShowBase->InitShowKey(this, InShowKey);
    return ShowBase;
}

void UShowSequencer::ClearShowObjects()
{
    checkf(Owner, TEXT("UShowSequencer::ClearShowObjects: Owner is Invalid."));

    UWorld* World = Owner->GetWorld();
    if (!World)
    {
        return;
    }

    UObjectPoolManager* PoolManager = World->GetSubsystem<UObjectPoolManager>();
    for (UShowBase* ShowBasePtr : RuntimeShowKeys)
    {
        EObjectPoolType PoolType = ShowSystem::GetShowKeyPoolType(ShowBasePtr->GetKeyType());
        PoolManager->ReturnPooledObject(ShowBasePtr, PoolType);
    }
    RuntimeShowKeys.Empty();
}

void UShowSequencer::Play()
{
    checkf(Owner, TEXT("UShowSequencer::Play: Owner is invalid"));

    ShowSequencerState = EShowSequencerState::ShowSequencer_Playing;
    PassedTime = 0.0f;
}

void UShowSequencer::Reset()
{
    for (UShowBase* ShowBasePtr : RuntimeShowKeys)
    {
        if (!ShowBasePtr)
        {
            continue;
        }

        ShowBasePtr->ExecuteReset();
    }

    PassedTime = 0.0f;
    ShowSequencerState = EShowSequencerState::ShowSequencer_Wait;
}

void UShowSequencer::Pause()
{
    if (ShowSequencerState != EShowSequencerState::ShowSequencer_Playing)
    {
        return;
    }

    for (UShowBase* ShowBasePtr : RuntimeShowKeys)
    {
        if (!ShowBasePtr)
        {
            continue;
        }

        ShowBasePtr->ExecutePause();
    }

    ShowSequencerState = EShowSequencerState::ShowSequencer_Pause;
}

void UShowSequencer::UnPause()
{
    if (ShowSequencerState != EShowSequencerState::ShowSequencer_Pause)
	{
		return;
	}

    for (UShowBase* ShowBasePtr : RuntimeShowKeys)
    {
        if (!ShowBasePtr)
        {
            continue;
        }

        ShowBasePtr->ExecuteUnPause();
    }

    ShowSequencerState = EShowSequencerState::ShowSequencer_Playing;
}

void UShowSequencer::ChangeTimeScale(float InTimeScale)
{
    TimeScale = InTimeScale;

    for (UShowBase* ShowBasePtr : RuntimeShowKeys)
    {
        if (!ShowBasePtr)
        {
            continue;
        }

        ShowBasePtr->OnUpdateSequenceTimeScale();
    }
}

void UShowSequencer::Tick(float DeltaTime)
{
    if (ShowSequencerState == EShowSequencerState::ShowSequencer_Playing)
    {
        PassedTime += (DeltaTime * TimeScale);

        bool bIsAllEnd = true;
        for (UShowBase* ShowBasePtr : RuntimeShowKeys)
        {
            if (!ShowBasePtr)
            {
                continue;
            }

            if (bIsAllEnd && !ShowBasePtr->IsEnd())
            {
                bIsAllEnd = false;
            }

            if (ShowBasePtr->IsWait())
            {
                if (ShowBasePtr->IsPassedStartTime(PassedTime))
                {
                    ShowBasePtr->ExecutePlay();
                }
            }
			else if (ShowBasePtr->IsPlaying())
			{
                ShowBasePtr->BaseTick(DeltaTime);
			}
        }
        
        // IsEnd() 에서 바로 Object Pool로 반환 안하는 이유 :
        // UShowSequencer 같은 경우는 그다지 길지 않은 연출에 사용하는데
        // Object Pool로 반환하고 TArray<UShowBase*> RuntimeShowKeys 에서 remove를 하는 것은
        // 메모리 이동이 너무 자주 발생할 것이기에 모든 키가 끝났을 때 한번에 반환하도록 함
        // 물론 툴에서는 지속적으로 플레이 해보면서 확인 하기 때문에 FShowSequencerEditorHelper::SetShowSequencerEditor 여기서 DontDestroy 함
        if (bIsAllEnd)
        {
            ShowSequencerState = EShowSequencerState::ShowSequencer_End;
            if (!bIsDontDestroy)
            {
                ClearShowObjects();
            }
        }
    }
}
