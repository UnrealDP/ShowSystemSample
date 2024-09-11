// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPool/ActorPoolManager.h"
#include "ObjectPool/PooledActor.h"

template <typename T>
T* UActorPoolManager::GetPooledObject(EActorType ActorType)
{
    int32 Index = static_cast<int32>(ActorType);
    if (ActorPools[Index].Num() == 0)
    {
        ExpandPool(ActorType, 10);  // 필요시 풀 확장
    }

    AActor* PooledActor = ActorPools[Index].Pop();
    PooledActor->SetActorHiddenInGame(false);  // 객체 활성화

    // 객체가 IPooledActor 인터페이스를 구현했는지 확인
    checkf(PooledActor->GetClass()->ImplementsInterface(UPooledActor::StaticClass()),
        TEXT("The pooled actor does not implement the IPooledActor interface."));

    IPooledActor::Execute_OnPooled(PooledActor);

    return Cast<T>(PooledActor);
}

template <typename T>
void UActorPoolManager::ReturnPooledObject(T* Object, EActorType ActorType)
{
    int32 Index = static_cast<int32>(ActorType);
    Object->SetActorHiddenInGame(true);  // 객체 비활성화

    // 객체가 IPooledActor 인터페이스를 구현했는지 확인
    checkf(Object->GetClass()->ImplementsInterface(UPooledActor::StaticClass()),
        TEXT("The pooled actor does not implement the IPooledActor interface."));

    IPooledActor::Execute_OnReturnedToPool(Object);

    ActorPools[Index].Add(Object);  // 풀에 객체를 다시 추가
}

void UActorPoolManager::ExpandPool(EActorType ActorType, int32 PoolSize)
{
    UWorld* World = GetWorld();
    int32 Index = static_cast<int32>(ActorType);
    for (int32 i = 0; i < PoolSize; i++)
    {
        // 액터 클래스는 ActorType에 맞게 정의된 클래스 타입이어야 함
        AActor* NewActor = World->SpawnActor<AActor>(/* 적절한 액터 클래스를 스폰 */);
        NewActor->SetActorHiddenInGame(true);  // 비활성화된 상태로 추가
        ActorPools[Index].Add(NewActor);
    }
}