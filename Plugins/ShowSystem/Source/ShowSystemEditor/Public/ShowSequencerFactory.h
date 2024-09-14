// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "ShowSequencerFactory.generated.h"

/**
 * 
 */
UCLASS()
class SHOWSYSTEMEDITOR_API UShowSequencerFactory : public UFactory
{
	GENERATED_BODY()
	

public:
	UShowSequencerFactory();

	// Override this to specify the type of object this factory creates
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	// Override this to specify the supported class
	virtual bool ShouldShowInNewMenu() const override;
};
