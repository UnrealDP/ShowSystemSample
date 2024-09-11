// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ActorPoolManager.generated.h"

// 클래스 종류를 Enum으로 정의
enum class EActorType : uint8
{
    Max
};
/**
 * 
 */
UCLASS()
class COREUTILS_API UActorPoolManager : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
    UActorPoolManager()
    {
        ActorPools.SetNum(static_cast<int32>(EActorType::Max));  // 클래스 수만큼 배열 초기화
    }

    // 특정 타입의 객체를 풀에서 가져오는 메서드
    template <typename T>
    T* GetPooledObject(EActorType ActorType);

    // 객체를 풀로 반환하는 메서드
    template <typename T>
    void ReturnPooledObject(T* Object, EActorType ActorType);

private:
    TArray<TArray<AActor*>> ActorPools;  // 객체 풀을 저장하는 배열

    // 풀 크기를 확장하는 메서드
    void ExpandPool(EActorType ActorType, int32 PoolSize);
};
