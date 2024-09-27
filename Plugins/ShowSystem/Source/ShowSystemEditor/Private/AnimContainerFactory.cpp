// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimContainerFactory.h"
#include "RunTime/Animation/AnimContainer.h"

UAnimContainerFactory::UAnimContainerFactory()
{
	bCreateNew = true;  // Factory creates new objects
	bEditAfterNew = true;  // Open the object after creation
	SupportedClass = UAnimContainer::StaticClass();
}

UObject* UAnimContainerFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	// Create a new instance of UShowSequencer
	return NewObject<UAnimContainer>(InParent, Class, Name, Flags);
}

bool UAnimContainerFactory::ShouldShowInNewMenu() const
{
	// Show this factory in the "Create Advanced Asset" menu
	return true;
}