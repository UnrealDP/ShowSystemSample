// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowSequencer.h"
#include "RunTime/ShowBase.h"
#include "RunTime/ShowSystem.h"

void UShowSequencer::Initialize(AActor* InOwner, TObjectPtr<UShowSequenceAsset> InShowSequenceAsset)
{
    Owner = InOwner;
    ShowSequenceAsset = InShowSequenceAsset;
    GenerateShowBase();
}

void UShowSequencer::BeginDestroy()
{
    // Owner 를 null 먼저 하면 ClearShowObjects 에서 checkf(Owner) 에서 에러 발생
    if (Owner)
    {
        ClearShowObjects();
        Owner = nullptr;
    }

    Super::BeginDestroy();
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

        UShowBase* ShowBase = CreateShowObject(ShowKey);
        RuntimeShowKeys[i] = ShowBase;
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
    for (TObjectPtr<UShowBase>& ShowBase : RuntimeShowKeys)
    {
        EObjectPoolType PoolType = ShowSystem::GetShowKeyPoolType(ShowBase->GetKeyType());
        PoolManager->ReturnPooledObject(ShowBase, PoolType);
    }
    RuntimeShowKeys.Empty();
}

void UShowSequencer::Play()
{
    checkf(Owner, TEXT("UShowSequencer::Play: Owner is invalid"));

    ShowSequencerState = EShowSequencerState::ShowSequencer_Playing;
    PassedTime = 0.0f;
}

void UShowSequencer::Stop()
{
    if (ShowSequencerState == EShowSequencerState::ShowSequencer_End)
	{
		return;
	}

    for (TObjectPtr<UShowBase>& ShowBase : RuntimeShowKeys)
    {
        if (!ShowBase)
        {
            continue;
        }

        ShowBase->ExecuteStop();
    }

    ShowSequencerState = EShowSequencerState::ShowSequencer_End;
}

void UShowSequencer::Pause()
{
    if (ShowSequencerState != EShowSequencerState::ShowSequencer_Playing)
    {
        return;
    }

    for (TObjectPtr<UShowBase>& ShowBase : RuntimeShowKeys)
    {
        if (!ShowBase)
        {
            continue;
        }

        ShowBase->ExecutePause();
    }

    ShowSequencerState = EShowSequencerState::ShowSequencer_Pause;
}

void UShowSequencer::UnPause()
{
    if (ShowSequencerState != EShowSequencerState::ShowSequencer_Pause)
	{
		return;
	}

    for (TObjectPtr<UShowBase>& ShowBase : RuntimeShowKeys)
    {
        if (!ShowBase)
        {
            continue;
        }

        ShowBase->ExecuteUnPause();
    }

    ShowSequencerState = EShowSequencerState::ShowSequencer_Playing;
}

void UShowSequencer::ChangeTimeScale(float InTimeScale)
{
    TimeScale = InTimeScale;

    for (TObjectPtr<UShowBase>& ShowBase : RuntimeShowKeys)
    {
        if (!ShowBase)
        {
            continue;
        }

        ShowBase->OnUpdateSequenceTimeScale();
    }
}

void UShowSequencer::Tick(float DeltaTime)
{
    if (ShowSequencerState == EShowSequencerState::ShowSequencer_Playing)
    {
        PassedTime += (DeltaTime * TimeScale);

        bool bIsAllEnd = true;
        for (TObjectPtr<UShowBase>& ShowBase : RuntimeShowKeys)
        {
            if (!ShowBase)
            {
                continue;
            }

            if (ShowBase->IsEnd())
            {
                UE_LOG(LogTemp, Warning, TEXT("UShowSequencer::Tick: TTT"));
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("UShowSequencer::Tick: FFF"));
            }

            if (bIsAllEnd && !ShowBase->IsEnd())
            {
                bIsAllEnd = false;
            }

            if (ShowBase->IsWait())
            {
                if (ShowBase->IsPassedStartTime(PassedTime))
                {
                    ShowBase->ExecutePlay();
                }
            }
			else if (ShowBase->IsPlaying())
			{
                ShowBase->BaseTick(DeltaTime);
			}
        }
        
        // IsEnd() 에서 바로 Object Pool로 반환 안하는 이유 :
        // UShowSequencer 같은 경우는 그다지 길지 않은 연출에 사용하는데
        // Object Pool로 반환하고 TArray<TObjectPtr<UShowBase>> RuntimeShowKeys 에서 remove를 하는 것은
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
