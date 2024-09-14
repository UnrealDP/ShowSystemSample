// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ObjectPoolType.h"
#include "ObjectPool/ObjectPoolCapacityDataAsset.h"
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
    T* GetPooledObject(EObjectPoolType ObjectType);

    // 객체를 풀로 반환하는 메서드
    template <typename T>
    void ReturnPooledObject(T* Object, EObjectPoolType ObjectType);

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
    void ExpandPool(EObjectPoolType ObjectType);

private:
    bool bIsInitialized = false;
    TArray<FObjectPoolTypeSettings> PoolSettings;  // 풀 설정을 저장하는 배열

    TArray<TArray<UObject*>> ObjectPools;  // 객체 풀을 저장하는 배열  
};
