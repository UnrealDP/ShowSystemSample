// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PooledActor.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPooledActor : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class COREUTILS_API IPooledActor
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// ��ü�� Ǯ���� �������� �� ȣ���
	virtual void OnPooled() = 0;

	// ��ü�� Ǯ�� ��ȯ�� �� ȣ���
	virtual void OnReturnedToPool() = 0;
};
