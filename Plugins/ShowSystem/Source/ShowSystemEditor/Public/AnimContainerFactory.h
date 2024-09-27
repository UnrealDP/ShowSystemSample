// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AnimContainerFactory.generated.h"

/**
 * 
 */
UCLASS()
class SHOWSYSTEMEDITOR_API UAnimContainerFactory : public UFactory
{
	GENERATED_BODY()
	
public:
	UAnimContainerFactory();

	// Override this to specify the type of object this factory creates
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	// Override this to specify the supported class
	virtual bool ShouldShowInNewMenu() const override;
};
