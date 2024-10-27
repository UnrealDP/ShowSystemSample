// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ObjectPoolType.generated.h"

/* 번거로운 과정으로 보일 수 있겠으나 아래와 같은 사항으로 EObjectPoolType 을 사용한다.
 * 1. 사용자가 직접 명시한 오브젝트 이외에는 오브젝트 풀에 마음대로 새로운 타입을 추가/삭제 못하게 하기 위해서
 * 2. 외부에서 오브젝트 풀에서 Get을 할때 잘못된 클래스 사용 등의 코드 실수를 막기 위해서
 * 3. 오브젝트 풀로 관리되고 있는 타입을 한눈에 알아볼 수 있게 하기 위해서
 * 4. enum 으로 각 타입의 풀에 접근시 Map 등을 사용하지 않고 바로 index 접근을 하기 위해서
 */

UENUM(BlueprintType)
enum class EObjectPoolType : uint8
{
    ObjectPool_ActionSkill          UMETA(DisplayName = "ObjectPool ActionSkill"),
    ObjectPool_ShowSequencer        UMETA(DisplayName = "ObjectPool ShowSequencer"),
    ObjectPool_ShowKeyAnim          UMETA(DisplayName = "ObjectPool ShowKeyAnim"),
    ObjectPool_ShowKeyCascade       UMETA(DisplayName = "ObjectPool ShowKeyCascade"),
    ObjectPool_ShowCamSequenceKey   UMETA(DisplayName = "ObjectPool ShowCamSequenceKey"),
    Max   UMETA(DisplayName = "Max Types")
};

template<typename T>
struct ObjectPoolTypeIndex
{
    static EObjectPoolType GetType()
    {
        static_assert(sizeof(T) == 0, "OBJPOOL_TYPE_INDEX must be defined for this class. "
            "Please define OBJPOOL_TYPE_INDEX for your pool object class.");
        return EObjectPoolType::Max;  // 기본값
    }

    static int32 GetIndex()
    {
        static_assert(sizeof(T) == 0, "OBJPOOL_TYPE_INDEX must be defined for this class. "
            "Please define OBJPOOL_TYPE_INDEX for your pool object class.");
        return -1;  // 기본값
    }
};

#define OBJPOOL_TYPE_INDEX(ClassType, EnumValue)      \
template<>                                           \
struct ObjectPoolTypeIndex<ClassType>                          \
{                                                    \
    static EObjectPoolType GetType() { return EnumValue; } \
    static int32 GetIndex() { return static_cast<int32>(EnumValue); } \
};
