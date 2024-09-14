// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowSequencerFactory.h"
#include "RunTime/ShowSequencer.h"

UShowSequencerFactory::UShowSequencerFactory()
{
	bCreateNew = true;  // Factory creates new objects
	bEditAfterNew = true;  // Open the object after creation
	SupportedClass = UShowSequencer::StaticClass();
}

UObject* UShowSequencerFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	// Create a new instance of UShowSequencer
	return NewObject<UShowSequencer>(InParent, Class, Name, Flags);
}

bool UShowSequencerFactory::ShouldShowInNewMenu() const
{
	// Show this factory in the "Create Advanced Asset" menu
	return true;
}
