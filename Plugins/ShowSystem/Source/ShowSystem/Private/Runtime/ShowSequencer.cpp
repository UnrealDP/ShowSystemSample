// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowSequencer.h"
#include "RunTime/ShowBase.h"
#include "RunTime/ShowSystem.h"

UShowSequencer::UShowSequencer()
{
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
    for (TObjectPtr<UShowBase>& showBase : RuntimeShowKeys)
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
    if (ShowSequencerState == EShowSequencerState::ShowSequencer_End)
	{
		return;
	}

    for (TObjectPtr<UShowBase>& showBase : RuntimeShowKeys)
    {
        if (!showBase)
        {
            continue;
        }

        showBase->ExecuteStop();
    }

    ShowSequencerState = EShowSequencerState::ShowSequencer_End;
}

void UShowSequencer::Pause()
{
    if (ShowSequencerState != EShowSequencerState::ShowSequencer_Playing)
    {
        return;
    }

    for (TObjectPtr<UShowBase>& showBase : RuntimeShowKeys)
    {
        if (!showBase)
        {
            continue;
        }

        showBase->ExecutePause();
    }

    ShowSequencerState = EShowSequencerState::ShowSequencer_Pause;
}

void UShowSequencer::UnPause()
{
    if (ShowSequencerState != EShowSequencerState::ShowSequencer_Pause)
	{
		return;
	}

    for (TObjectPtr<UShowBase>& showBase : RuntimeShowKeys)
    {
        if (!showBase)
        {
            continue;
        }

        showBase->ExecuteUnPause();
    }

    ShowSequencerState = EShowSequencerState::ShowSequencer_Playing;
}

void UShowSequencer::ChangeTimeScale(float InTimeScale)
{
    TimeScale = InTimeScale;

    for (TObjectPtr<UShowBase>& showBase : RuntimeShowKeys)
    {
        if (!showBase)
        {
            continue;
        }

        showBase->OnUpdateSequenceTimeScale();
    }
}

void UShowSequencer::Tick(float DeltaTime)
{
    if (ShowSequencerState == EShowSequencerState::ShowSequencer_Playing)
    {
        PassedTime += (DeltaTime * TimeScale);

        bool bIsAllEnd = true;
        for (TObjectPtr<UShowBase>& showBase : RuntimeShowKeys)
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
                if (showBase->IsPassedStartTime(PassedTime))
                {
                    showBase->ExecutePlay();
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

#if WITH_EDITOR
void UShowSequencer::EditorInitialize()
{
    if (RuntimeShowKeys.IsEmpty())
    {
        RuntimeShowKeys.SetNum(ShowKeys.Num());
    }
    
    for (int32 i = 0; i < ShowKeys.Num(); ++i)
    {
        if (RuntimeShowKeys[i])
        {
            continue;
        }

        const FInstancedStruct& Key = ShowKeys[i];
        checkf(Key.GetScriptStruct()->IsChildOf(FShowKey::StaticStruct()), TEXT("UShowSequencer::EditorPlay: not FShowKey."));

        const FShowKey* ShowKey = Key.GetPtr<FShowKey>();
        if (!ShowKey)
        {
            continue;
        }

        if (EditorPoolSettings.Num() == 0)
        {
            UObjectPoolManager::GetPoolSettings(EditorPoolSettings);
        }

        EObjectPoolType PoolType = ShowSystem::GetShowKeyPoolType(ShowKey->KeyType);
        int32 Index = static_cast<int32>(PoolType);
        UShowBase* ShowBase = NewObject<UShowBase>((UObject*)GetTransientPackage(), EditorPoolSettings[Index].ObjectClass);
        ShowBase->InitShowKey(this, ShowKey);
        RuntimeShowKeys[i] = ShowBase;
    }
}

FShowKey* UShowSequencer::EditorAddKey(FInstancedStruct& NewKey)
{
    checkf(NewKey.GetScriptStruct()->IsChildOf(FShowKey::StaticStruct()), TEXT("UShowSequencer::EditorInitializeKey: not FShowKey."));

    ShowKeys.Add(MoveTemp(NewKey));

    FShowKey* NewShowKey = ShowKeys.Last().GetMutablePtr<FShowKey>();
    if (!NewShowKey)
    {
        return nullptr;
    }

    if (EditorPoolSettings.Num() == 0)
    {
        UObjectPoolManager::GetPoolSettings(EditorPoolSettings);
    }

    EObjectPoolType PoolType = ShowSystem::GetShowKeyPoolType(NewShowKey->KeyType);
    int32 Index = static_cast<int32>(PoolType);
    UShowBase* NewShowBase = NewObject<UShowBase>((UObject*)GetTransientPackage(), EditorPoolSettings[Index].ObjectClass);
    NewShowBase->InitShowKey(this, NewShowKey);
    RuntimeShowKeys.Add(NewShowBase);

    MarkPackageDirty();
    return NewShowKey;
}

void UShowSequencer::EditorPlay()
{ 
    EditorInitialize();
    Play(); 
}

void UShowSequencer::EditorStop() 
{ 
    ShowSequencerState = EShowSequencerState::ShowSequencer_End;
    EditorClearShowObjects();
}

void UShowSequencer::EditorClearShowObjects()
{
    for (TObjectPtr<UShowBase>& showBase : RuntimeShowKeys)
    {
        showBase = nullptr;
    }
    RuntimeShowKeys.Empty();
}

void UShowSequencer::EditorBeginDestroy()
{
    EditorClearShowObjects();
    Owner = nullptr;
}
#endif