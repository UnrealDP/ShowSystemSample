// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ActorPoolType.h"
#include "ObjectPool/ActorPoolCapacityDataAsset.h"
#include "ObjectPool/Pooled.h"
#include "ActorPoolManager.generated.h"

/**
 * 
 */
UCLASS()
class COREUTILS_API UActorPoolManager : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection);
    virtual void Deinitialize() override;

	// 풀 설정을 초기화하는 메서드
    void InitializePoolSettings(FString AssetPath);

    // 특정 타입의 객체를 풀에서 가져오는 메서드
    template <typename T>
    T* GetPooledObject(EActorPoolType ActorType, const FActorSpawnParameters& SpawnParameters = nullptr)
    {
        GetPooledObject(ActorType, NULL, SpawnParameters);
    }

    template <typename T>
    T* GetPooledObject(EActorPoolType ActorType, FVector const& Location, FRotator const& Rotation, const FActorSpawnParameters& SpawnParameters = nullptr)
    {
        FTransform Transform;
        if (Location)
        {
            Transform.SetLocation(*Location);
        }
        if (Rotation)
        {
            Transform.SetRotation(FQuat(*Rotation));
        }

        GetPooledObject(ActorType, Transform, SpawnParameters);
    }

    template <typename T>
    T* GetPooledObject(EActorPoolType ActorType, FTransform const& Transform, const FActorSpawnParameters& SpawnParameters = nullptr)
    {
        EnsurePoolsInitialized(ActorType);

        int32 Index = static_cast<int32>(ActorType);
        // 액터 클래스는 ActorType에 맞게 정의된 클래스 타입이거나 상속 관계여야 함
        checkf(T::StaticClass()->IsChildOf(PoolSettings[Index].ActorClass),
            TEXT("UActorPoolManager::ExpandPool / T not match ActorClass."));

        AActor* PooledActor;
        if (ActorPools[Index].Num() == 0)
        {
            ExpandPool(ActorType, Transform, SpawnParameters);  // 필요시 풀 확장
            PooledActor = ActorPools[Index].Pop();
        }
        else
        {
            PooledActor = ActorPools[Index].Pop();
            if (SpawnParameters)
            {
                UpdateSpawnParameters(PooledActor, SpawnParameters);
            }
        }
        PooledActor->SetActorHiddenInGame(false);  // 객체 활성화

        // 객체가 IPooled 인터페이스를 구현했는지 확인
        checkf(PooledActor->GetClass()->ImplementsInterface(UPooled::StaticClass()),
            TEXT("The pooled actor does not implement the IPooled interface."));

        IPooled* PooledInterface = Cast<IPooled>(PooledActor);
        if (PooledInterface)
        {
            PooledInterface->OnPooled();
        }
        return Cast<T>(PooledActor);
    }

    // 객체를 풀로 반환하는 메서드
    void ReturnPooledObject(AActor* Object, EActorPoolType ActorType);

private:
    inline void EnsurePoolsInitialized(EActorPoolType ActorType)
    {
        // ActorPools 배열이 초기화되었는지 확인
        checkf(ActorPools.Num() > 0, TEXT("[UActorPoolManager] ActorPools 배열이 초기화되지 않았습니다. InitializePoolSettings 함수를 먼저 호출하세요."));

        // 지정한 ActorType에 해당하는 풀 배열이 유효한지 확인
        checkf(ActorPools.IsValidIndex(static_cast<int32>(ActorType)), TEXT("[UActorPoolManager] 지정된 ActorType에 해당하는 풀 배열이 초기화되지 않았습니다."));

        // 해당 ActorType의 풀 배열에 객체가 있는지 확인
        checkf(ActorPools[static_cast<int32>(ActorType)].Num() > 0, TEXT("[UActorPoolManager] 지정된 ActorType에 해당하는 풀에 객체가 없습니다."));
    }

    // 풀 크기를 확장하는 메서드
    template <typename T>
    void ExpandPool(EActorPoolType ActorType, FTransform const& Transform, const FActorSpawnParameters& SpawnParameters)
    {
        int32 Index = static_cast<int32>(ActorType);
        // 액터 클래스는 ActorType에 맞게 정의된 클래스 타입이거나 상속 관계여야 함
        checkf(T::StaticClass()->IsChildOf(PoolSettings[Index].ActorClass),
            TEXT("UActorPoolManager::ExpandPool / T not match ActorClass."));

        UWorld* World = GetWorld();
        int32 ReservedActorCount = PoolSettings[Index].ReservedActorCount;

        for (int32 i = 0; i < ReservedActorCount; i++)
        {
            T* NewActor = World->SpawnActor<T>(PoolSettings[Index].ActorClass, Transform, SpawnParameters);
            NewActor->SetActorHiddenInGame(true);  // 비활성화된 상태로 추가
            ActorPools[Index].Add(NewActor);
        }
    }

    void UpdateSpawnParameters(AActor* Actor, const FActorSpawnParameters& SpawnParameters);

private:
    bool bIsInitialized = false;
    TArray<FActorPoolTypeSettings> PoolSettings;  // 풀 설정을 저장하는 배열

    TArray<TArray<AActor*>> ActorPools;  // 객체 풀을 저장하는 배열    
};
