// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ObjectPoolType.h"
#include "ObjectPool/ObjectPoolCapacityDataAsset.h"
#include "ObjectPool/Pooled.h"
#include "ObjectPoolManager.generated.h"

/**
 * 
 */
UCLASS()
class COREUTILS_API UObjectPoolManager : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection);
    virtual void Deinitialize() override;

    // 풀 설정을 초기화하는 메서드
    void InitializePoolSettings(FString AssetPath);

    // 특정 타입의 객체를 풀에서 가져오는 메서드
    template <typename T>
    T* GetPooledObject(EObjectPoolType ObjectType)
    {
        EnsurePoolsInitialized(ObjectType);

        int32 Index = static_cast<int32>(ObjectType);

        // Object 클래스는 ObjectType에 맞게 정의된 클래스 타입이거나 상속 관계여야 함
        checkf(T::StaticClass()->IsChildOf(PoolSettings[Index].ObjectClass),
            TEXT("UObjectPoolManager::ExpandPool / T not match ObjectClass."));

        UObject* PooledObject;
        if (ObjectPools[Index].Num() == 0)
        {
            ExpandPool<T>(ObjectType);  // 필요시 풀 확장
        }

        PooledObject = ObjectPools[Index].Pop();

        // 객체가 IPooled 인터페이스를 구현했는지 확인
        checkf(PooledObject->GetClass()->ImplementsInterface(UPooled::StaticClass()),
            TEXT("The pooled Object does not implement the IPooled interface."));

        IPooled* PooledInterface = Cast<IPooled>(PooledObject);
        if (PooledInterface)
        {
            PooledInterface->OnPooled();
        }

        return Cast<T>(PooledObject);
    }

    // 객체를 풀로 반환하는 메서드
    void ReturnPooledObject(UObject* Object, EObjectPoolType ObjectType);

private:
    inline void EnsurePoolsInitialized(EObjectPoolType ObjectType)
    {
        // ObjectPools 배열이 초기화되었는지 확인
        checkf(ObjectPools.Num() > 0, TEXT("[UObjectPoolManager] ObjectPools 배열이 초기화되지 않았습니다. InitializePoolSettings 함수를 먼저 호출하세요."));

        // 지정한 ObjectType에 해당하는 풀 배열이 유효한지 확인
        checkf(ObjectPools.IsValidIndex(static_cast<int32>(ObjectType)), TEXT("[UObjectPoolManager] 지정된 ObjectType에 해당하는 풀 배열이 초기화되지 않았습니다."));

        // 해당 ObjectType의 풀 배열에 객체가 있는지 확인
        checkf(ObjectPools[static_cast<int32>(ObjectType)].Num() > 0, TEXT("[UObjectPoolManager] 지정된 ObjectType에 해당하는 풀에 객체가 없습니다."));
    }

    // 풀 크기를 확장하는 메서드
    template <typename T>
    void ExpandPool(EObjectPoolType ObjectType)
    {
        int32 Index = static_cast<int32>(ObjectType);

        // Object 클래스는 ObjectType에 맞게 정의된 클래스 타입이거나 상속 관계여야 함
        checkf(T::StaticClass()->IsChildOf(PoolSettings[Index].ObjectClass),
            TEXT("UObjectPoolManager::ExpandPool / T not match ObjectClass."));

        int32 ReservedObjectCount = PoolSettings[Index].ReservedObjectCount;

        for (int32 i = 0; i < ReservedObjectCount; i++)
        {
            T* Object = NewObject<T>();  // UObject를 직접 생성
            if (Object)
            {
                ObjectPools[Index].Add(Object);
            }
        }
    }

private:
    bool bIsInitialized = false;
    TArray<FObjectPoolTypeSettings> PoolSettings;  // 풀 설정을 저장하는 배열

    TArray<TArray<UObject*>> ObjectPools;  // 객체 풀을 저장하는 배열  
};
