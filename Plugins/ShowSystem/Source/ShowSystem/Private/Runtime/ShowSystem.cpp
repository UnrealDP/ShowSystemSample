// Fill out your copyright notice in the Description page of Project Settings.

#include "RunTime/ShowSystem.h"
#include "InstancedStruct.h"
#include "RunTime/ShowKeys/ShowAnimStatic.h"

EObjectPoolType ShowSystem::GetShowKeyPoolType(EShowKeyType InShowKeyType)
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

UScriptStruct* ShowSystem::GetShowKeyStaticStruct(EShowKeyType InShowKeyType)
{
    switch (InShowKeyType)
    {
    case EShowKeyType::ShowKey_Anim:
        return FShowAnimStaticKey::StaticStruct();
    case EShowKeyType::ShowKey_Particle:
        return FShowAnimStaticKey::StaticStruct();
    default:
        checkf(false, TEXT("ShowSystem::GetShowKeyStaticStruct Invalid EShowKeyType: [ %d ]"), static_cast<int32>(InShowKeyType));
        return nullptr;
    }
}