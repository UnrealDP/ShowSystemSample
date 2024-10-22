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

    // 에디터에서 프리뷰는 UWorldSubsystem 을 상속받은 서브시스템은 지원하지 않음.
    // 그래서 직접 PoolSettings 를 가져오는 함수를 만들어서 에디터에서 사용할 수 있도록 함.
    static void GetPoolSettings(TArray<FObjectPoolTypeSettings>& OutPoolSettings);

    // 특정 타입의 객체를 풀에서 가져오는 메서드
    template <typename T>
    T* GetPooledObject(EObjectPoolType ObjectType)
    {
        int32 Index = static_cast<int32>(ObjectType);

        // Object 클래스는 ObjectType에 맞게 정의된 클래스 타입이거나 상속 관계여야 함
        checkf(PoolSettings[Index].ObjectClass->IsChildOf(T::StaticClass()),
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

    template <typename T>
    T* GetPooledObject()
    {
        EObjectPoolType ObjectType = ObjectPoolTypeIndex<T>::GetType();

        checkf(ObjectType != EObjectPoolType::Max,
            TEXT("UObjectPoolManager::GetPooledObject / ObjectType is invalid"));

        return GetPooledObject<T>(ObjectType);
    }

    // 객체를 풀로 반환하는 메서드
    void ReturnPooledObject(UObject* Object, EObjectPoolType ObjectType);

private:

    // 풀 크기를 확장하는 메서드
    template <typename T>
    void ExpandPool(EObjectPoolType ObjectType)
    {
        int32 Index = static_cast<int32>(ObjectType);

        // Object 클래스는 ObjectType에 맞게 정의된 클래스 타입이거나 상속 관계여야 함
        checkf(PoolSettings[Index].ObjectClass->IsChildOf(T::StaticClass()),
            TEXT("UObjectPoolManager::ExpandPool / T not match ObjectClass."));

        int32 ReservedObjectCount = PoolSettings[Index].ReservedObjectCount;

        for (int32 i = 0; i < ReservedObjectCount; i++)
        {
            T* NewObjectInstance = NewObject<T>(this, PoolSettings[Index].ObjectClass);
            if (NewObjectInstance)
            {
                NewObjectInstance->AddToRoot();
                ObjectPools[Index].Add(NewObjectInstance);
            }
        }
    }

private:
    bool bIsInitialized = false;
    TArray<FObjectPoolTypeSettings> PoolSettings;  // 풀 설정을 저장하는 배열

    TArray<TArray<UObject*>> ObjectPools;  // 객체 풀을 저장하는 배열  
};
