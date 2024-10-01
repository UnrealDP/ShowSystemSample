// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowSequencer.h"
#include "RunTime/ShowBase.h"
#include "ObjectPool/ObjectPoolManager.h"

UShowSequencer::UShowSequencer()
{
    GenerateShowBase();
}

void UShowSequencer::BeginDestroy()
{
	ClearShowObjects();

    Owner = nullptr;

    Super::BeginDestroy();
}

void UShowSequencer::GenerateShowBase()
{
    RuntimeShowKeys.SetNum(ShowKeys.Num());
    for (int32 i = 0; i < ShowKeys.Num(); ++i)
    {
        const FInstancedStruct& Key = ShowKeys[i];

        checkf(Key.GetScriptStruct() == FShowKey::StaticStruct(), TEXT("UShowSequencer::GenerateShowBase: not FShowKey."));

        const FShowKey* ShowKey = Key.GetPtr<FShowKey>();
        if (!ShowKey)
        {
            continue;
        }

        UShowBase* ShowBase = CreateShowObject(*ShowKey);
        RuntimeShowKeys[i] = ShowBase;
    }
}

UShowBase* UShowSequencer::CreateShowObject(const FShowKey& InShowKey)
{
    checkf(Owner, TEXT("UShowSequencer::CreateShowObject: Owner is Invalid."));

    UWorld* World = Owner->GetWorld();
    UObjectPoolManager* PoolManager = World->GetSubsystem<UObjectPoolManager>();

    EObjectPoolType PoolType = ShowSystem::GetShowKeyPoolType(InShowKey.KeyType);    
    UShowBase* ShowBase = PoolManager->GetPooledObject<UShowBase>(PoolType);
    ShowBase->InitShowKey(this, InShowKey);
    return ShowBase;
}

void UShowSequencer::ClearShowObjects()
{
    checkf(Owner, TEXT("UShowSequencer::ClearShowObjects: Owner is Invalid."));

    UWorld* World = Owner->GetWorld();
    UObjectPoolManager* PoolManager = World->GetSubsystem<UObjectPoolManager>();
    for (TObjectPtr<UShowBase> showBase : RuntimeShowKeys)
    {
        EObjectPoolType PoolType = ShowSystem::GetShowKeyPoolType(showBase->GetKeyType());
        PoolManager->ReturnPooledObject(showBase, PoolType);
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
    ShowSequencerState = EShowSequencerState::ShowSequencer_End;

    ClearShowObjects();
}

void UShowSequencer::Pause()
{
    ShowSequencerState = EShowSequencerState::ShowSequencer_Pause;
}

void UShowSequencer::UnPause()
{
    ShowSequencerState = EShowSequencerState::ShowSequencer_Playing;
}

void UShowSequencer::ChangeSpeed(float Speed)
{
    // 모든 키에 대해 속도 변경
}

void UShowSequencer::Tick(float DeltaTime)
{
    if (ShowSequencerState == EShowSequencerState::ShowSequencer_Playing)
    {
        PassedTime += DeltaTime;

        bool bIsAllEnd = true;
        for (TObjectPtr<UShowBase> showBase : RuntimeShowKeys)
        {
            if (!showBase)
            {
                continue;
            }

            if (bIsAllEnd && !showBase->IsEnd())
            {
                bIsAllEnd = false;
            }

            if (showBase->IsWait())
            {
                if (showBase->IsPassed(PassedTime))
                {
                    showBase->Play();
                }
            }
			else if (showBase->IsPlaying())
			{
				showBase->Tick(DeltaTime);
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