// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShowAnimMontageNotifyHook.generated.h"

class UAnimMontage;

/**
 * 
 */
UCLASS()
class SHOWSYSTEM_API UShowAnimMontageNotifyHook : public UObject
{
	GENERATED_BODY()

public:
	UShowAnimMontageNotifyHook();

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};

//UCLASS()
//class SHOWSYSTEM_API AnimMontageNotifyHook
//{
//    GENERATED_BODY()
//
//    AnimMontageNotifyHook()
//    {
//    }
//
//    UFUNCTION()
//    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
//};