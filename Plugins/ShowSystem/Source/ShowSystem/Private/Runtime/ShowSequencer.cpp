// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowSequencer.h"
#include "RunTime/ShowBase.h"
#include "ObjectPool/ObjectPoolManager.h"

UShowSequencer::UShowSequencer()
{
    for (const FInstancedStruct Key : ShowKeys)
    {
        // FShowKey 타입에 따라 UShowKey 인스턴스 생성
        // RuntimeShowKeys에 추가
    }
}

UShowBase* UShowSequencer::CreateShowObject(const FShowKey& InShowKey)
{
    EObjectPoolType PoolType = ShowSystem::GetShowKeyPoolType(InShowKey.KeyType);
    UObjectPoolManager* PoolManager = GetWorld()->GetSubsystem<UObjectPoolManager>();
    UShowBase* ShowBase = PoolManager->GetPooledObject<UShowBase>(PoolType);
    ShowBase->InitShowKey(InShowKey);
    return ShowBase;
}

void UShowSequencer::Play()
{
    ShowSequencerState = EShowSequencerState::ShowSequencer_Playing;
    PassedTime = 0.0f;
}

void UShowSequencer::Stop()
{
    ShowSequencerState = EShowSequencerState::ShowSequencer_End;
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

        int endCount = 0;
        for (TObjectPtr<UShowBase> showBase : RuntimeShowKeys)
        {
            if (!showBase)
            {
                continue;
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
			else if (showBase->IsEnd())
			{
				endCount++;
			}
        }
        
        if (endCount == RuntimeShowKeys.Num())
		{
			ShowSequencerState = EShowSequencerState::ShowSequencer_End;
		}
    }
}