// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPool/ActorPoolManager.h"
#include "ObjectPool/PooledActor.h"

template <typename T>
T* UActorPoolManager::GetPooledObject(EActorType ActorType)
{
    int32 Index = static_cast<int32>(ActorType);
    if (ActorPools[Index].Num() == 0)
    {
        ExpandPool(ActorType, 10);  // �ʿ�� Ǯ Ȯ��
    }

    AActor* PooledActor = ActorPools[Index].Pop();
    PooledActor->SetActorHiddenInGame(false);  // ��ü Ȱ��ȭ

    // ��ü�� IPooledActor �������̽��� �����ߴ��� Ȯ��
    checkf(PooledActor->GetClass()->ImplementsInterface(UPooledActor::StaticClass()),
        TEXT("The pooled actor does not implement the IPooledActor interface."));

    IPooledActor::Execute_OnPooled(PooledActor);

    return Cast<T>(PooledActor);
}

template <typename T>
void UActorPoolManager::ReturnPooledObject(T* Object, EActorType ActorType)
{
    int32 Index = static_cast<int32>(ActorType);
    Object->SetActorHiddenInGame(true);  // ��ü ��Ȱ��ȭ

    // ��ü�� IPooledActor �������̽��� �����ߴ��� Ȯ��
    checkf(Object->GetClass()->ImplementsInterface(UPooledActor::StaticClass()),
        TEXT("The pooled actor does not implement the IPooledActor interface."));

    IPooledActor::Execute_OnReturnedToPool(Object);

    ActorPools[Index].Add(Object);  // Ǯ�� ��ü�� �ٽ� �߰�
}

void UActorPoolManager::ExpandPool(EActorType ActorType, int32 PoolSize)
{
    UWorld* World = GetWorld();
    int32 Index = static_cast<int32>(ActorType);
    for (int32 i = 0; i < PoolSize; i++)
    {
        // ���� Ŭ������ ActorType�� �°� ���ǵ� Ŭ���� Ÿ���̾�� ��
        AActor* NewActor = World->SpawnActor<AActor>(/* ������ ���� Ŭ������ ���� */);
        NewActor->SetActorHiddenInGame(true);  // ��Ȱ��ȭ�� ���·� �߰�
        ActorPools[Index].Add(NewActor);
    }
}