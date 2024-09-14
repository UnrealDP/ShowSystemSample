// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Pooled.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPooled : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class COREUTILS_API IPooled
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 객체가 풀에서 꺼내졌을 때 호출됨
	virtual void OnPooled() = 0;

	// 객체가 풀로 반환될 때 호출됨
	virtual void OnReturnedToPool() = 0;
};
