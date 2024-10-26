// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTime/EShowKeyType.h"
#include "ObjectPool/ObjectPoolType.h"

class UShowBase;

/**
 * 
 */
class SHOWSYSTEM_API ShowSystem
{
public:
    static EObjectPoolType GetShowKeyPoolType(EShowKeyType InShowKeyType);
    static UScriptStruct* GetShowKeyStaticStruct(EShowKeyType InShowKeyType);

    static void NotifyShowKeyChange(UShowBase* ShowBasePtr, FName PropertyName);
    
    static bool ValidateRuntimeShowKey(AActor* Owner, UShowBase* ShowBasePtr, FText& ErrTxt);
    static bool ValidateShowAnimStatic(AActor* Owner, UShowBase* ShowBasePtr, FText& ErrTxt);
    static bool ValidateShowCascade(AActor* Owner, UShowBase* ShowBasePtr, FText& ErrTxt);
};
