// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectPool/ObjectPoolType.h"

UENUM(BlueprintType)
enum class EShowKeyType : uint8
{
    ShowKey_Anim UMETA(DisplayName = "Animation Key"),
    ShowKey_Particle UMETA(DisplayName = "Particle Key"),
    Max   UMETA(DisplayName = "Max Types")
};

/**
 * 
 */
class SHOWSYSTEM_API ShowSystem
{
public:
    static EObjectPoolType GetShowKeyPoolType(EShowKeyType InShowKeyType)
    {
        switch (InShowKeyType)
        {
            case EShowKeyType::ShowKey_Anim:
                return EObjectPoolType::ObjectPool_ShowKeyAnim;
            case EShowKeyType::ShowKey_Particle:
                return EObjectPoolType::ObjectPool_ShowKeyAnim;
            default:
                checkf(false, TEXT("ShowSystem::GetShowKeyPoolType Invalid EShowKeyType: [ %d ]"), static_cast<int32>(InShowKeyType));
                return EObjectPoolType::Max;
        }
    }
};
