// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTime/EShowKeyType.h"
#include "ObjectPool/ObjectPoolType.h"

/**
 * 
 */
class SHOWSYSTEM_API ShowSystem
{
public:
    static EObjectPoolType GetShowKeyPoolType(EShowKeyType InShowKeyType);
    static UScriptStruct* GetShowKeyStaticStruct(EShowKeyType InShowKeyType);
};
