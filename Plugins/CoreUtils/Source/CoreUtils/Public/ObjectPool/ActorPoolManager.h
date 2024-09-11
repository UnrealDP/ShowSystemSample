// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ActorPoolManager.generated.h"

// Ŭ���� ������ Enum���� ����
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
        ActorPools.SetNum(static_cast<int32>(EActorType::Max));  // Ŭ���� ����ŭ �迭 �ʱ�ȭ
    }

    // Ư�� Ÿ���� ��ü�� Ǯ���� �������� �޼���
    template <typename T>
    T* GetPooledObject(EActorType ActorType);

    // ��ü�� Ǯ�� ��ȯ�ϴ� �޼���
    template <typename T>
    void ReturnPooledObject(T* Object, EActorType ActorType);

private:
    TArray<TArray<AActor*>> ActorPools;  // ��ü Ǯ�� �����ϴ� �迭

    // Ǯ ũ�⸦ Ȯ���ϴ� �޼���
    void ExpandPool(EActorType ActorType, int32 PoolSize);
};
